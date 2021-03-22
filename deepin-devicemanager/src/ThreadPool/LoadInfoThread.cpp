#include "LoadInfoThread.h"

#include <QDebug>

#include "ReadFilePool.h"
#include "GenerateDevicePool.h"
#include "ZmqOrder.h"

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool(new ReadFilePool)
    , mp_GenerateDevicePool(new GenerateDevicePool)
    , m_Running(false)
    , m_FinishedReadFilePool(false)
{
    connect(mp_ReadFilePool, &ReadFilePool::finishedAll, this, &LoadInfoThread::slotFinishedReadFilePool);
}

LoadInfoThread::~LoadInfoThread()
{
    delete mp_ReadFilePool;
    delete mp_GenerateDevicePool;
}


void LoadInfoThread::run()
{
    // 请求后台更新信息
    ZmqOrder order;
    if (!order.connect() || !order.reqUpdateUI()) {
        emit finished("finish");
        return;
    }

    m_Running = true;
    if (mp_ReadFilePool) {
        mp_ReadFilePool->readAllFile();
        mp_ReadFilePool->waitForDone(-1);
    }

    // 为了保证上面那个线程池完全结束
    while (true) {
        if (m_FinishedReadFilePool)
            break;
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

