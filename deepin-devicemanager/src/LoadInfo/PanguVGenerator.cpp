// 项目自身文件
#include "PanguVGenerator.h"

// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"

PanguVGenerator::PanguVGenerator()
{

}

void PanguVGenerator::generatorMonitorDevice()
{
    const QList<QMap<QString, QString>> lstMapHDMI = DeviceManager::instance()->cmdInfo("EDID_HDMI");
    QList<QMap<QString, QString> >::const_iterator itHDMI = lstMapHDMI.begin();
    for (; itHDMI != lstMapHDMI.end(); ++itHDMI) {
        if ((*itHDMI).size() < 1) {
            continue;
        }
        DeviceMonitor *device = new  DeviceMonitor();
        device->setInfoFromEdid(*itHDMI);
        DeviceManager::instance()->addMonitor(device);
    }

    const QList<QMap<QString, QString>> lstMapVGA = DeviceManager::instance()->cmdInfo("EDID_VGA");
    QList<QMap<QString, QString> >::const_iterator it = lstMapVGA.begin();
    for (; it != lstMapVGA.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceMonitor *device = new DeviceMonitor();
        device->setInfoFromEdid(*it);
        DeviceManager::instance()->addMonitor(device);
    }
}
