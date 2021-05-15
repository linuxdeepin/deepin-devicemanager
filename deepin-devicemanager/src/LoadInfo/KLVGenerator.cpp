#include "KLVGenerator.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceKeyboard.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceCpu.h"

#include<QDebug>

KLVGenerator::KLVGenerator()
{

}

void KLVGenerator::generatorComputerDevice()
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
        //device.setVendor(sysInfo[0]["vendor"]);
        device.setName(sysInfo[0]["product"]);
    }

    // setOsDescription
    QString os = "UOS";
    DSysInfo::DeepinType type = DSysInfo::deepinType();
    if (DSysInfo::DeepinProfessional == type) {
        os =  "UOS 20";
    } else if (DSysInfo::DeepinPersonal == type) {
        os =  "UOS 20 Home";
    } else if (DSysInfo::DeepinDesktop == type) {
        os =  "Deepin 20 Beta";
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

        // 按照华为要求,去除所有括号中的信息
        reg.setPattern("\\([\\s\\S]*\\)");
        index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
        }

        device.setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}

void KLVGenerator::generatorGpuDevice()
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

void KLVGenerator::generatorMonitorDevice()
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "LCD");
//    mapInfo.insert("Vendor", "HUAWEI");
    mapInfo.insert("CurResolution", "2160x1440@60Hz");
    mapInfo.insert("SupportResolution", "2160x1440@60Hz");
    mapInfo.insert("Size", "14 Inch");
//    mapInfo.insert("Date", "2019年7月");

    DeviceMonitor monitor;
    monitor.setInfoFromSelfDefine(mapInfo);
    DeviceManager::instance()->addMonitor(monitor);
}

void KLVGenerator::generatorAudioDevice()
{
    getAudioInfoFromCatAudio();
}

void KLVGenerator::generatorPowerDevice()
{
    const QList<QMap<QString, QString> > &daemon = DeviceManager::instance()->cmdInfo("Daemon");
    bool hasDaemon = false;
    // 守护进程信息
    if (daemon.size() > 0) {
        hasDaemon = true;
    }
    // 电池信息
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("upower");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DevicePower device;
        if (!device.setKLUInfoFromUpower(*it)) {
            continue;
        }
        if (hasDaemon) {
            // KLU的问题特殊处理
            QMap<QString, QString> tempMap = daemon[0];

            // HW写死
            // 应HW的要求，将电量极低时执行固定为  Suspend
            tempMap["critical-action"] = "Suspend";


//            device.setDaemonInfo(daemon[0]);
            device.setDaemonInfo(tempMap);
        }
        DeviceManager::instance()->addPowerDevice(device);
    }
}

void KLVGenerator::getMemoryInfoFromLshw()
{
    const QList<QMap<QString, QString>> lstMemory = DeviceManager::instance()->cmdInfo("lshw_memory");
    if (lstMemory.size() == 0) {
        return;
    } else if (lstMemory.size() == 1) {
        DeviceMemory device;
        device.setInfoFromLshw(lstMemory[0]);
        DeviceManager::instance()->addMemoryDevice(device);
    } else if (lstMemory.size() > 1) {
        QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();
        for (; it != lstMemory.end(); ++it) {
            if ((!(*it)["size"].contains("GiB") && !(*it)["size"].contains("MiB")) || (*it)["description"] == "System Memory") {
                continue;
            }
            DeviceMemory device;
            device.setKLUInfoFromLshw(*it);
            DeviceManager::instance()->addMemoryDevice(device);
        }
    }
}
void KLVGenerator::getKeyboardInfoFromHwinfo()
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

void KLVGenerator::getOthersInfoFromHwinfo()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }

        bool isOtherDevice = true;
        QString curBus = (*it)["SysFS BusID"];
        curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        const QStringList &lstBusId = DeviceManager::instance()->getBusId();
        if (lstBusId.indexOf(curBus) != -1) {
            isOtherDevice = false;
        }
        if ((*it)["Driver"].contains("usb-storage")) {
            isOtherDevice = false;
        }

        if (isOtherDevice) {
            DeviceOthers device;
            device.setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
        }
    }
}

void KLVGenerator::getDiskInfoFromHwinfo()
{
    const QList<QMap<QString, QString>> lstDisk = DeviceManager::instance()->cmdInfo("hwinfo_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            continue;
        }
        DeviceStorage device;
        if (device.setKLUHwinfoInfo(*dIt) && device.isValid()) {
            DeviceManager::instance()->addStorageDeivce(device);
            addBusIDFromHwinfo((*dIt)["SysFS BusID"]);
        }
    }
}

void KLVGenerator::getDiskInfoFromLshw()
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

        // KLV的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*dIt).keys()) {
            tempMap.insert(key, (*dIt)[key]);
        }
        // HW写死
        // 应HW的要求，将描述固定为   Universal Flash Storage
        tempMap["description"] = "Universal Flash Storage";
        // 应HW的要求，添加interface   UFS 3.1
        tempMap["interface"] = "UFS 3.1";

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}

void KLVGenerator::getDiskInfoFromLsblk()
{
    const QList<QMap<QString, QString>> lstblk = DeviceManager::instance()->cmdInfo("lsblk_d");
    if (lstblk.size() > 0) {
        QMap<QString, QString>::const_iterator it = lstblk[0].begin();
        for (; it != lstblk[0].end(); ++it) {
            DeviceManager::instance()->setKLUStorageDeviceMediaType(it.key(), it.value());
        }
    }
}

void KLVGenerator::getDiskInfoFromSmartCtl()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("smart");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // 按照华为的需求，如果是固态硬盘就不显示转速
        if (tempMap["Rotation Rate"] == "Solid State Device") {
            tempMap["Rotation Rate"] = "KLU_SSD";
        }

        DeviceManager::instance()->setStorageInfoFromSmartctl(tempMap["ln"], tempMap);
    }
}

void KLVGenerator::getAudioInfoFromCatAudio()
{
    const QList<QMap<QString, QString>> lstAudio = DeviceManager::instance()->cmdInfo("cat_audio");

    QList<QMap<QString, QString> >::const_iterator it = lstAudio.begin();
    for (; it != lstAudio.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceAudio device;
        device.setInfoFromCatAudio(*it);
        DeviceManager::instance()->addAudioDevice(device);
    }
}

void KLVGenerator::getBluetoothInfoFromCatWifiInfo()
{
    const QList<QMap<QString, QString> >  &lstWifiInfo = DeviceManager::instance()->cmdInfo("wifiinfo");
    if (lstWifiInfo.size() == 0) {
        return;
    }
    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();

    for (; it != lstWifiInfo.end(); ++it) {
        if ((*it).size() < 3) {
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // cat /sys/hisys/wal/wifi_devices_info  获取结果为 HUAWEI HI103
        if (tempMap["Chip Type"].contains("HUAWEI", Qt::CaseInsensitive)) {
            tempMap["Chip Type"] = tempMap["Chip Type"].remove("HUAWEI").trimmed();
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = "HISILICON";
        tempMap["Type"] = "Bluetooth Device";

        DeviceManager::instance()->setBluetoothInfoFromWifiInfo(tempMap);
    }
}

void KLVGenerator::getNetworkInfoFromCatWifiInfo()
{
    const QList<QMap<QString, QString> >  &lstWifiInfo = DeviceManager::instance()->cmdInfo("wifiinfo");
    if (lstWifiInfo.size() == 0) {
        return;
    }
    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();

    for (; it != lstWifiInfo.end(); ++it) {
        if ((*it).size() < 3) {
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // cat /sys/hisys/wal/wifi_devices_info  获取结果为 HUAWEI HI103
        if (tempMap["Chip Type"].contains("HUAWEI", Qt::CaseInsensitive)) {
            tempMap["Chip Type"] = tempMap["Chip Type"].remove("HUAWEI").trimmed();
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = "HISILICON";
        tempMap["Type"] = "Network Device";

        DeviceManager::instance()->setNetworkInfoFromWifiInfo(tempMap);
    }
}

void KLVGenerator::generatorCpuDevice()
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
        device.setCpuInfo(lscpu, lshw, dmidecode4[i], *it, lstCatCpu.size(), coreNum);

        // HW要求将CPU界面的表格上 频率 改为 最大频率，不显示范围
        if (dmidecode4[i].find("Max Speed") != dmidecode4[i].end()) {
            device.setfrequencyShowRange(false, dmidecode4[i]["Max Speed"]);
        }


        DeviceManager::instance()->addCpuDevice(device);
    }
}

void KLVGenerator::generatorBluetoothDevice()
{
    DeviceGenerator::generatorBluetoothDevice();

    // HW 要求修改名称,制造商以及类型
    getBluetoothInfoFromCatWifiInfo();
}

void KLVGenerator::generatorNetworkDevice()
{
    DeviceGenerator::generatorNetworkDevice();

    // HW 要求修改名称,制造商以及类型
    getNetworkInfoFromCatWifiInfo();
}
