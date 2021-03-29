#include "LoadInfoThread.h"

#include <QDebug>
#include <QDateTime>

#include "ReadFilePool.h"
#include "GenerateDevicePool.h"
#include "ZmqOrder.h"

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool(new ReadFilePool)
    , mp_GenerateDevicePool(new GenerateDevicePool)
    , m_Running(false)
    , m_FinishedReadFilePool(false)
    , mp_ZmqOrder(nullptr)
{
    connect(mp_ReadFilePool, &ReadFilePool::finishedAll, this, &LoadInfoThread::slotFinishedReadFilePool);
}

LoadInfoThread::~LoadInfoThread()
{
    if (mp_ReadFilePool) {
        delete mp_ReadFilePool;
        mp_ReadFilePool = nullptr;
    }
    if (mp_GenerateDevicePool) {
        delete mp_GenerateDevicePool;
        mp_GenerateDevicePool = nullptr;
    }
    if (mp_ZmqOrder) {
        delete mp_ZmqOrder;
        mp_ZmqOrder = nullptr;
    }
}


void LoadInfoThread::run()
{
    if (!mp_ZmqOrder) {
        mp_ZmqOrder = new ZmqOrder;
        if (!mp_ZmqOrder->connect()) {
            emit finished("finish");
            m_Running = false;
            return;
        }
    }

    // 请求后台更新信息
    if (!mp_ZmqOrder->reqUpdateUI()) {
        emit finished("finish");
        m_Running = false;
        return;
    }

    m_Running = true;
    if (mp_ReadFilePool) {
        mp_ReadFilePool->readAllFile();
        mp_ReadFilePool->waitForDone(-1);
    }

    // 为了保证上面那个线程池完全结束
    long long begin = QDateTime::currentMSecsSinceEpoch();
    while (true) {
        if (m_FinishedReadFilePool)
            break;
        long long end = QDateTime::currentSecsSinceEpoch();
        if (end - begin > 6000) {
            break;
        }
        usleep(100);
    }
    m_FinishedReadFilePool = false;
    if (mp_GenerateDevicePool) {
        mp_GenerateDevicePool->generateDevice();
        mp_GenerateDevicePool->waitForDone(-1);
    }
    emit finished("finish");
    m_Running = false;
}

void LoadInfoThread::slotFinishedReadFilePool(const QString &info)
{
    m_FinishedReadFilePool = true;
}

void LoadInfoThread::setFramework(const QString &arch)
{
    // 设置架构
    mp_ReadFilePool->setFramework(arch);
}

