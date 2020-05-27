#include "DeviceGenerator.h"
#include "CmdTool.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMouse.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceKeyboard.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"

#include <QDebug>

QStringList DeviceGenerator::m_ListBusID = {};

void DeviceGenerator::clear()
{
    m_ListBusID.clear();
}

DeviceGenerator::DeviceGenerator()
{

}
DeviceGenerator::~DeviceGenerator()
{

}

void DeviceGenerator::generatorComputerDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();

    DeviceComputer device;

    // home url
    if (cmdInfo["cat_os_release"].size() > 0) {
        QString value = cmdInfo["cat_os_release"][0]["HOME_URL"];
        device.setHomeUrl(value.replace("\"", ""));
    }

    // name type
    if (cmdInfo["lshw_system"].size() > 0) {
        device.setType(cmdInfo["lshw_system"][0]["description"]);
        device.setVendor(cmdInfo["lshw_system"][0]["vendor"]);
        device.setName(cmdInfo["lshw_system"][0]["product"]);
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
    if (cmdInfo["cat_version"].size() > 0) {
        QString info = cmdInfo["cat_version"][0]["OS"].trimmed();
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

void DeviceGenerator::generatorCpuDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstCatCpu  = cmdInfo["cat_cpuinfo"];
    if (lstCatCpu.size() == 0) {
        return;
    }

    const QMap<QString, QString> &lscpu = cmdInfo["lscpu"].size() > 0 ? cmdInfo["lscpu"][0] : QMap<QString, QString>();
    const QMap<QString, QString> &lshw = cmdInfo["lshw_cpu"].size() > 0 ? cmdInfo["lshw_cpu"][0] : QMap<QString, QString>();
    const QMap<QString, QString> &dmidecode = cmdInfo["dmidecode4"].size() > 0 ? cmdInfo["dmidecode4"][0] : QMap<QString, QString>();

    QList<QMap<QString, QString> >::const_iterator it = lstCatCpu.begin();
    for (; it != lstCatCpu.end(); ++it) {
        DeviceCpu device;
        device.setCpuInfo(lscpu, lshw, dmidecode, *it);
        DeviceManager::instance()->addCpuDevice(device);
    }
}

void DeviceGenerator::generatorBiosDevice()
{
    getBiosInfo();
    getSystemInfo();
    getBaseBoardInfo();
    getChassisInfo();
    getBiosMemoryInfo();
}

void DeviceGenerator::generatorMemoryDevice()
{
    getMemoryInfoFromLshw();
    getMemoryInfoFromDmidecode();
}

void DeviceGenerator::generatorDiskDevice()
{
    // 添加从hwinfo中获取的信息
    getDiskInfoFromHwinfo();
    // 添加从lshw中获取的信息
    getDiskInfoFromLshw();

    getDiskInfoFromLsblk();
    getDiskInfoFromSmartCtl();
}

void DeviceGenerator::generatorGpuDevice()
{
    getGpuInfoFromHwinfo();
    getGpuInfoFromLshw();
    getGpuInfoFromXrandr();
    getGpuSizeFromDmesg();
}

void DeviceGenerator::generatorMonitorDevice()
{
    getMonitorInfoFromHwinfo();
    getMonitorInfoFromXrandrVerbose();
    getMonitorRefreshRateFromXrandr();
}

void DeviceGenerator::generatorNetworkDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["lshw_network"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceNetwork device;
        device.setInfoFromLshw(*it);
        DeviceManager::instance()->addNetworkDevice(device);
    }
}

void DeviceGenerator::generatorAudioDevice()
{
    getAudioInfoFromHwinfo();
    getAudioInfoFromLshw();
}

void DeviceGenerator::generatorBluetoothDevice()
{
    getBluetoothInfoFromHciconfig();
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();
}

void DeviceGenerator::generatorKeyboardDevice()
{
    getKeyboardInfoFromHwinfo();
    getKeyboardInfoFromLshw();
    getKeyboardInfoFromCatDevices();
}

void DeviceGenerator::generatorMouseDevice()
{
    getMouseInfoFromHwinfo();
    getMouseInfoFromLshw();
    getMouseInfoFromCatDevices();
}

void DeviceGenerator::generatorPrinterDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["printer"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DevicePrint device;
        device.setInfo(*it);
        DeviceManager::instance()->addPrintDevice(device);
    }
}

void DeviceGenerator::generatorCameraDevice()
{
    getImageInfoFromHwinfo();
    getImageInfoFromLshw();
}

void DeviceGenerator::generatorCdromDevice()
{
    getCdromInfoFromHwinfo();
    getCdromInfoFromLshw();
}

void DeviceGenerator::generatorOthersDevice()
{
    getOthersInfoFromHwinfo();
    getOthersInfoFromLshw();
}

void DeviceGenerator::generatorPowerDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    bool hasDaemon = false;
    // 守护进程信息
    if (cmdInfo.find("Daemon") != cmdInfo.end() && cmdInfo["Daemon"].size() > 0) {
        hasDaemon = true;
    }
    // 电池或这电源信息
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["upower"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DevicePower device;
        device.setInfoFromUpower(*it);
        if (hasDaemon) {
            device.setDaemonInfo(cmdInfo["Daemon"][0]);
        }
        DeviceManager::instance()->addPowerDevice(device);
    }
}

void DeviceGenerator::getBiosInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode0"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setBiosInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void DeviceGenerator::getSystemInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode1"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setSystemInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void DeviceGenerator::getBaseBoardInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode2"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setBaseBoardInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void DeviceGenerator::getChassisInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode3"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setChassisInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void DeviceGenerator::getBiosMemoryInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode16"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setMemoryInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceGenerator::getMemoryInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMemory  = cmdInfo["lshw_memory"];
    if (lstMemory.size() == 0) {
        return;
    } else if (lstMemory.size() == 1) {
        DeviceMemory device;
        device.setInfoFromLshw(lstMemory[0]);
        DeviceManager::instance()->addMemoryDevice(device);
    } else if (lstMemory.size() > 1) {
        QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();
        for (; it != lstMemory.end(); ++it) {
            if (!(*it)["size"].contains("GiB") || (*it)["description"] == "System Memory") {
                continue;
            }
            DeviceMemory device;
            device.setInfoFromLshw(*it);
            DeviceManager::instance()->addMemoryDevice(device);
        }
    }
}
void DeviceGenerator::getMemoryInfoFromDmidecode()
{
    // 加载从dmidecode获取的信息
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &dmiMemory  = cmdInfo["dmidecode17"];
    QList<QMap<QString, QString> >::const_iterator dIt = dmiMemory.begin();
    for (; dIt != dmiMemory.end(); ++dIt) {
        if ((*dIt).size() < 2 || (*dIt)["size"] == "No Module Installed") {
            continue;
        }
        DeviceManager::instance()->setMemoryInfoFromDmidecode(*dIt);
    }
}

void DeviceGenerator::getDiskInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstDisk  = cmdInfo["hwinfo_disk"];
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            continue;
        }
        DeviceStorage device;
        if (device.setHwinfoInfo(*dIt) && device.isValid()) {
            DeviceManager::instance()->addStorageDeivce(device);
            getBusIDFromHwinfo((*dIt)["SysFS BusID"]);
        }
    }
}

void DeviceGenerator::getDiskInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstDisk  = cmdInfo["lshw_disk"];
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            continue;
        }
        DeviceManager::instance()->addLshwinfoIntoStorageDevice(*dIt);
    }
}

void DeviceGenerator::getDiskInfoFromLsblk()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    if (cmdInfo["lsblk_d"].size() > 0) {
        QMap<QString, QString>::const_iterator it = cmdInfo["lsblk_d"][0].begin();
        for (; it != cmdInfo["lsblk_d"][0].end(); ++it) {
            DeviceManager::instance()->setStorageDeviceMediaType(it.key(), it.value());
        }
    }
}
void DeviceGenerator::getDiskInfoFromSmartCtl()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["smart"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceManager::instance()->setStorageInfoFromSmartctl((*it)["ln"], *it);
    }
}

void DeviceGenerator::getGpuInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_display"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceGpu device;
        device.setHwinfoInfo(*it);
        DeviceManager::instance()->addGpuDevice(device);
        getBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}

void DeviceGenerator::getGpuInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_display"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceManager::instance()->setGpuInfoFromLshw(*it);
    }
}

void DeviceGenerator::getGpuInfoFromXrandr()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["xrandr"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}
void DeviceGenerator::getGpuSizeFromDmesg()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    if (cmdInfo["dmesg"].size() > 0) {
        DeviceManager::instance()->setGpuSizeFromDmesg(cmdInfo["dmesg"][0]["Size"]);
    }
}

void DeviceGenerator::getMonitorInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_monitor"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceMonitor device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMonitor(device);
        getBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}
void DeviceGenerator::getMonitorInfoFromXrandrVerbose()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["xrandr_verbose"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"]);
    }
}
void DeviceGenerator::getMonitorRefreshRateFromXrandr()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["xrandr"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setCurrentResolution((*it)["curResolution"], (*it)["rate"]);
    }
}

void DeviceGenerator::getAudioInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_sound"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceAudio device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addAudioDevice(device);
        getBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}
void DeviceGenerator::getAudioInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_multimedia"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceManager::instance()->setAudioInfoFromLshw(*it);
    }
}

void DeviceGenerator::getBluetoothInfoFromHciconfig()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hciconfig"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceBluetooth device;
        device.setInfoFromHciconfig(*it);
        DeviceManager::instance()->addBluetoothDevice(device);
    }
}
void DeviceGenerator::getBlueToothInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            continue;
        }
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0") {
            if (DeviceManager::instance()->setBluetoothInfoFromHwinfo(*it))
                getBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}
void DeviceGenerator::getBluetoothInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setBluetoothInfoFromLshw(*it);
    }
}
void DeviceGenerator::getKeyboardInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_keyboard"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it).contains("Device Files")) {
            DeviceKeyboard device;
            device.setInfoFromHwinfo(*it);
            DeviceManager::instance()->addKeyboardDevice(device);
            getBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}
void DeviceGenerator::getKeyboardInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setKeyboardInfoFromLshw(*it);
    }
}
void DeviceGenerator::getKeyboardInfoFromCatDevices()
{

}
void DeviceGenerator::getMouseInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_mouse"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }

        // 不让数位板显示在鼠标设备里面(武汉那边的数位板)
        if ((*it)["Device"].contains("PM")) {
            continue;
        }

        DeviceMouse device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMouseDevice(device);
        getBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}
void DeviceGenerator::getMouseInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->addMouseInfoFromLshw(*it);
    }
}
void DeviceGenerator::getMouseInfoFromCatDevices()
{

}

void DeviceGenerator::getImageInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        if ((*it)["Model"].contains("camera", Qt::CaseInsensitive) ||
                (*it)["Device"].contains("camera", Qt::CaseInsensitive) ||
                (*it)["Driver"].contains("uvcvideo", Qt::CaseInsensitive)) {
            DeviceImage device;
            device.setInfoFromHwinfo(*it);
            DeviceManager::instance()->addImageDevice(device);
            getBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}
void DeviceGenerator::getImageInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceManager::instance()->setCameraInfoFromLshw(*it);
    }
}

void DeviceGenerator::getCdromInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_cdrom"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }
        DeviceCdrom device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addCdromDevice(device);
        getBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}
void DeviceGenerator::getCdromInfoFromLshw()
{

}

void DeviceGenerator::getOthersInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            continue;
        }

        bool isOtherDevice = true;
        QString curBus = (*it)["SysFS BusID"];
        curBus.replace(QRegExp("\\.[0-9]{1,2}$"), "");
        if (m_ListBusID.indexOf(curBus) != -1) {
            isOtherDevice = false;
        }

        if (isOtherDevice) {
            DeviceOthers device;
            device.setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
        }
    }
}
void DeviceGenerator::getOthersInfoFromLshw()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["lshw_usb"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceManager::instance()->setOthersDeviceInfoFromLshw(*it);
    }
}

void DeviceGenerator::getBusIDFromHwinfo(const QString &sysfsBusID)
{
    QString busID = sysfsBusID;
    busID.replace(QRegExp("\\.[0-9]+$"), "");

    m_ListBusID.append(busID);
}

