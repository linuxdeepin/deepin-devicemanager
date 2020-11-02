#include "LoadInfoThread.h"

#include "ReadFilePool.h"
#include "GenerateDevicePool.h"
#include "ZmqOrder.h"

LoadInfoThread::LoadInfoThread()
    : mp_ReadFilePool(new ReadFilePool)
    , mp_GenerateDevicePool(new GenerateDevicePool)
    , m_Running(false)
{

}

void LoadInfoThread::run()
{
    ZmqOrder order;
    if (order.connect()) {
        order.updateData();
    }

    m_Running = true;
    if (mp_ReadFilePool) {
        mp_ReadFilePool->readAllFile();
        mp_ReadFilePool->waitForDone(-1);
    }

    if (mp_GenerateDevicePool) {
        mp_GenerateDevicePool->generateDevice();
        mp_GenerateDevicePool->waitForDone(-1);
    }

    emit finished("finish");
    m_Running = false;
}

bool LoadInfoThread::isRunning()
{
    return m_Running;
}
