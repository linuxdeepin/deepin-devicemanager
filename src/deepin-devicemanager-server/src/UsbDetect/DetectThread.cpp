#include "DetectThread.h"
#include "DetectJob.h"

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_DectectJob(new DetectJob(this))
{
    // 连接槽函数
    connect(mp_DectectJob, &DetectJob::usbChanged, this, &DetectThread::slotUsbChanged);
}

void DetectThread::run()
{
    if (mp_DectectJob) {
        mp_DectectJob->working();
    }
}

void DetectThread::setUSleepElapse(unsigned int elapse)
{
    if (mp_DectectJob)
        mp_DectectJob->setUSleepElapse(elapse);
}

void DetectThread::slotUsbChanged()
{
    emit usbChanged();
}
