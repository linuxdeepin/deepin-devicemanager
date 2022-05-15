#include "DriverScanner.h"
#include "DeviceManager.h"
#include "HttpDriverInterface.h"

#include <QDebug>

#include <unistd.h>


DriverScanner::DriverScanner(QObject *parent)
    : QThread(parent)
    , m_IsStop(false)
{

}

void DriverScanner::run()
{
    HttpDriverInterface *hdi  = HttpDriverInterface::getInstance();
    connect(hdi, &HttpDriverInterface::sigRequestFinished, this, [ = ]() {
        m_IsStop = true;
    });

    foreach (DriverInfo *info, m_ListDriverInfo) {
        if (!m_IsStop) {
            hdi->getRequest(info);
            emit scanInfo(info->name(), 100 / m_ListDriverInfo.size());
            sleep(1);
        } else {
            emit scanFinished(SR_NETWORD_ERR);
        }

        //        emit scanInfo(info->name(), 0);
        //        usleep(10000);
        //        emit scanInfo(info->name(), 20);
        //        usleep(10000);
        //        emit scanInfo(info->name(), 40);
        //        usleep(10000);
        //        emit scanInfo(info->name(), 60);
        //        usleep(10000);
        //        emit scanInfo(info->name(), 80);
        //        usleep(10000);
        //        emit scanInfo(info->name(), 100);
    }
    if (!m_IsStop) {
        // 扫描结束
        usleep(10000);
        emit scanFinished(SR_SUCESS);
    }
}

void DriverScanner::setDriverList(QList<DriverInfo *> lstInfo)
{
    m_ListDriverInfo = lstInfo;
    m_IsStop = false;
}

