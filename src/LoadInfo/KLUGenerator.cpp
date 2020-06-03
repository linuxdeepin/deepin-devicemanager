#include "KLUGenerator.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceKeyboard.h"

KLUGenerator::KLUGenerator()
{

}

void KLUGenerator::generatorGpuDevice()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("gpuinfo");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceGpu device;
        device.setGpuInfo(*it);
        DeviceManager::instance()->addGpuDevice(device);
    }
}

void KLUGenerator::generatorMonitorDevice()
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "Notebook monitor");
    mapInfo.insert("Vendor", "HUAWEI");
    mapInfo.insert("CurResolution", "2160x1440@60Hz");
    mapInfo.insert("SupportResolution", "2160x1440@60Hz");
    mapInfo.insert("Size", "296x197mm");
    mapInfo.insert("Date", "2019年7月");

    DeviceMonitor monitor;
    monitor.setInfoFromSelfDefine(mapInfo);
    DeviceManager::instance()->addMonitor(monitor);
}

void KLUGenerator::getKeyboardInfoFromHwinfo()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hwinfo_keyboard");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it).contains("Device Files")) {
            DeviceKeyboard device;
            device.setKLUInfoFromHwinfo(*it);
            DeviceManager::instance()->addKeyboardDevice(device);
            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}
