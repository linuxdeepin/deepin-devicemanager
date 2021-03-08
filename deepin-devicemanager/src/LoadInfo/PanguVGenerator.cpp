#include "PanguVGenerator.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "DeviceComputer.h"

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

void PanguVGenerator::getDiskInfoFromLshw()
{
    QString modelStr = "";
    const QList<QMap<QString, QString>> listMapInfo = DeviceManager::instance()->cmdInfo("bootdevice");
    if (listMapInfo.size() > 0) {
        QMap<QString, QString> map = listMapInfo[0];
        modelStr = map["Model"];
    }

    const QList<QMap<QString, QString>> lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            continue;
        }

        // PanguV的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*dIt).keys()) {
            tempMap.insert(key, (*dIt)[key]);
        }

//        qDebug() << tempMap["product"] << " ***** " << modelStr << " " << (tempMap["product"] == modelStr);
        // HW写死
        if (tempMap["product"] == modelStr) {
            // 应HW的要求，将描述固定为   Universal Flash Storage
            tempMap["description"] = "Universal Flash Storage";
            // 应HW的要求，添加interface   UFS 3.0
            tempMap["interface"] = "UFS 3.0";
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}

void PanguVGenerator::generatorComputerDevice()
{
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer device;

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device.setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        device.setType(sysInfo[0]["description"]);
//        device.setVendor(sysInfo[0]["vendor"]);
        device.setName(sysInfo[0]["product"]);
    }

    // setOsDescription
    QString os = "UnionTech OS";
    DSysInfo::UosEdition type = DSysInfo::uosEditionType();
    if (DSysInfo::UosProfessional == type) {
        os =  "UnionTech OS Desktop 20 Professional";
    } else if (DSysInfo::UosHome == type) {
        os =  "UnionTech OS Desktop 20 Home";
    } else if (DSysInfo::UosCommunity == type) {
        os =  "Deepin 20";
    } else if (DSysInfo::UosEnterprise == type) {
        os =  "UnionTech OS Server 20 Enterprise";
    } else if (DSysInfo::UosEnterpriseC == type) {
        os =  "UnionTech OS Server 20 Enterprise-C";
    } else if (DSysInfo::UosEuler == type) {
        os =  "UnionTech OS Server 20 Euler";
    }
    device.setOsDescription(os);


    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
        int index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
            info.insert(index, reg.cap(1));
        }
        device.setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}

void PanguVGenerator::generatorBluetoothDevice()
{
    DeviceGenerator::generatorBluetoothDevice();
}

void PanguVGenerator::generatorNetworkDevice()
{
    DeviceGenerator::generatorNetworkDevice();
}

