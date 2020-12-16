#include "PanguVGenerator.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "DeviceComputer.h"
#include "DeviceCpu.h"

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

void PanguVGenerator::generatorCpuDevice()
{
    const QList<QMap<QString, QString> >  &lstCatCpu = DeviceManager::instance()->cmdInfo("cat_cpuinfo");
    if (lstCatCpu.size() == 0) {
        return;
    }

    const QList<QMap<QString, QString> >  &lsCpu = DeviceManager::instance()->cmdInfo("lscpu");
    const QMap<QString, QString> &lscpu = lsCpu.size() > 0 ? lsCpu[0] : QMap<QString, QString>();

    const QList<QMap<QString, QString> >  &lshwCpu = DeviceManager::instance()->cmdInfo("lshw_cpu");
    const QMap<QString, QString> &lshw = lshwCpu.size() > 0 ? lshwCpu[0] : QMap<QString, QString>();

    const QList<QMap<QString, QString> >  &dmidecode4 = DeviceManager::instance()->cmdInfo("dmidecode4");
//    const QMap<QString, QString> &dmidecode = dmidecode4.size() > 1 ? dmidecode4[1] : QMap<QString, QString>();

    // calculate core num
    int coreNum = 0;
    QList<int> coreCountList;
    QList<QMap<QString, QString> >::const_iterator dit = dmidecode4.begin();
    for (; dit != dmidecode4.end(); ++dit) {
        if ((*dit).find("Core Count") != (*dit).end()) {
            coreNum += (*dit)["Core Count"].trimmed().toInt();
            coreCountList << (*dit)["Core Count"].trimmed().toInt();  // 统计物理CPU以及其中的核数
        }
    }

    QList<QMap<QString, QString> >::const_iterator it = lstCatCpu.begin();
    for (; it != lstCatCpu.end(); ++it) {
        DeviceCpu device;

        int i = 0;
        int count = 0;   // 累计逻辑CPU数目
        int cpuSize = DeviceManager::instance()->getCPUDevices().size();   // 现已添加的逻辑CPU个数

        // 判断当前逻辑CPU属于哪个物理CPU信息
        while (i < coreCountList.size()) {
            // 从第一个物理CPU开始匹配
            if (count == 0) {
                count += coreCountList[i];
            }

            // 已有逻辑CPU小于已统计的物理CPU中的逻辑个数
            if (cpuSize < count) {
                break;
            } else {
                ++i;
                count += coreCountList[i];  // 累加CPU核数
            }
        }

        // 设置逻辑CPU信息
        device.setCpuInfo(lscpu, lshw, dmidecode4[i + 1], *it, lstCatCpu.size(), coreNum);

        // HW要求将CPU界面的表格上 频率 改为 最大频率，不显示范围
        if (dmidecode4[i + 1].find("Max Speed") != dmidecode4[i + 1].end()) {
            device.setfrequencyShowRange(false, dmidecode4[i + 1]["Max Speed"]);
        }


        DeviceManager::instance()->addCpuDevice(device);
    }
}
