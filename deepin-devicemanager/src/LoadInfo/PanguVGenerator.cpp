#include "PanguVGenerator.h"
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
        DeviceMonitor device;
        device.setInfoFromEdid(*itHDMI);
        DeviceManager::instance()->addMonitor(device);
    }

    const QList<QMap<QString, QString>> lstMapVGA = DeviceManager::instance()->cmdInfo("EDID_VGA");
    QList<QMap<QString, QString> >::const_iterator it = lstMapVGA.begin();
    for (; it != lstMapVGA.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceMonitor device;
        device.setInfoFromEdid(*it);
        DeviceManager::instance()->addMonitor(device);
    }
}

void PanguVGenerator::getDiskInfoFromSmartCtl()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("smart");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }

        // PanguV的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // 按照华为的需求，如果是固态硬盘就不显示转速
        if (tempMap["Rotation Rate"] == "Solid State Device") {
            tempMap["Rotation Rate"] = "PanGuV_SSD";
        }

        DeviceManager::instance()->setStorageInfoFromSmartctl(tempMap["ln"], tempMap);
    }
}
