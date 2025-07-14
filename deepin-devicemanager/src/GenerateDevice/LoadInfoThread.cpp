// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LoadInfoThread.h"
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"
#include "DBusInterface.h"
#include "DeviceManager.h"
#include "DDLog.h"

#include <DApplication>

#include <QLoggingCategory>
#include <QDateTime>
#include <QTimer>

#include<malloc.h>
#include <unistd.h>

using namespace DDLog;
DWIDGET_USE_NAMESPACE
static bool firstLoadFlag = true;

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool()
    , mp_GenerateDevicePool()
    , m_Running(false)
    , m_FinishedReadFilePool(false)
    , m_Start(true)
{
    qCDebug(appLog) << "LoadInfoThread constructor";
    connect(&mp_ReadFilePool, &GetInfoPool::finishedAll, this, &LoadInfoThread::slotFinishedReadFilePool);
}

LoadInfoThread::~LoadInfoThread()
{
    qCDebug(appLog) << "LoadInfoThread destructor start";
    long long begin = QDateTime::currentMSecsSinceEpoch();
    while (m_Running)
    {
        // qCDebug(appLog) << "LoadInfoThread destructor waiting for thread to finish";
        long long end = QDateTime::currentMSecsSinceEpoch();
        if (end - begin > 1000) {
            qCWarning(appLog) << "LoadInfoThread destructor timeout, force exit";
            _exit(0);
        }
        usleep(100);
    }
    mp_ReadFilePool.deleteLater();
    mp_GenerateDevicePool.deleteLater();
    qCDebug(appLog) << "LoadInfoThread destructor end";
}


void LoadInfoThread::run()
{
    qCDebug(appLog) << "LoadInfoThread::run start";
    // 判断后台是否正处理update状态
    QString info;
    DBusInterface::getInstance()->getInfo("is_server_running", info);
    qCDebug(appLog) << "LoadInfoThread::run is_server_running:" << info;
    // 请求后台更新信息
    m_Running = true;
    if (!info.toInt()) {
        qCDebug(appLog) << "LoadInfoThread::run server is not running, start to get info";
        m_Start = false;
        mp_ReadFilePool.getAllInfo();
        mp_ReadFilePool.waitForDone(-1);

        bool readDataFlag = false;
        int readNum = 0;
        while (!readDataFlag) {
            // qCDebug(appLog) << "LoadInfoThread::run waiting for read data, readNum:" << readNum;
            if (m_FinishedReadFilePool) {
                // qCDebug(appLog) << "LoadInfoThread::run finished read file pool, get all info again";
                mp_ReadFilePool.getAllInfo();
                mp_ReadFilePool.waitForDone(-1);
            }
            // 为了保证上面那个线程池完全结束
            long long begin = QDateTime::currentMSecsSinceEpoch();
            while (true) {
                readDataFlag = !DeviceManager::instance()->cmdInfo("dmidecode4").isEmpty();
                if (readDataFlag && m_FinishedReadFilePool) {
                    // qCDebug(appLog) << "LoadInfoThread::run read data finished";
                    break;
                }
                long long end = QDateTime::currentMSecsSinceEpoch();
                if (end - begin > 4000) {
                    // qCWarning(appLog) << "LoadInfoThread::run read data timeout";
                    break;
                }
                usleep(100);
            }
            if ((++readNum) > 3) {
                // qCWarning(appLog) << "LoadInfoThread::run read data retry timeout";
                if (m_FinishedReadFilePool && !readDataFlag) {
                    // qCDebug(appLog) << "LoadInfoThread::run finished read file pool, but read data failed, get all info again";
                    mp_ReadFilePool.getAllInfo();
                    mp_ReadFilePool.waitForDone(-1);
                    begin = QDateTime::currentMSecsSinceEpoch();
                    while (true) {
                        // qCDebug(appLog) << "LoadInfoThread::run waiting for read file pool finished";
                        if (m_FinishedReadFilePool) {
                            // qCDebug(appLog) << "LoadInfoThread::run read file pool finished";
                            break;
                        }
                        long long end = QDateTime::currentMSecsSinceEpoch();
                        if (end - begin > 4000) {
                            // qCWarning(appLog) << "LoadInfoThread::run wait for read file pool finished timeout";
                            break;
                        }
                        usleep(100);
                    }
                }
                break;
            }
        }

        m_FinishedReadFilePool = false;
        mp_GenerateDevicePool.generateDevice();
        mp_GenerateDevicePool.waitForDone(-1);
    } else {
        qCDebug(appLog) << "LoadInfoThread::run server is running, do nothing";
    }

    emit finished("finish");
    m_Running = false;
    qCDebug(appLog) << "LoadInfoThread::run end";
}

void LoadInfoThread::slotFinishedReadFilePool(const QString &)
{
    qCDebug(appLog) << "LoadInfoThread::slotFinishedReadFilePool";
    m_FinishedReadFilePool = true;
    // 首次加载刷新
    if (firstLoadFlag) {
        qCDebug(appLog) << "LoadInfoThread::slotFinishedReadFilePool first load, emit finishedReadFilePool";
        firstLoadFlag = false;
        emit finishedReadFilePool();
    }
}

void LoadInfoThread::setFramework(const QString &arch)
{
    qCDebug(appLog) << "LoadInfoThread::setFramework, arch:" << arch;
    // 设置架构
    mp_ReadFilePool.setFramework(arch);
}

