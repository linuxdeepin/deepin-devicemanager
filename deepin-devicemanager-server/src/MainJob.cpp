// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MainJob.h"
#include "ThreadPool.h"
#include "DetectThread.h"
#include "DebugTimeManager.h"
#include "DBusInterface.h"
#ifndef DISABLE_DRIVER
#include "DriverDBusInterface.h"
#endif
#include "DBusEnableInterface.h"
#include "DBusWakeupInterface.h"
#include "DeviceInfoManager.h"
#include "EnableSqlManager.h"
#include "EnableUtils.h"
#include "WakeupUtils.h"
#include "DriverManager.h"
#include "NotifyThread.h"

//todo: deleted later
#include "DriverInstaller.h"

#include <QDateTime>
#include <QThread>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <QDBusConnection>
#include <QDebug>
#include <QFile>

#include <unistd.h>

static QMutex mutex;
const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString DRIVER_SERVICE_PATH = "/com/deepin/drivermanager";
const QString DEVICE_SERVICE_PATH = "/com/deepin/devicemanager";
const QString ENABLE_SERVICE_PATH = "/com/deepin/enablemanager";
const QString WAKEUP_SERVICE_PATH = "/com/deepin/wakeupmanager";
bool  MainJob::s_ServerIsUpdating = false;
bool  MainJob::s_ClientIsUpdating = false;
const QString DEVICE_REPO_PATH = "/etc/apt/sources.list.d/devicemanager.list";
const QString DRIVER_REPO_PATH = "/etc/apt/sources.list.d/driver.list";

MainJob::MainJob(QObject *parent)
    : QObject(parent)
    , mp_Pool(new ThreadPool)
    , mp_DetectThread(nullptr)
    , mp_IFace(new DBusInterface(this))
    , mp_DriverOperateIFace(nullptr)
    , m_FirstUpdate(true)
{
    // 守护进程启动的时候加载所有信息
    updateAllDevice();
    //启动时，检测驱动是否要更新，如果要更新则通知系统
// 取消开机驱动安装提示
//    DriverManager *drivermanager = new DriverManager(this);
//    if (drivermanager->checkDriverInfo()) {
//        NotifyThread *thread = new NotifyThread();
//        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
//        thread->start();
//    }
}

MainJob::~MainJob()
{
}
void MainJob::working()
{
    // 启动dbus
    if (!initDBus()) {
        exit(1);
    }

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->setWorkingFlag(EnableSqlManager::getInstance()->monitorWorkingFlag());
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);

    // 在驱动管理延迟加载1000ms
    QTimer::singleShot(1000, this, [ = ]() {
        //初始化源
#ifndef DISABLE_DRIVER
        initDriverRepoSource();
#endif
        // 后台加载后先禁用设备
        const QString &info = DeviceInfoManager::getInstance()->getInfo("hwinfo");
        EnableUtils::disableOutDevice(info);
        EnableUtils::disableInDevice();
        WakeupUtils::updateWakeupDeviceInfo(info);

#ifndef DISABLE_DRIVER
        mp_DriverOperateIFace = (new DriverDBusInterface(this));
#endif
        mp_Enable = (new DBusEnableInterface(this));
        mp_Wakeup = (new DBusWakeupInterface(this));
        if (!initDriverDbus()) {
            exit(1);
        }

        connect(mp_Enable, &DBusEnableInterface::update, this, &MainJob::slotUsbChanged);
        connect(mp_IFace, &DBusInterface::update, this, &MainJob::slotUsbChanged);
#ifndef DISABLE_DRIVER
        connect(mp_DriverOperateIFace, &DriverDBusInterface::sigFinished, this, &MainJob::slotDriverControl);
#endif
    });

    //todo: 先不删除，后续测试完再删除。
//    DriverInstaller *mp_driverInstaller = new DriverInstaller;;
//    mp_driverInstaller->installPackage("cnrcupsir2625zk","5.00-1");
}

INSTRUCTION_RES MainJob::executeClientInstruction(const QString &instructions)
{
    QMutexLocker locker(&mutex);
    s_ServerIsUpdating = true;
    INSTRUCTION_RES res = IR_NULL;

    if (instructions.startsWith("DETECT")) {
        this->thread()->msleep(1000);
        // 跟新缓存信息
        updateAllDevice();
    } else if (instructions.startsWith("START")) {
        if (m_FirstUpdate) {
            updateAllDevice();
        }
        res = IR_UPDATE;
    } else {
        res = IR_NULL;
    }

    s_ServerIsUpdating = false;
    return res;
}

bool MainJob::isZhaoXin()
{
    QString cmd = "sudo dmidecode -t 4";
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();
    if (info.contains("ZHAOXIN KaiXian KX-U")) {
        return true;
    } else {
        return false;
    }
}

bool MainJob::serverIsRunning()
{
    return s_ServerIsUpdating;
}

bool MainJob::clientIsRunning()
{
    return s_ClientIsUpdating;
}

void MainJob::setWorkingFlag(bool flag)
{
    mp_DetectThread->setWorkingFlag(flag);
    EnableSqlManager::getInstance()->setMonitorWorkingFlag(flag);
}

void MainJob::slotUsbChanged()
{
    executeClientInstruction("DETECT");
}

void MainJob::slotDriverControl(bool success)
{
    if (success)
        executeClientInstruction("DETECT");
}

void MainJob::onFirstUpdate()
{
    if (m_FirstUpdate) {
        updateAllDevice();
    }
}

void MainJob::updateAllDevice()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateAllDevice()");
    if (m_FirstUpdate)
        mp_Pool->loadDeviceInfo();
    else
        mp_Pool->updateDeviceInfo();
    mp_Pool->waitForDone(-1);
    PERF_PRINT_END("POINT-01");
    m_FirstUpdate = false;
}

bool MainJob::initDBus()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    //1. 申请一个总线连接
    if (!systemBus.isConnected()) {
        return false;
    }

    //2. 在总线连接上挂载服务，这样其他进程才能请求该服务
    if (!systemBus.registerService(SERVICE_NAME)) {
        return false;
    }

    //3. 在挂载的服务上注册一个执行服务的对象
    if (!systemBus.registerObject(DEVICE_SERVICE_PATH, mp_IFace, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }

    return true;
}

bool MainJob::initDriverDbus()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    //1. 申请一个总线连接
    if (!systemBus.isConnected()) {
        return false;
    }

    //2. 在总线连接上挂载服务，这样其他进程才能请求该服务
    if (!systemBus.registerService(SERVICE_NAME)) {
        return false;
    }
#ifndef DISABLE_DRIVER
    if (!systemBus.registerObject(DRIVER_SERVICE_PATH, mp_DriverOperateIFace, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))  {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }
#endif
    if (!systemBus.registerObject(ENABLE_SERVICE_PATH, mp_Enable, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }
    if (!systemBus.registerObject(WAKEUP_SERVICE_PATH, mp_Wakeup, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }

    static QThread t;
    mp_Enable->moveToThread(&t);
    t.start();

    return true;
}

void MainJob::initDriverRepoSource()
{
    QFile fileDriver(DRIVER_REPO_PATH);
    if (fileDriver.open(QIODevice::ReadOnly)) {
        QString info = fileDriver.readAll();
        QStringList lines = info.split("\n");
        foreach (QString line, lines) {
            if (line.contains("pro-driver-packages")) {
                fileDriver.close();
                return;
            }
        }
        fileDriver.close();
    }

    QFile file(DEVICE_REPO_PATH);
    if (QFile::exists(DEVICE_REPO_PATH)) {
        return;
    }
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qInfo() << file.errorString();
        return;
    }

    file.write("deb https://pro-driver-packages.uniontech.com eagle non-free\n");
    file.close();

    QString cmd = "apt update";
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
}

