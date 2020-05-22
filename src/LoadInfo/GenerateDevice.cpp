#include "GenerateDevice.h"
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


GenerateDevice::GenerateDevice()
{

}
GenerateDevice::~GenerateDevice()
{

}

void GenerateDevice::generatorComputerDevice()
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

void GenerateDevice::generatorCpuDevice()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstCatCpu  = cmdInfo["cat_cpuinfo"];
    if (lstCatCpu.size() == 0) {
        return;
    }

    const QMap<QString, QString> &lscpu = cmdInfo["lscpu"][0];
    const QMap<QString, QString> &lshw = cmdInfo["lshw_cpu"][0];
    const QMap<QString, QString> &dmidecode = cmdInfo["dmidecode4"][0];

    QList<QMap<QString, QString> >::const_iterator it = lstCatCpu.begin();
    for (; it != lstCatCpu.end(); ++it) {
        DeviceCpu device;
        device.setCpuInfo(lscpu, lshw, dmidecode, *it);
        DeviceManager::instance()->addCpuDevice(device);
    }
}

void GenerateDevice::generatorBiosDevice()
{
    getBiosInfo();
    getSystemInfo();
    getBaseBoardInfo();
    getChassisInfo();
    getMemoryInfo();
}

void GenerateDevice::generatorMemoryDevice()
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
            if ((*it)["description"] == "DIMM [empty]" || (*it)["description"] == "[empty]" || (*it)["description"] == "DDR4 [empty]" || (*it)["description"] == "System Memory") {
                continue;
            }
            DeviceMemory device;
            device.setInfoFromLshw(*it);
            DeviceManager::instance()->addMemoryDevice(device);
        }
    }

    // 加载从dmidecode获取的信息
    const QList< QMap<QString, QString> > &dmiMemory  = cmdInfo["dmidecode17"];
    QList<QMap<QString, QString> >::const_iterator dIt = dmiMemory.begin();
    for (; dIt != dmiMemory.end(); ++dIt) {
        if ((*dIt).size() < 2 || (*dIt)["size"] == "No Module Installed") {
            continue;
        }
        DeviceManager::instance()->setMemoryInfoFromDmidecode(*dIt);
    }
}

void GenerateDevice::generatorDiskDevice()
{
    // 添加从hwinfo中获取的信息
    getDiskInfoFromHwinfo();
    // 添加从lshw中获取的信息
    getDiskInfoFromLshw();

    getDiskInfoFromLsblk();
    getDiskInfoFromSmartCtl();
}

void GenerateDevice::generatorGpuDevice()
{
    getGpuInfoFromHwinfo();
    getGpuInfoFromLshw();
    getGpuInfoFromXrandr();
    getGpuSizeFromDmesg();
}

void GenerateDevice::generatorMonitorDevice()
{
    getMonitorInfoFromHwinfo();
    getMonitorInfoFromXrandrVerbose();
    getMonitorRefreshRateFromXrandr();
}

void GenerateDevice::generatorNetworkDevice()
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

void GenerateDevice::generatorAudioDevice()
{
    getAudioInfoFromHwinfo();
    getAudioInfoFromLshw();
}

void GenerateDevice::generatorBluetoothDevice()
{
    getBluetoothInfoFromHciconfig();
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();
}

void GenerateDevice::generatorKeyboardDevice()
{
    getKeyboardInfoFromHwinfo();
    getKeyboardInfoFromLshw();
    getKeyboardInfoFromCatDevices();
}

void GenerateDevice::generatorMouseDevice()
{
    getMouseInfoFromHwinfo();
    getMouseInfoFromLshw();
    getMouseInfoFromCatDevices();
}

void GenerateDevice::generatorPrinterDevice()
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

void GenerateDevice::getBiosInfo()
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
void GenerateDevice::getSystemInfo()
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
void GenerateDevice::getBaseBoardInfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList<QMap<QString, QString>> lstInfo = cmdInfo["dmidecode2"];
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        DeviceBios device;
        device.setBaseBoardInfo(*it, "");
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void GenerateDevice::getChassisInfo()
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
void GenerateDevice::getMemoryInfo()
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

void GenerateDevice::getDiskInfoFromHwinfo()
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
        }
    }
}

void GenerateDevice::getDiskInfoFromLshw()
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

void GenerateDevice::getDiskInfoFromLsblk()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    if (cmdInfo["lsblk_d"].size() > 0) {
        QMap<QString, QString>::const_iterator it = cmdInfo["lsblk_d"][0].begin();
        for (; it != cmdInfo["lsblk_d"][0].end(); ++it) {
            DeviceManager::instance()->setStorageDeviceMediaType(it.key(), it.value());
        }
    }
}
void GenerateDevice::getDiskInfoFromSmartCtl()
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

void GenerateDevice::getGpuInfoFromHwinfo()
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
    }
}

void GenerateDevice::getGpuInfoFromLshw()
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

void GenerateDevice::getGpuInfoFromXrandr()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["xrandr"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setGpuInfoFromLshw(*it);
    }
}
void GenerateDevice::getGpuSizeFromDmesg()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    if (cmdInfo["dmesg"].size() > 0) {
        DeviceManager::instance()->setGpuSizeFromDmesg(cmdInfo["dmesg"][0]["Size"]);
    }
}

void GenerateDevice::getMonitorInfoFromHwinfo()
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
    }
}
void GenerateDevice::getMonitorInfoFromXrandrVerbose()
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
void GenerateDevice::getMonitorRefreshRateFromXrandr()
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

void GenerateDevice::getAudioInfoFromHwinfo()
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
    }
}
void GenerateDevice::getAudioInfoFromLshw()
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

void GenerateDevice::getBluetoothInfoFromHciconfig()
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
void GenerateDevice::getBlueToothInfoFromHwinfo()
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
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb") {
            DeviceManager::instance()->setBluetoothInfoFromHwinfo(*it);
        }
    }
}
void GenerateDevice::getBluetoothInfoFromLshw()
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
void GenerateDevice::getKeyboardInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_keyboard"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceKeyboard device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addKeyboardDevice(device);
    }
}
void GenerateDevice::getKeyboardInfoFromLshw()
{

}
void GenerateDevice::getKeyboardInfoFromCatDevices()
{

}
void GenerateDevice::getMouseInfoFromHwinfo()
{
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = CmdTool::getCmdInfo();
    const QList< QMap<QString, QString> > &lstMap  = cmdInfo["hwinfo_mouse"];
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceMouse device;
        device.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMouseDevice(device);
    }
}
void GenerateDevice::getMouseInfoFromLshw()
{

}
void GenerateDevice::getMouseInfoFromCatDevices()
{

}
