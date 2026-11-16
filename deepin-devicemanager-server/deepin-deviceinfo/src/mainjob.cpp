// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainjob.h"
#include "deviceinterface.h"
#include "debugtimemanager.h"
#include "threadpool.h"
#include "detectthread.h"
#include "controlinterface.h"
#include "DDLog.h"

#include <QMutex>
#include <QProcess>
#include <QLoggingCategory>
#include <QTimer>
#include <QDBusConnection>

#include <atomic>

using namespace DDLog;

// 一次全量刷新的最长等待时间（DBus 层面的"服务端忙"保底）。
static constexpr int kFullRefreshTimeoutMs = 20000;

// 启动/唤醒时同步"禁用/唤醒"策略所用的 hwinfo 等待上限。
// 这段代码跑在主线程上（sqlCopytoKernel），一旦 hwinfo 卡住，整个进程的
// D-Bus 派发都会停摆。实测约 400ms，这里给足余量。
static constexpr int kPolicyHwinfoTimeoutMs = 15000;

static QMutex mainJobMutex;
static std::atomic<bool> s_ServerIsUpdating { false };
// 供静态的 serverIsRunning() 查询"是否还有任务在写缓存"，
// 让忙状态覆盖总超时之后仍在收尾的那段时间。
static std::atomic<ThreadPool *> s_pool { nullptr };
static bool s_ClientIsUpdating = false;

MainJob::MainJob(const char *name, QObject *parent)
    : QObject(parent)
    , m_pool(new ThreadPool)
    , m_firstUpdate(true)
{
    m_deviceInterface = new DeviceInterface(name, this);
    s_pool = m_pool;
    // 守护进程启动的时候加载所有信息
    updateAllDevice();

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->setWorkingFlag(ControlInterface::getInstance()->monitorWorkingDBFlag());
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);

    // 在驱动管理延迟加载1000ms
    QTimer::singleShot(1000, this, [ = ]() {
        sqlCopytoKernel();

        // 客户端主动请求刷新 / 驱动操作完成：需要完整设备信息，走全量刷新
        connect(m_deviceInterface, &DeviceInterface::sigUpdate, this, &MainJob::slotRefreshRequest);
        connect(ControlInterface::getInstance(), &ControlInterface::sigUpdate, this, &MainJob::slotRefreshRequest);
#ifndef DISABLE_DRIVER
        connect(ControlInterface::getInstance(), &ControlInterface::sigFinished, this, &MainJob::slotDriverControl);
#endif
    });

    QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PrepareForSleep", //"System has woken up from sleep (S3)" Signal
        this, SLOT(slotWakeupHandle(bool))
    );
}

MainJob::~MainJob()
{
    s_pool = nullptr;

    if (mp_DetectThread) {
        mp_DetectThread->setWorkingFlag(false);
        mp_DetectThread->wait(2000);
    }
}

void MainJob::sqlCopytoKernel()
{
    ControlInterface::getInstance()->disableInDevice();
    // 后台加载后先禁用设备 内核参数持久化
    QProcess process;
    QStringList options;
    options << "--netcard" << "--keyboard"  << "--mouse" <<  "--usb";
    process.start("hwinfo", options);
    if (!process.waitForFinished(kPolicyHwinfoTimeoutMs)) {
        process.kill();
        process.waitForFinished(1000);
        qCWarning(appLog) << "hwinfo timeout in sqlCopytoKernel, skip policy sync";
        return;
    }
    QString info = process.readAllStandardOutput();
    process.close();
    if (info.isEmpty()) {
        qCWarning(appLog) << "hwinfo returned empty output in sqlCopytoKernel, skip policy sync";
        return;
    }
    // init from sql db
    ControlInterface::getInstance()->disableOutDevice(info);
    ControlInterface::getInstance()->updateWakeup(info);
}

void MainJob::slotWakeupHandle(bool isSleep)
{
    QTimer::singleShot(1500, this, [ = ]() {
        qCInfo(appLog) << "Signal: login1.Manager.PrepareForSleep:" << isSleep;
        if (isSleep)
            return;

        QProcess process; //先唤醒DBUS
        QString command = "gdbus call --system --dest org.deepin.DeviceControl --object-path /org/deepin/DeviceControl --method org.deepin.DeviceControl.disableInDevice";
        process.start(command);
        process.waitForFinished(1000);

        //有的硬件唤醒起来也需要延时
        QTimer::singleShot(2000, this, [ = ]() {
            sqlCopytoKernel();
        });
    });
}

bool MainJob::serverIsRunning()
{
    if (s_ServerIsUpdating)
        return true;

    ThreadPool *pool = s_pool.load();
    return pool != nullptr && pool->activeThreadCount() > 0;
}

bool MainJob::clientIsRunning()
{
    return s_ClientIsUpdating;
}

void MainJob::setWorkingFlag(bool flag)
{
    mp_DetectThread->setWorkingFlag(flag);
    ControlInterface::getInstance()->setMonitorWorkingDBFlag(flag);
}

void MainJob::slotUsbChanged()
{
    qCInfo(appLog) << "usb/bluetooth changed, cache refresh deferred to client request";
}

void MainJob::slotRefreshRequest()
{
    // 客户端点刷新、启用/禁用设备、驱动安装卸载等场景需要完整的设备信息
    executeClientInstruction("DETECT");
}

void MainJob::slotDriverControl(bool success)
{
    if (success)
        executeClientInstruction("DETECT");
}

void MainJob::updateAllDevice()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateAllDevice()");
    s_ServerIsUpdating = true;
    if (m_firstUpdate)
        m_pool->loadDeviceInfo();
    else
        m_pool->updateDeviceInfo();
    // 整个刷新（命令派发 + 全部任务结束）只在这里限时一次。
    if (!m_pool->waitForDone(kFullRefreshTimeoutMs)) {
        qCWarning(appLog) << "device info refresh timed out after"
                          << kFullRefreshTimeoutMs << "ms, tasks still running:"
                          << m_pool->activeThreadCount();
    }
    s_ServerIsUpdating = false;
    PERF_PRINT_END("POINT-01");
    m_firstUpdate = false;
}

void MainJob::executeClientInstruction(const QString &instructions)
{
    QMutexLocker locker(&mainJobMutex);

    if (instructions.startsWith("DETECT")) {
        updateAllDevice();
    } else if (instructions.startsWith("START")) {
        if (m_firstUpdate) {
            updateAllDevice();
        }
    }
}
