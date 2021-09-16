#include "DetectThread.h"
#include "MonitorUsb.h"

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_MonitorUsb(new MonitorUsb())
{
    // 连接槽函数
    connect(mp_MonitorUsb, SIGNAL(usbChanged()), this, SIGNAL(usbChanged()), Qt::QueuedConnection);
}

void DetectThread::run()
{
    if (mp_MonitorUsb) {
        mp_MonitorUsb->monitor();
    }
}
