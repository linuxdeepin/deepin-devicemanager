#include"DeviceManager.h"
#include"DeviceCpu.h"
#include"DeviceGpu.h"
#include"DeviceMouse.h"
#include"DeviceStorage.h"
#include"DeviceMemory.h"
#include"DeviceMonitor.h"
#include"DeviceBios.h"
#include"DeviceBluetooth.h"
#include"DeviceAudio.h"
#include"DeviceNetwork.h"
#include"DeviceImage.h"
#include"DeviceKeyboard.h"
#include"DeviceOthers.h"
#include"DevicePower.h"
#include"DevicePrint.h"
#include"DeviceOtherPCI.h"
#include"DeviceComputer.h"
#include"DeviceCdrom.h"
#include<QDebug>

DeviceManager    *DeviceManager::sInstance = nullptr;

DeviceManager::DeviceManager()
{

}

void DeviceManager::clear()
{
    m_cmdInfo.clear();

    m_ListDeviceMouse.clear();
    m_ListDeviceStorage.clear();
    m_ListDeviceMonitor.clear();
    m_ListBluetooth.clear();
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

void DeviceManager::addMouseDevice(const DeviceMouse &device)
{
    // 如果不是重复设备则添加到设备列表
    m_ListDeviceMouse.append(device);
}

const QList<DeviceMouse> &DeviceManager::getMouseDevices()
{
    return m_ListDeviceMouse;
}

bool DeviceManager::addMouseInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceMouse>::iterator it = m_ListDeviceMouse.begin();
    for (; it != m_ListDeviceMouse.end(); ++it) {
        if ((*it).setInfoFromlshw(mapInfo)) {
            return true;
        }
    }
    return false;
}

void DeviceManager::addCpuDevice(const DeviceCpu &device)
{
    m_ListDeviceCPU.append(device);
}

const QList<DeviceCpu> &DeviceManager::getCPUDevices()
{
    return m_ListDeviceCPU;
}

void DeviceManager::addStorageDeivce(const DeviceStorage &device)
{
    m_ListDeviceStorage.append(device);
}

void DeviceManager::addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {

        if ((*it).addInfoFromlshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        if ((*it).addInfoFromSmartctl(name, mapInfo)) {
            return;
        }
    }
}

const QList<DeviceStorage> &DeviceManager::getStorageDevices()
{
    return m_ListDeviceStorage;
}

bool DeviceManager::setStorageDeviceMediaType(const QString &name, const QString &value)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        if ((*it).setMediaType(name, value)) {
            return true;
        }
    }
    return  false;
}

bool DeviceManager::setKLUStorageDeviceMediaType(const QString &name, const QString &value)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        if ((*it).setKLUMediaType(name, value)) {
            return true;
        }
    }
    return  false;
}

void DeviceManager::addGpuDevice(const DeviceGpu &device)
{
    m_ListDeviceGPU.append(device);
}
const QList<DeviceGpu> &DeviceManager::getGPUDevices()
{
    return m_ListDeviceGPU;
}

void DeviceManager::setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceGpu>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        (*it).setLshwInfo(mapInfo);
    }
}

void DeviceManager::setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceGpu>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        (*it).setXrandrInfo(mapInfo);
    }
}

void DeviceManager::setGpuSizeFromDmesg(const QString &info)
{
    QList<DeviceGpu>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        (*it).setDmesgInfo(info);
    }
}

void DeviceManager::addMemoryDevice(const DeviceMemory &device)
{
    m_ListDeviceMemory.append(device);
}
const QList<DeviceMemory> &DeviceManager::getMemoryDevices()
{
    return m_ListDeviceMemory;
}

void DeviceManager::setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceMemory>::iterator it = m_ListDeviceMemory.begin();
    for (; it != m_ListDeviceMemory.end(); ++it) {
        if ((*it).setInfoFromDmidecode(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addMonitor(const DeviceMonitor &device)
{
    m_ListDeviceMonitor.append(device);
}
const QList<DeviceMonitor> &DeviceManager::getMonitorDevices()
{
    return m_ListDeviceMonitor;
}

void DeviceManager::setMonitorInfoFromXrandr(const QString &main, const QString &edid)
{
    QList<DeviceMonitor>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        if ((*it).setInfoFromXradr(main, edid)) {
            return;
        }
    }
}

void DeviceManager::setCurrentResolution(const QString &resolution, const QString &rate)
{
    QList<DeviceMonitor>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        if ((*it).setCurrentResolution(resolution, rate)) {
            return;
        }
    }
}

void DeviceManager::addBiosDevice(const DeviceBios &device)
{
    m_ListDeviceBios.append(device);
}
const QList<DeviceBios> &DeviceManager::getBiosDevices()
{
    return m_ListDeviceBios;
}

void DeviceManager::setLanguageInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBios>::iterator it = m_ListDeviceBios.begin();
    for (; it != m_ListDeviceBios.end(); ++it) {
        if ((*it).setBiosLanguageInfo(mapInfo)) {
            return;
        }
    }
}


void DeviceManager::addBluetoothDevice(const DeviceBluetooth &device)
{
    m_ListBluetooth.append(device);
}
const QList<DeviceBluetooth> &DeviceManager::getBluetoothDevices()
{
    return m_ListBluetooth;
}

void DeviceManager::setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBluetooth>::iterator it = m_ListBluetooth.begin();
    for (; it != m_ListBluetooth.end(); ++it) {
        if ((*it).setInfoFromLshw(mapInfo)) {
            return;
        }
    }
}

bool DeviceManager::setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBluetooth>::iterator it = m_ListBluetooth.begin();
    for (; it != m_ListBluetooth.end(); ++it) {
        if ((*it).setInfoFromHwinfo(mapInfo)) {
            return true;
        }
    }
    return false;
}

void DeviceManager::addAudioDevice(const DeviceAudio &device)
{
    m_ListDeviceAudio.append(device);
}
const QList<DeviceAudio> &DeviceManager::getAudioDevices()
{
    return m_ListDeviceAudio;
}

void DeviceManager::setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceAudio>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        if ((*it).setInfoFromLshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::setAudioChipFromDmesg(const QString &info)
{
    QList<DeviceAudio>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        (*it).setAudioChipFromDmesg(info);
    }
}

void DeviceManager::addNetworkDevice(const DeviceNetwork &device)
{
    m_ListDeviceNetwork.append(device);
}
const QList<DeviceNetwork> &DeviceManager::getNetworkDevices()
{
    return m_ListDeviceNetwork;
}

void DeviceManager::setNetworkInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceNetwork>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {
        if ((*it).setInfoFromHwinfo(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addImageDevice(const DeviceImage &device)
{
    m_ListDeviceImage.append(device);
}

void DeviceManager::setCameraInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceImage>::iterator it = m_ListDeviceImage.begin();
    for (; it != m_ListDeviceImage.end(); ++it) {
        (*it).setInfoFromLshw(mapInfo);
    }
}

const QList<DeviceImage> &DeviceManager::getImageDevices()
{
    return m_ListDeviceImage;
}
void DeviceManager::addKeyboardDevice(const DeviceKeyboard &device)
{
    m_ListDeviceKeyboard.append(device);
}
const QList<DeviceKeyboard> &DeviceManager::getKeyboardDevices()
{
    return m_ListDeviceKeyboard;
}
void DeviceManager::setKeyboardInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceKeyboard>::iterator it = m_ListDeviceKeyboard.begin();
    for (; it != m_ListDeviceKeyboard.end(); ++it) {
        if ((*it).setInfoFromlshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addOthersDevice(const DeviceOthers &device)
{
    bool isOtherDevice = true;
    foreach (const DeviceStorage &disk, m_ListDeviceStorage) {
        if (disk.keyFromStorage() == device.logicalName() && device.logicalName() != "") {
            isOtherDevice = false;
            break;
        }
    }
    if (isOtherDevice) {
        m_ListDeviceOthers.append(device);
    }
}

void DeviceManager::addOthersDeviceFromHwinfo(const DeviceOthers &device)
{
    foreach (const DeviceOthers &cur, m_ListDeviceOthers) {
        if (cur.busInfo() == device.busInfo() && cur.busInfo() != "") {
            return;
        }
    }
    m_ListDeviceOthers.append(device);
}

const QList<DeviceOthers> &DeviceManager::getOthersDevices()
{
    return m_ListDeviceOthers;
}

void DeviceManager::setOthersDeviceInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceOthers>::iterator it = m_ListDeviceOthers.begin();
    for (; it != m_ListDeviceOthers.end(); ++it) {
        (*it).setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::addPowerDevice(const DevicePower &device)
{
    m_ListDevicePower.append(device);
}
const QList<DevicePower> &DeviceManager::getPowerDevices()
{
    return m_ListDevicePower;
}
void DeviceManager::setPowerDaemonInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DevicePower>::iterator it = m_ListDevicePower.begin();
    for (; it != m_ListDevicePower.end(); ++it) {
        (*it).setDaemonInfo(mapInfo);
    }
}
void DeviceManager::addPrintDevice(const DevicePrint &device)
{
    m_ListDevicePrint.append(device);
}
const QList<DevicePrint> &DeviceManager::getPrintDevices()
{
    return m_ListDevicePrint;
}

void DeviceManager::addOtherPCIDevice(const DeviceOtherPCI &device)
{
    m_ListDeviceOtherPCI.append(device);
}
const QList<DeviceOtherPCI> &DeviceManager::getOtherPCIDevices()
{
    return m_ListDeviceOtherPCI;
}

void DeviceManager::addComputerDevice(const DeviceComputer &device)
{
    m_ListDeviceComputer.append(device);
}
const QList<DeviceComputer> &DeviceManager::getComputerDevices()
{
    return m_ListDeviceComputer;
}

void DeviceManager::addCdromDevice(const DeviceCdrom &device)
{
    m_ListDeviceCdrom.append(device);
}
const QList<DeviceCdrom> DeviceManager::getCdromDevices()
{
    return m_ListDeviceCdrom;
}

void DeviceManager::addLshwinfoIntoCdromDevice(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceCdrom>::iterator it = m_ListDeviceCdrom.begin();
    for (; it != m_ListDeviceCdrom.end(); ++it) {
        (*it).setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::addBusId(const QStringList &busId)
{
    m_BusIdList.append(busId);
}
const QStringList &DeviceManager::getBusId()
{
    return m_BusIdList;
}

void DeviceManager::addCmdInfo(const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo)
{
    foreach (const QString &key, cmdInfo.keys()) {
        m_cmdInfo[key].append(cmdInfo[key]);
    }
}
const QList<QMap<QString, QString>> &DeviceManager::cmdInfo(const QString &key)
{
    return m_cmdInfo[key];
}
