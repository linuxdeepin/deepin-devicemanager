// 项目自身文件
#include "DeviceManager.h"

// Qt库文件
#include <QDebug>
#include <QFile>
#include <QMutexLocker>

// 其它头文件
#include "DeviceCpu.h"
#include "DeviceGpu.h"
#include "DeviceStorage.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceBios.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceOthers.h"
#include "DevicePower.h"
#include "DevicePrint.h"
#include "DeviceOtherPCI.h"
#include "DeviceComputer.h"
#include "DeviceCdrom.h"
#include "DeviceInput.h"
#include "MacroDefinition.h"

DeviceManager    *DeviceManager::sInstance = nullptr;
int DeviceManager::m_CurrentXlsRow = 1;

QMutex addCmdMutex;

DeviceManager::DeviceManager()
    : m_CpuNum(1)
{

}

DeviceManager::~DeviceManager()
{
    clear();
}

void DeviceManager::clear()
{
    // 清除所有命令
    m_cmdInfo.clear();

    // 清除内存中的所有设备指针
    foreach (auto device, m_ListDeviceMouse)
        delete device;

    foreach (auto device, m_ListDeviceStorage)
        delete device;

    foreach (auto device, m_ListDeviceMonitor)
        delete device;

    foreach (auto device, m_ListDeviceBluetooth)
        delete device;

    foreach (auto device, m_ListDeviceAudio)
        delete device;

    foreach (auto device, m_ListDeviceImage)
        delete device;

    foreach (auto device, m_ListDeviceKeyboard)
        delete device;

    foreach (auto device, m_ListDeviceOthers)
        delete device;

    foreach (auto device, m_ListDevicePower)
        delete device;

    foreach (auto device, m_ListDevicePrint)
        delete device;

    foreach (auto device, m_ListDeviceOtherPCI)
        delete device;

    foreach (auto device, m_ListDeviceCdrom)
        delete device;

    foreach (auto device, m_ListDeviceComputer)
        delete device;

    foreach (auto device, m_ListDeviceNetwork)
        delete device;

    foreach (auto device, m_ListDeviceBios)
        delete device;

    foreach (auto device, m_ListDeviceGPU)
        delete device;

    foreach (auto device, m_ListDeviceMemory)
        delete device;

    foreach (auto device, m_ListDeviceCPU)
        delete device;

    // 清空存储设备指针的列表
    m_ListDeviceMouse.clear();
    m_ListDeviceStorage.clear();
    m_ListDeviceMonitor.clear();
    m_ListDeviceBluetooth.clear();
    m_ListDeviceAudio.clear();
    m_ListDeviceImage.clear();
    m_ListDeviceKeyboard.clear();
    m_ListDeviceOthers.clear();
    m_ListDevicePower.clear();
    m_ListDevicePrint.clear();
    m_ListDeviceOtherPCI.clear();
    m_ListDeviceCdrom.clear();
    m_ListDeviceComputer.clear();
    m_ListDeviceNetwork.clear();
    m_ListDeviceBios.clear();
    m_ListDeviceGPU.clear();
    m_ListDeviceMemory.clear();
    m_ListDeviceCPU.clear();
}

const QList<QPair<QString, QString>> &DeviceManager::getDeviceTypes()
{
    // 获取设备类型
    // 清空设备类型列表
    m_ListDeviceType.clear();
    bool addSeperator = false;
    // 添加概况信息
    if (true) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Overview"), "overview##Overview"));
        addSeperator = true;
    }

    // 添加cpu信息
    if (m_ListDeviceCPU.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("CPU"), "cpu##CPU"));
        addSeperator = true;
    }

    if (m_CpuNum > 1) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("CPU quantity"), ""));
        addSeperator = true;
    }

    if (addSeperator) {
        m_ListDeviceType.append(QPair<QString, QString>("Separator", "Separator##Separator"));
        addSeperator = false;
    }

    // 板载接口设备
    if (m_ListDeviceBios.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Motherboard"), "motherboard##Bios"));
        addSeperator = true;
    }

    if (m_ListDeviceMemory.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Memory"), "memory##Memory"));
        addSeperator = true;
    }

    if (m_ListDeviceGPU.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Display Adapter"), "displayadapter##GPU"));
        addSeperator = true;
    }

    if (m_ListDeviceAudio.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Sound Adapter"), "audiodevice##Audio"));
        addSeperator = true;
    }

    if (m_ListDeviceStorage.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Storage"), "storage##Storage"));
        addSeperator = true;
    }

    if (m_ListDeviceOtherPCI.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Other PCI Devices"), "otherpcidevices##OtherPCI"));
        addSeperator = true;
    }

    if (m_ListDevicePower.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Battery"), "battery##Power"));
        addSeperator = true;
    }

    if (addSeperator) {
        m_ListDeviceType.append(QPair<QString, QString>("Separator", "Separator##Separator"));
        addSeperator = false;
    }

    // 网络设备
    if (m_ListDeviceBluetooth.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Bluetooth"), "bluetooth##Bluetooth"));
        addSeperator = true;
    }

    if (m_ListDeviceNetwork.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Network Adapter"), "networkadapter##Network"));
        addSeperator = true;
    }

    if (addSeperator) {
        m_ListDeviceType.append(QPair<QString, QString>("Separator", "Separator##Separator"));
        addSeperator = false;
    }

    // 输入设备
    if (m_ListDeviceMouse.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Mouse"), "mouse##Mouse"));
        addSeperator = true;
    }

    if (m_ListDeviceKeyboard.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Keyboard"), "keyboard##Keyboard"));
        addSeperator = true;
    }

    if (addSeperator) {
        m_ListDeviceType.append(QPair<QString, QString>("Separator", "Separator##Separator"));
    }

    // 外设设备
    if (m_ListDeviceMonitor.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Monitor"), "monitor##Monitor"));
    }

    if (m_ListDeviceCdrom.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("CD-ROM"), "cdrom##Cdrom"));
    }

    if (m_ListDevicePrint.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Printer"), "printer##Print"));
    }

    if (m_ListDeviceImage.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Camera"), "camera##Image"));
    }

    if (m_ListDeviceOthers.size() > 0) {
        m_ListDeviceType.append(QPair<QString, QString>(tr("Other Devices", "Other Input Devices"), "otherdevices##Others"));
    }

    return m_ListDeviceType;
}

void DeviceManager::setDeviceListClass()
{
    // 添加设备类型与设备指针列表的映射关系
    m_DeviceClassMap[tr("CPU")] = m_ListDeviceCPU;
    m_DeviceClassMap[tr("Motherboard")] =  m_ListDeviceBios;
    m_DeviceClassMap[tr("Memory")]  =  m_ListDeviceMemory;
    m_DeviceClassMap[tr("Display Adapter")] =  m_ListDeviceGPU;
    m_DeviceClassMap[tr("Sound Adapter")] =  m_ListDeviceAudio;
    m_DeviceClassMap[tr("Storage")]  =  m_ListDeviceStorage;
    m_DeviceClassMap[tr("Other PCI Devices")] =  m_ListDeviceOtherPCI;
    m_DeviceClassMap[tr("Battery")] =  m_ListDevicePower;
    m_DeviceClassMap[tr("Bluetooth")] =  m_ListDeviceBluetooth;
    m_DeviceClassMap[tr("Network Adapter")]  =  m_ListDeviceNetwork;
    m_DeviceClassMap[tr("Mouse")] =  m_ListDeviceMouse;
    m_DeviceClassMap[tr("Keyboard")]  =  m_ListDeviceKeyboard;
    m_DeviceClassMap[tr("Monitor")] =  m_ListDeviceMonitor;
    m_DeviceClassMap[tr("CD-ROM")] =  m_ListDeviceCdrom;
    m_DeviceClassMap[tr("Printer")] =  m_ListDevicePrint;
    m_DeviceClassMap[tr("Camera")] =  m_ListDeviceImage;
    m_DeviceClassMap[tr("Other Devices", "Other Input Devices")] =  m_ListDeviceOthers;
}

bool DeviceManager::getDeviceList(const QString &name, QList<DeviceBaseInfo *> &lst)
{
    // name为概况Overview时,没有设备列表
    if (name == tr("Overview"))
        return false;

    // 获取设备指针列表
    if (m_DeviceClassMap.find(name) != m_DeviceClassMap.end())
        lst = m_DeviceClassMap[name];

    return true;
}

DeviceBaseInfo *DeviceManager::getBluetoothAtIndex(int index)
{
    if (m_ListDeviceBluetooth.size() <= index)
        return nullptr;
    return m_ListDeviceBluetooth[index];
}

void DeviceManager::addMouseDevice(DeviceInput *const device)
{
    // 如果不是重复设备则添加到设备列表
    m_ListDeviceMouse.append(device);
}

DeviceBaseInfo *DeviceManager::getMouseDevice(const QString &unique_id)
{
    if (unique_id.isEmpty()) {
        return nullptr;
    }
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMouse.begin(); it != m_ListDeviceMouse.end(); ++it) {
        DeviceInput *mouse = dynamic_cast<DeviceInput *>(*it);
        if (mouse && mouse->uniqueID() == unique_id) {
            return *it;
        }
    }
    return nullptr;
}

bool DeviceManager::addMouseInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中添加鼠标信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMouse.begin();
    for (; it != m_ListDeviceMouse.end(); ++it) {
        DeviceInput *device = dynamic_cast<DeviceInput *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromlshw(mapInfo))
            return true;
    }
    return false;
}

void DeviceManager::addCpuDevice(DeviceCpu *const device)
{
    // 添加CPU设备
    m_ListDeviceCPU.append(device);
}

void DeviceManager::addStorageDeivce(DeviceStorage *const device)
{
    m_ListDeviceStorage.append(device);
}

void DeviceManager::addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中添加存储设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device)
            continue;

        if (device->addInfoFromlshw(mapInfo))
            return;
    }
}

void DeviceManager::addLshwinfoIntoNVMEStorageDevice(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中添加NVME存储设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device)
            continue;

        if (device->addNVMEInfoFromlshw(mapInfo))
            return;
    }
}

void DeviceManager::setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo)
{
    // // 从smartctl中添加存储设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device)
            continue;

        if (device->addInfoFromSmartctl(name, mapInfo))
            return;
    }
}

void DeviceManager::mergeDisk()
{
    QMap<QString, QList<int> > allSerialIDs;
    for (int i = 0; i < m_ListDeviceStorage.size(); ++i) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(m_ListDeviceStorage[i]);
        if (!device->getDiskSerialID().isEmpty()) {
            allSerialIDs[device->getDiskSerialID()].append(i);
        }
    }
    for (auto serialIDs : allSerialIDs) {
        if (serialIDs.size() < 2)
            continue;
        DeviceStorage *fDevice = dynamic_cast<DeviceStorage *>(m_ListDeviceStorage[serialIDs[0] ]);
        for (int i = 1; i < serialIDs.size(); ++i) {
            DeviceStorage *curDevice = dynamic_cast<DeviceStorage *>(m_ListDeviceStorage[serialIDs[i] ]);
            fDevice->appendDisk(curDevice);
            m_ListDeviceStorage.removeAt(serialIDs[i]);
            delete curDevice;
        }
    }
}

void DeviceManager::checkDiskSize()
{
    for (int i = 0; i < m_ListDeviceStorage.size(); ++i) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(m_ListDeviceStorage[i]);
        device->checkDiskSize();
    }
}

bool DeviceManager::setStorageDeviceMediaType(const QString &name, const QString &value)
{
    // 设置存储设备介质类型
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device)
            continue;

        // 设置介质类型
        if (device->setMediaType(name, value))
            return true;
    }

    return  false;
}

bool DeviceManager::setKLUStorageDeviceMediaType(const QString &name, const QString &value)
{
    // 设置KLU机器存储设备介质类型
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);

        if (!device)
            continue;

        // 设置KLU介质类型
        if (device->setKLUMediaType(name, value))
            return true;
    }
    return  false;
}

void DeviceManager::addGpuDevice(DeviceGpu *const device)
{
    // 添加显示适配器
    m_ListDeviceGPU.append(device);
}

void DeviceManager::setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中添加显示适配器信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device)
            continue;

        device->setLshwInfo(mapInfo);
    }
}

void DeviceManager::setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo)
{
    // 从xrandr中添加显示适配器信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device)
            continue;

        device->setXrandrInfo(mapInfo);
    }
}

void DeviceManager::setGpuSizeFromDmesg(const QString &info)
{
    // 从dmesg中设置显卡大小
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device)
            continue;

        device->setDmesgInfo(info);
    }
}

void DeviceManager::addMemoryDevice(DeviceMemory *const device)
{
    // 添加内存
    m_ListDeviceMemory.append(device);
}

void DeviceManager::setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    // 从dmidecode中添加内存信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMemory.begin();
    for (; it != m_ListDeviceMemory.end(); ++it) {
        DeviceMemory *device = dynamic_cast<DeviceMemory *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromDmidecode(mapInfo))
            return;
    }
}

void DeviceManager::addMonitor(DeviceMonitor *const device)
{
    // 添加显示设备
    m_ListDeviceMonitor.append(device);
}

void DeviceManager::setMonitorInfoFromXrandr(const QString &main, const QString &edid, const QString &rate)
{
    // 从xrandr中添加显示设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        DeviceMonitor *device = dynamic_cast<DeviceMonitor *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromXradr(main, edid, rate))
            return;
    }
}

void DeviceManager::addBiosDevice(DeviceBios *const device)
{
    // 添加主板信息
    m_ListDeviceBios.append(device);
}

void DeviceManager::setLanguageInfo(const QMap<QString, QString> &mapInfo)
{
    // 设置语言信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBios.begin();
    for (; it != m_ListDeviceBios.end(); ++it) {
        DeviceBios *device = dynamic_cast<DeviceBios *>(*it);
        if (!device)
            continue;

        if (device->setBiosLanguageInfo(mapInfo))
            return;
    }
}

void DeviceManager::addBluetoothDevice(DeviceBluetooth *const device)
{
    m_ListDeviceBluetooth.append(device);
}

void DeviceManager::setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中获取蓝牙信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin();
    for (; it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromLshw(mapInfo))
            return;
    }
}

bool DeviceManager::setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 从hwinfo中获取蓝牙信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin();
    for (; it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromHwinfo(mapInfo))
            return true;
    }
    return false;
}

bool DeviceManager::setBluetoothInfoFromWifiInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin();
    for (; it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(*it);
        if (device->setInfoFromWifiInfo(mapInfo)) {
            return true;
        }
    }
    return false;
}

DeviceBaseInfo *DeviceManager::getBluetoothDevice(const QString &unique_id)
{
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin(); it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *bt = dynamic_cast<DeviceBluetooth *>(*it);
        if (bt && bt->uniqueID() == unique_id) {
            return *it;
        }
    }
    return nullptr;
}

void DeviceManager::addAudioDevice(DeviceAudio *const device)
{
    m_ListDeviceAudio.append(device);
}

void DeviceManager::deleteDisableDuplicate_AudioDevice(void)
{
    if (m_ListDeviceAudio.size() > 0) {
        for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin(); it != m_ListDeviceAudio.end(); ++it) {
            DeviceAudio *audio_1 = dynamic_cast<DeviceAudio *>(*it);
            //QString tpath = audio_1->uniqueID();
            // 判断该设备是否已经存在，
            if (!audio_1->enable()) {
                for (QList<DeviceBaseInfo *>::iterator it2 = m_ListDeviceAudio.begin(); it2 != m_ListDeviceAudio.end(); ++it2) {
                    DeviceAudio *audio_2 = dynamic_cast<DeviceAudio *>(*it2);
                    if (audio_2->name() == audio_1->name())
                        if (audio_2->enable())
                            m_ListDeviceAudio.removeOne(audio_2);
                }
            }
        }

    }
}

DeviceBaseInfo *DeviceManager::getAudioDevice(const QString &path)
{
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin(); it != m_ListDeviceAudio.end(); ++it) {
        DeviceAudio *audio = dynamic_cast<DeviceAudio *>(*it);
        QString tpath = audio->uniqueID();
        // 判断该设备是否已经存在，1.1:1.1 -> 1.1:1.0
        if (audio && path == tpath.replace(QRegExp("[1-9]$"), "0")) {
            return *it;
        }
         QString syspath = audio->sysPath();   //remove dumplicate syspath
         if (audio && path == syspath) {
             return *it;
         }
    }
    return nullptr;
}

void DeviceManager::setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中获取音频适配器信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromLshw(mapInfo))
            return;
    }
}

void DeviceManager::setAudioChipFromDmesg(const QString &info)
{
    // 从dmesg中获取声卡芯片型号
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(*it);
        if (!device)
            continue;

        device->setAudioChipFromDmesg(info);
    }
}

void DeviceManager::addNetworkDevice(DeviceNetwork *const device)
{
    // 添加网络适配器
    m_ListDeviceNetwork.append(device);
}

bool DeviceManager::setNetworkInfoFromWifiInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {

        DeviceNetwork *device = dynamic_cast<DeviceNetwork *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromWifiInfo(mapInfo)) {
            return true;
        }
    }
    return false;
}

DeviceBaseInfo *DeviceManager::getNetworkDevice(const QString &unique_id)
{
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceNetwork.begin(); it != m_ListDeviceNetwork.end(); ++it) {
        DeviceNetwork *net = dynamic_cast<DeviceNetwork *>(*it);
        if (!unique_id.isEmpty() && net && net->uniqueID() == unique_id) {
            return *it;
        }
    }
    return nullptr;
}

void DeviceManager::correctNetworkLinkStatus(QString linkStatus, QString networkDriver)
{
    if (m_ListDeviceNetwork.size() == 0)
        return;
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {
        DeviceNetwork *device = dynamic_cast<DeviceNetwork *>(*it);
        if (!device)
            continue;
        if (networkDriver == device->logicalName())
            device->correctCurrentLinkStatus(linkStatus);
    }
}

QStringList DeviceManager::networkDriver()
{
    m_networkDriver.clear();
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {
        DeviceNetwork *device = dynamic_cast<DeviceNetwork *>(*it);
        if (!device)
            continue;
        //保存各个网卡的逻辑名称，用于判断具体网卡
        m_networkDriver.append(device->logicalName());
    }
    return m_networkDriver;
}

void DeviceManager::correctPowerInfo(const QMap<QString, QMap<QString, QString>> &mapInfo)
{
    if (m_ListDevicePower.size() == 0)
        return;
    QList<DeviceBaseInfo *>::iterator it = m_ListDevicePower.begin();
    for (; it != m_ListDevicePower.end(); ++it) {
        DevicePower *device = dynamic_cast<DevicePower *>(*it);
        if (!device)
            continue;

        //根据获取到的数据，重新设置电池信息
        device->setInfoFromUpower(mapInfo["upower"]);
        device->setDaemonInfo(mapInfo["Daemon"]);
    }
}

void DeviceManager::addImageDevice(DeviceImage *const device)
{
    // 添加图像设备
    m_ListDeviceImage.append(device);
}

DeviceBaseInfo *DeviceManager::getImageDevice(const QString &unique_id)
{
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceImage.begin(); it != m_ListDeviceImage.end(); ++it) {
        DeviceImage *image = dynamic_cast<DeviceImage *>(*it);
        if (image && image->uniqueID() == unique_id) {
            return *it;
        }
    }
    return nullptr;
}

void DeviceManager::setCameraInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw获取图像设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceImage.begin();
    for (; it != m_ListDeviceImage.end(); ++it) {
        DeviceImage *device = dynamic_cast<DeviceImage *>(*it);
        if (!device)
            continue;

        device->setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::addKeyboardDevice(DeviceInput *const device)
{
    // 添加键盘
    m_ListDeviceKeyboard.append(device);
}

void DeviceManager::setKeyboardInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 从lshw获取键盘信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceKeyboard.begin();
    for (; it != m_ListDeviceKeyboard.end(); ++it) {
        DeviceInput *device = dynamic_cast<DeviceInput *>(*it);
        if (!device)
            continue;

        if (device->setInfoFromlshw(mapInfo))
            return;
    }
}

void DeviceManager::addOthersDevice(DeviceOthers *const device)
{
    // 添加其他设备
    bool isOtherDevice = true;
    foreach (auto disk, m_ListDeviceStorage) {
        DeviceStorage *deviceDisk = dynamic_cast<DeviceStorage *>(disk);
        if (!deviceDisk)
            continue;

        // 排除存储设备
        if (deviceDisk->keyFromStorage() == device->logicalName() && device->logicalName() != "") {
            isOtherDevice = false;
            break;
        }
    }

    // 添加其他设备
    if (isOtherDevice)
        m_ListDeviceOthers.append(device);
}

DeviceBaseInfo *DeviceManager::getOthersDevice(const QString &unique_id)
{
    if (unique_id.isEmpty()) {
        return nullptr;
    }
    for (QList<DeviceBaseInfo *>::iterator it = m_ListDeviceOthers.begin(); it != m_ListDeviceOthers.end(); ++it) {
        DeviceOthers *other = dynamic_cast<DeviceOthers *>(*it);
        if (other && other->uniqueID() == unique_id) {
            return *it;
        }
    }
    return nullptr;
}

void DeviceManager::addOthersDeviceFromHwinfo(DeviceOthers *const device)
{
    // 从hwinfo中获取其他设备信息
    foreach (auto cur, m_ListDeviceOthers) {
        DeviceOthers *deviceOthers = dynamic_cast<DeviceOthers *>(cur);
        if (!deviceOthers)
            continue;

        if (deviceOthers->busInfo() == device->busInfo() && deviceOthers->busInfo() != "")
            return;
    }
    m_ListDeviceOthers.append(device);
}

void DeviceManager::setOthersDeviceInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    //从lshw中获取其他设备信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceOthers.begin();
    for (; it != m_ListDeviceOthers.end(); ++it) {
        DeviceOthers *device = dynamic_cast<DeviceOthers *>(*it);
        if (!device)
            continue;

        device->setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::setCpuRefreshInfoFromlscpu(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceCPU.begin();
    for (; it != m_ListDeviceCPU.end(); ++it) {
        DeviceCpu *device = dynamic_cast<DeviceCpu *>(*it);
        if (!device)
            continue;

        device->setCurFreq(mapInfo["CPU MHz"]);
    }
}

void DeviceManager::addPowerDevice(DevicePower *const device)
{
    // 添加电池设备
    m_ListDevicePower.append(device);
}

void DeviceManager::addPrintDevice(DevicePrint *const device)
{
    // 添加打印机信息
    m_ListDevicePrint.append(device);
}

void DeviceManager::addOtherPCIDevice(DeviceOtherPCI *const device)
{
    // 添加其他PCI设备
    m_ListDeviceOtherPCI.append(device);
}

void DeviceManager::addComputerDevice(DeviceComputer *const device)
{
    // 添加计算机设备
    m_ListDeviceComputer.append(device);
}

void DeviceManager::addCdromDevice(DeviceCdrom *const device)
{
    // 添加CDROM
    m_ListDeviceCdrom.append(device);
}

void DeviceManager::addLshwinfoIntoCdromDevice(const QMap<QString, QString> &mapInfo)
{
    // 从lshw中添加CDROM信息
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceCdrom.begin();
    for (; it != m_ListDeviceCdrom.end(); ++it) {
        DeviceCdrom *device = dynamic_cast<DeviceCdrom *>(*it);
        if (!device)
            continue;

        device->setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::addBusId(const QStringList &busId)
{
    // 添加设备总线信息
    m_BusIdList.append(busId);
}

const QStringList &DeviceManager::getBusId()
{
    return m_BusIdList;
}

void DeviceManager::addCmdInfo(const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo)
{
    QMutexLocker locker(&addCmdMutex);
    // 添加命令信息
    foreach (const QString &key, cmdInfo.keys()) {
        if (m_cmdInfo.find(key) == m_cmdInfo.end())
            m_cmdInfo.insert(key, cmdInfo[key]);
        else
            m_cmdInfo[key].append(cmdInfo[key]);
    }
}

const QList<QMap<QString, QString>> &DeviceManager::cmdInfo(const QString &key)
{
    QMutexLocker locker(&addCmdMutex);
    return m_cmdInfo[key];
}

bool DeviceManager::exportToTxt(const QString &filePath)
{
    // 导出设备信息到txt文件
    QFile txtFile(filePath);
    if (false == txtFile.open(QIODevice::WriteOnly))
        return false;

    QTextStream out(&txtFile);
    overviewToTxt(out);
    EXPORT_TO_TXT(out, m_ListDeviceCPU, QObject::tr("CPU"), QObject::tr("No CPU found"));
    EXPORT_TO_TXT(out, m_ListDeviceBios, QObject::tr("Motherboard"), QObject::tr("No motherboard found"));
    EXPORT_TO_TXT(out, m_ListDeviceMemory, QObject::tr("Memory"), QObject::tr("No memory found"));
    EXPORT_TO_TXT(out, m_ListDeviceStorage, QObject::tr("Storage"), QObject::tr("No disk found"));
    EXPORT_TO_TXT(out, m_ListDeviceGPU, QObject::tr("Display Adapter"), QObject::tr("No GPU found"));
    EXPORT_TO_TXT(out, m_ListDeviceMonitor, QObject::tr("Monitor"), QObject::tr("No monitor found"));
    EXPORT_TO_TXT(out, m_ListDeviceNetwork, QObject::tr("Network Adapter"), QObject::tr("No network adapter found"));
    EXPORT_TO_TXT(out, m_ListDeviceAudio, QObject::tr("Sound Adapter"), QObject::tr("No audio device found"));
    EXPORT_TO_TXT(out, m_ListDeviceBluetooth, QObject::tr("Bluetooth"), QObject::tr("No Bluetooth device found"));
    EXPORT_TO_TXT(out, m_ListDeviceOtherPCI, QObject::tr("Other PCI Devices"), QObject::tr("No other PCI devices found"));
    EXPORT_TO_TXT(out, m_ListDevicePower, QObject::tr("Power"), QObject::tr("No battery found"));
    EXPORT_TO_TXT(out, m_ListDeviceKeyboard, QObject::tr("Keyboard"), QObject::tr("No keyboard found"));
    EXPORT_TO_TXT(out, m_ListDeviceMouse, QObject::tr("Mouse"), QObject::tr("No mouse found"));
    EXPORT_TO_TXT(out, m_ListDevicePrint, QObject::tr("Printer"), QObject::tr("No printer found"));
    EXPORT_TO_TXT(out, m_ListDeviceImage, QObject::tr("Camera"), QObject::tr("No camera found"));
    EXPORT_TO_TXT(out, m_ListDeviceCdrom, QObject::tr("CD-ROM"), QObject::tr("No CD-ROM found"));
    EXPORT_TO_TXT(out, m_ListDeviceOthers, QObject::tr("Other Devices"), QObject::tr("No other devices found"));
    txtFile.close();

    return true;
}

bool DeviceManager::exportToXlsx(const QString &filePath)
{
    // 导出设备信息到xlsx表格
    QXlsx::Document xlsx;
    QXlsx::Format boldFont;
    overviewToXlsx(xlsx, boldFont);
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceCPU, QObject::tr("CPU"), QObject::tr("No CPU found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceBios, QObject::tr("Motherboard"), QObject::tr("No motherboard found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceMemory, QObject::tr("Memory"), QObject::tr("No memory found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceStorage, QObject::tr("Storage"), QObject::tr("No disk found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceGPU, QObject::tr("Display Adapter"), QObject::tr("No GPU found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceMonitor, QObject::tr("Monitor"), QObject::tr("No monitor found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceNetwork, QObject::tr("Network Adapter"), QObject::tr("No network adapter found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceAudio, QObject::tr("Sound Adapter"), QObject::tr("No audio device found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceBluetooth, QObject::tr("Bluetooth"), QObject::tr("No Bluetooth device found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceOtherPCI, QObject::tr("Other PCI Devices"), QObject::tr("No other PCI devices found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDevicePower, QObject::tr("Power"), QObject::tr("No battery found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceKeyboard, QObject::tr("Keyboard"), QObject::tr("No keyboard found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceMouse, QObject::tr("Mouse"), QObject::tr("No mouse found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDevicePrint, QObject::tr("Printer"), QObject::tr("No printer found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceImage, QObject::tr("Camera"), QObject::tr("No camera found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceCdrom, QObject::tr("CD-ROM"), QObject::tr("No CD-ROM found"));
    EXPORT_TO_XLSX(xlsx, boldFont, m_ListDeviceOthers, QObject::tr("Other Devices"), QObject::tr("No other devices found"));
    m_CurrentXlsRow = 1;
    xlsx.saveAs(filePath);

    return true;
}

bool DeviceManager::exportToDoc(const QString &filePath)
{
    // 导出设备信息到doc文件
    Docx::Document doc(":/template.docx");
    overviewToDoc(doc);
    EXPORT_TO_DOC(doc, m_ListDeviceCPU, QObject::tr("CPU"), QObject::tr("No CPU found"));
    EXPORT_TO_DOC(doc, m_ListDeviceBios, QObject::tr("Motherboard"), QObject::tr("No motherboard found"));
    EXPORT_TO_DOC(doc, m_ListDeviceMemory, QObject::tr("Memory"), QObject::tr("No memory found"));
    EXPORT_TO_DOC(doc, m_ListDeviceStorage, QObject::tr("Storage"), QObject::tr("No disk found"));
    EXPORT_TO_DOC(doc, m_ListDeviceGPU, QObject::tr("Display Adapter"), QObject::tr("No GPU found"));
    EXPORT_TO_DOC(doc, m_ListDeviceMonitor, QObject::tr("Monitor"), QObject::tr("No monitor found"));
    EXPORT_TO_DOC(doc, m_ListDeviceNetwork, QObject::tr("Network Adapter"), QObject::tr("No network adapter found"));
    EXPORT_TO_DOC(doc, m_ListDeviceAudio, QObject::tr("Sound Adapter"), QObject::tr("No audio device found"));
    EXPORT_TO_DOC(doc, m_ListDeviceBluetooth, QObject::tr("Bluetooth"), QObject::tr("No Bluetooth device found"));
    EXPORT_TO_DOC(doc, m_ListDeviceOtherPCI, QObject::tr("Other PCI Devices"), QObject::tr("No other PCI devices found"));
    EXPORT_TO_DOC(doc, m_ListDevicePower, QObject::tr("Power"), QObject::tr("No battery found"));
    EXPORT_TO_DOC(doc, m_ListDeviceKeyboard, QObject::tr("Keyboard"), QObject::tr("No keyboard found"));
    EXPORT_TO_DOC(doc, m_ListDeviceMouse, QObject::tr("Mouse"), QObject::tr("No mouse found"));
    EXPORT_TO_DOC(doc, m_ListDevicePrint, QObject::tr("Printer"), QObject::tr("No printer found"));
    EXPORT_TO_DOC(doc, m_ListDeviceImage, QObject::tr("Camera"), QObject::tr("No camera found"));
    EXPORT_TO_DOC(doc, m_ListDeviceCdrom, QObject::tr("CD-ROM"), QObject::tr("No CD-ROM found"));
    EXPORT_TO_DOC(doc, m_ListDeviceOthers, QObject::tr("Other Devices"), QObject::tr("No other devices found"));

    doc.save(filePath);

    return true;
}

bool DeviceManager::exportToHtml(const QString &filePath)
{
    // 导出设备信息到html文件
    QFile html(filePath);
    if (false == html.open(QIODevice::WriteOnly))
        return false;

    html.write("<!DOCTYPE html>\n");
    html.write("<html>\n");
    html.write("<body>\n");

    overviewToHtml(html);

    EXPORT_TO_HTML(html, m_ListDeviceCPU, QObject::tr("CPU"), QObject::tr("No CPU found"));
    EXPORT_TO_HTML(html, m_ListDeviceBios, QObject::tr("Motherboard"), QObject::tr("No motherboard found"));
    EXPORT_TO_HTML(html, m_ListDeviceMemory, QObject::tr("Memory"), QObject::tr("No memory found"));
    EXPORT_TO_HTML(html, m_ListDeviceStorage, QObject::tr("Storage"), QObject::tr("No disk found"));
    EXPORT_TO_HTML(html, m_ListDeviceGPU, QObject::tr("Display Adapter"), QObject::tr("No GPU found"));
    EXPORT_TO_HTML(html, m_ListDeviceMonitor, QObject::tr("Monitor"), QObject::tr("No monitor found"));
    EXPORT_TO_HTML(html, m_ListDeviceNetwork, QObject::tr("Network Adapter"), QObject::tr("No network adapter found"));
    EXPORT_TO_HTML(html, m_ListDeviceAudio, QObject::tr("Sound Adapter"), QObject::tr("No audio device found"));
    EXPORT_TO_HTML(html, m_ListDeviceBluetooth, QObject::tr("Bluetooth"), QObject::tr("No Bluetooth device found"));
    EXPORT_TO_HTML(html, m_ListDeviceOtherPCI, QObject::tr("Other PCI Devices"), QObject::tr("No other PCI devices found"));
    EXPORT_TO_HTML(html, m_ListDevicePower, QObject::tr("Power"), QObject::tr("No battery found"));
    EXPORT_TO_HTML(html, m_ListDeviceKeyboard, QObject::tr("Keyboard"), QObject::tr("No keyboard found"));
    EXPORT_TO_HTML(html, m_ListDeviceMouse, QObject::tr("Mouse"), QObject::tr("No mouse found"));
    EXPORT_TO_HTML(html, m_ListDevicePrint, QObject::tr("Printer"), QObject::tr("No printer found"));
    EXPORT_TO_HTML(html, m_ListDeviceImage, QObject::tr("Camera"), QObject::tr("No camera found"));
    EXPORT_TO_HTML(html, m_ListDeviceCdrom, QObject::tr("CD-ROM"), QObject::tr("No CD-ROM found"));
    EXPORT_TO_HTML(html, m_ListDeviceOthers, QObject::tr("Other Devices"), QObject::tr("No other devices found"));

    html.write("</body>\n");                                                \
    html.write("</html>\n");

    html.close();

    return true;
}

int DeviceManager::currentXlsRow()
{
    return m_CurrentXlsRow++;
}

void DeviceManager::overviewToTxt(QTextStream &out)
{
    // 概况信息导出到txt
    out << "[" << tr("Overview") << "]\n-------------------------------------------------";
    out << "\n";

    // 导出设备信息
    out.setFieldWidth(21);
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
    out << tr("Device") + ": ";
    out.setFieldWidth(0);
    out << m_OveriewMap["Overview"];
    out << "\n";

    // 导出操作系统信息
    out.setFieldWidth(21);
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
    out << tr("OS") + ": ";
    out.setFieldWidth(0);
    out << m_OveriewMap["OS"];
    out << "\n";

    foreach (auto iter, m_ListDeviceType) {
        if (iter.first == tr("Overview"))
            continue;

        // 导出设备的概况信息
        if (m_OveriewMap.find(iter.first) != m_OveriewMap.end()) {
            out.setFieldWidth(21);
            out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
            out << iter.first + ": ";
            out.setFieldWidth(0);
            out << m_OveriewMap[iter.first];
            out << "\n";
        }
    }
    out << "\n";
}



void DeviceManager::overviewToHtml(QFile &html)
{
    // 导出概况信息到html
    html.write((QString("<h2>") + "[" + tr("Overview") + "]" + "</h2>").toUtf8());
    QDomDocument doc;

    // 导出设备信息到html
    infoToHtml(doc, "Device", m_OveriewMap["Overview"]);

    // 导出操作系统信息到html
    infoToHtml(doc, "OS", m_OveriewMap["OS"]);

    // 导出设备概况信息到html
    foreach (auto iter, m_ListDeviceType) {
        if (iter.first == tr("Overview"))
            continue;

        if (m_OveriewMap.find(iter.first) != m_OveriewMap.end())
            infoToHtml(doc, iter.first, m_OveriewMap[iter.first]);
    }

    doc.appendChild(doc.createElement("br"));
    html.write(doc.toString().toStdString().data());
}

void DeviceManager::overviewToDoc(Docx::Document &doc)
{
    // 导出概况信息到doc文件
    doc.addHeading("[" + tr("Overview") + "]");
    doc.addParagraph("-------------------------------------------------");

    // 导出设备信息到doc
    QString line = tr("Device") + ":  " + m_OveriewMap["Overview"];
    doc.addParagraph(line);

    // 导出操作系统信息到doc
    line = tr("OS") + ":  " + m_OveriewMap["OS"];
    doc.addParagraph(line);

    // 导出设备概况信息到doc
    foreach (auto iter, m_ListDeviceType) {
        if (iter.first == tr("Overview"))
            continue;

        if (m_OveriewMap.find(iter.first) != m_OveriewMap.end()) {
            line = iter.first + ":  " + m_OveriewMap[iter.first];
            doc.addParagraph(line);
        }
    }
    doc.addParagraph("\n");
}

void DeviceManager::overviewToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    // 导出概况信息到xlsx文件
    boldFont.setFontBold(true);
    xlsx.write(m_CurrentXlsRow++, 1, "[" + tr("Overview") + "]", boldFont);

    // 设置字体格式
    boldFont.setFontBold(false);
    boldFont.setFontSize(10);

    // 导出设备信息到xlsx文件
    xlsx.write(m_CurrentXlsRow, 1, tr("Device"), boldFont);
    xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap["Overview"], boldFont);

    // 导出操作系统信息到xlsx文件
    xlsx.write(m_CurrentXlsRow, 1, tr("OS"), boldFont);
    xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap["OS"], boldFont);

    // 导出设备概况信息到xlsx文件
    foreach (auto iter, m_ListDeviceType) {
        if (iter.first == tr("Overview"))
            continue;

        if (m_OveriewMap.find(iter.first) != m_OveriewMap.end()) {

            xlsx.write(m_CurrentXlsRow, 1, iter.first, boldFont);
            xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap[iter.first], boldFont);
        }
    }
    m_CurrentXlsRow++;
}

void DeviceManager::infoToHtml(QDomDocument &doc, const QString &key, const QString &value)
{
    // 导出设备信息到html
    // 创建表格,设置表格属性
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    QDomElement tr = doc.createElement("tr");

    // 创建单元格,设置单元格属性
    QDomElement td = doc.createElement("td");
    td.setAttribute("width", "15%");
    td.setAttribute("style", "text-align:left;");

    // 设置单元格内容
    QDomText nameText = doc.createTextNode(QObject::tr(key.toStdString().c_str()) + ": ");
    td.appendChild(nameText);
    tr.appendChild(td);

    // 创建单元格,设置单元格属性
    QDomElement td2 = doc.createElement("td");
    td2.setAttribute("width", "85%");

    // 设置单元格内容
    QDomText valueText;
    valueText = doc.createTextNode(value);
    td2.appendChild(valueText);

    tr.appendChild(td2);

    // 插入表格
    table.appendChild(tr);

    doc.appendChild(table);
}

const QMap<QString, QString>  &DeviceManager::getDeviceOverview()
{
    // 获取所有设备的概况信息
    m_OveriewMap.clear();

    auto iter = m_DeviceClassMap.begin();

    // 根据设备指针类,获取所有设备概况信息
    for (; iter != m_DeviceClassMap.end(); ++iter) {
        if (m_OveriewMap.find(iter.key()) == m_OveriewMap.end()) {
            foreach (auto device, iter.value()) {
                QString ov = device->getOverviewInfo();

                // 每一类别获取首个设备概况信息
                if (false == ov.isEmpty()) {
                    if (m_OveriewMap.find(iter.key()) == m_OveriewMap.end()) {
                        m_OveriewMap[iter.key()] = ov;
                    } else {
                        // 每一类别获取第n个设备概况信息
                        m_OveriewMap[iter.key()] += "/";
                        m_OveriewMap[iter.key()] += ov;
                    }
                }
            }
        }
    }

    // 设备名称 and 操作系统
    if (m_ListDeviceComputer.size() > 0) {
        m_OveriewMap["Overview"] = m_ListDeviceComputer[0]->getOverviewInfo();
        m_OveriewMap["OS"] = dynamic_cast<DeviceComputer *>(m_ListDeviceComputer[0])->getOSInfo();
    }


    // CPU 概况显示 样式"Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz (四核 / 四逻辑处理器)"
    if (!m_ListDeviceCPU.isEmpty())
        m_OveriewMap[tr("CPU")] = m_ListDeviceCPU[0] ->getOverviewInfo();

    if (m_CpuNum > 1)
        m_OveriewMap[tr("CPU quantity")] = QString::number(m_CpuNum);

    return m_OveriewMap;
}

const QMap<QString, QMap<QString, QStringList> > &DeviceManager::getDeviceDriverPool()
{
    // 获取所有设备驱动与设备名称设备类别的对应关系
    auto iter = m_DeviceClassMap.begin();

    for (; iter != m_DeviceClassMap.end(); ++iter) {

        foreach (auto device, iter.value()) {
            if (m_DeviceDriverPool.find(device->driver()) != m_DeviceDriverPool.end()) {
                // 驱动内容不为空时添加
                if (false == device->driver().isEmpty())
                    m_DeviceDriverPool[device->driver()][iter.key()] += device->name();
            } else {
                if (false == device->driver().isEmpty())
                    m_DeviceDriverPool[device->driver()][iter.key()] += device->name();
            }

        }
    }
    return m_DeviceDriverPool;
}

void DeviceManager::addInputInfo(const QString &key, const QMap<QString, QString> &mapInfo)
{
    // 添加输入设备信息
    if (m_InputDeviceInfo.find(key) == m_InputDeviceInfo.end())
        m_InputDeviceInfo.insert(key, mapInfo);
}
const QMap<QString, QString> &DeviceManager::inputInfo(const QString &key)
{
    return m_InputDeviceInfo[key];
}

bool DeviceManager::isDeviceExistInPairedDevice(const QString &mac)
{
    // 获取蓝牙设备配对信息
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("bt_device");

    // 判断mac是否在蓝牙设备列表中
    foreach (auto item, cmdInfo) {
        for (auto it = item.begin(); it != item.end(); ++it) {
            if (it.value() == mac)
                return true;
        }
    }
    return false;
}

void DeviceManager::setCpuNum(int num)
{
    m_CpuNum = num;
}

void DeviceManager::setCpuFrequencyIsCur(const bool &flag)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceCPU.begin();
    for (; it != m_ListDeviceCPU.end(); ++it) {
        DeviceCpu *device = dynamic_cast<DeviceCpu *>(*it);
        if (!device)
            continue;

        device->setFrequencyIsCur(flag);
    }
}
