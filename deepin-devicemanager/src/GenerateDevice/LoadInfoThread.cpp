// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LoadInfoThread.h"
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"
#include "DBusInterface.h"
#include "DeviceManager.h"
#include "DDLog.h"
#include "commondefine.h"

#include <DApplication>

#include <QLoggingCategory>
#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>

#include<malloc.h>
#include <unistd.h>

DWIDGET_USE_NAMESPACE
using namespace DDLog;

static constexpr int kServerIdlePollMs = 100;         // 轮询间隔
static constexpr int kMaxWaitServerIdleMs = 40000;    // 整体等待上限，按真实耗时计算
static constexpr int kServerStatusTimeoutMs = 30000;  // 单次查询超时，需大于服务端刷新上限
static constexpr int kPollStartGraceMs = 300;         // 留给 refreshInfo 消息派发的时间
static constexpr int kServerProbeRetryMs = 200;       // 查询失败（服务未就绪）时的重试间隔
static constexpr int kServerProbeRetryCount = 5;      // 查询失败时的重试次数

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool()
    , mp_GenerateDevicePool()
{
    connect(&mp_ReadFilePool, &GetInfoPool::finishedAll, this, &LoadInfoThread::slotFinishedReadFilePool);
}

LoadInfoThread::~LoadInfoThread()
{
    long long begin = QDateTime::currentMSecsSinceEpoch();
    while (m_Running)
    {
        long long end = QDateTime::currentMSecsSinceEpoch();
        if (end - begin > 1000)
            _exit(0);
        usleep(100);
    }
    mp_ReadFilePool.deleteLater();
    mp_GenerateDevicePool.deleteLater();
}


void LoadInfoThread::run()
{
    m_Running = true;

    // 每次加载都先让服务端刷一遍缓存，再读。
    DBusInterface::getInstance()->refreshInfo();

    // 服务端正在全量刷新时缓存可能只写了一半，先等它刷完再读。
    waitServerRefreshFinished();

    mp_ReadFilePool.getAllInfo();
    mp_ReadFilePool.waitForDone(-1);

    bool readDataFlag = false;
    int readNum = 0;
    while (!readDataFlag) {
        if (m_FinishedReadFilePool) {
            mp_ReadFilePool.getAllInfo();
            mp_ReadFilePool.waitForDone(-1);
        }
        // 为了保证上面那个线程池完全结束
        long long begin = QDateTime::currentMSecsSinceEpoch();
        while (true) {
            readDataFlag = !DeviceManager::instance()->cmdInfo("dmidecode4").isEmpty();
            if (readDataFlag && m_FinishedReadFilePool)
                break;
            long long end = QDateTime::currentMSecsSinceEpoch();
            if (end - begin > 4000)
                break;
            usleep(100);
        }
        if ((++readNum) > 3) {
            if (m_FinishedReadFilePool && !readDataFlag) {
                mp_ReadFilePool.getAllInfo();
                mp_ReadFilePool.waitForDone(-1);
                begin = QDateTime::currentMSecsSinceEpoch();
                while (true) {
                    if (m_FinishedReadFilePool)
                        break;
                    long long end = QDateTime::currentMSecsSinceEpoch();
                    if (end - begin > 4000)
                        break;
                    usleep(100);
                }
            }
            break;
        }
    }

    m_FinishedReadFilePool = false;
    // generateDevice() 内部已等待所有设备生成任务完成，无需再次 waitForDone
    mp_GenerateDevicePool.generateDevice();

    emit finished("finish");
    m_Running = false;
}

void LoadInfoThread::waitServerRefreshFinished()
{
    // refreshInfo() 是异步调用，消息派发需要一点时间。不等这一会儿就去查，
    // 第一轮很可能查到"空闲"，结果又读到了旧缓存。
    msleep(kPollStartGraceMs);

    QElapsedTimer timer;
    timer.start();

    QString info;
    int probeFailCount = 0;
    while (timer.elapsed() < kMaxWaitServerIdleMs) {
        info.clear();
        // 传入更长的超时：服务端刷新期间不处理 D-Bus 调用，查询会一直排队。
        // 但不能超过剩余预算，否则单次调用就能把总预算撑破（例如预算剩 1s
        // 却发出一个 30s 的调用，实际要等 30s 才返回）。
        const int remainMs = kMaxWaitServerIdleMs - static_cast<int>(timer.elapsed());
        const int timeoutMs = qMin(kServerStatusTimeoutMs, remainMs);
        if (!DBusInterface::getInstance()->getInfo("is_server_running", info, timeoutMs)) {
            // 服务尚未在总线上注册（如刚开机还没起来）时重试几次，
            // 重试无果则当作服务不存在，继续读本地缓存。
            if (++probeFailCount > kServerProbeRetryCount)
                return;
            msleep(kServerProbeRetryMs);
            continue;
        }
        probeFailCount = 0;
        if (info == kServerNotRunning || info == kCheckAuthFailed) {
            return;
        }
        msleep(kServerIdlePollMs);
    }

    qCWarning(appLog) << "wait server refresh finished timeout after" << timer.elapsed()
                      << "ms, load cache anyway";
}

void LoadInfoThread::slotFinishedReadFilePool(const QString &)
{
    m_FinishedReadFilePool = true;
}

void LoadInfoThread::setFramework(const QString &arch)
{
    // 设置架构
    mp_ReadFilePool.setFramework(arch);
}

