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
    m_ListDeviceMouse.clear();
    m_ListDeviceCPU.clear();
    m_ListDeviceStorage.clear();
    m_ListDeviceGPU.clear();
    m_ListDeviceMemory.clear();
    m_ListDeviceMonitor.clear();
    m_ListDeviceBios.clear();
    m_ListBluetooth.clear();
    m_ListDeviceAudio.clear();
    m_ListDeviceNetwork.clear();
    m_ListDeviceImage.clear();
    m_ListDeviceKeyboard.clear();
    m_ListDeviceOthers.clear();
    m_ListDevicePower.clear();
    m_ListDevicePrint.clear();
    m_ListDeviceOtherPCI.clear();
    m_ListDeviceComputer.clear();
    m_ListDeviceCdrom.clear();
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

void DeviceManager::setCatDevicesIntoMouseDevice(const QString &info)
{
    QList<DeviceMouse>::iterator it = m_ListDeviceMouse.begin();
    for (; it != m_ListDeviceMouse.end(); ++it) {

        if ((*it).setInfoFromCatInputDevices(info)) {
            return;
        }
    }
}

bool DeviceManager::setLshwinfoIntoMouseDevice(const QString &info)
{
    QList<DeviceMouse>::iterator it = m_ListDeviceMouse.begin();
    for (; it != m_ListDeviceMouse.end(); ++it) {

        if ((*it).setInfoFromlshw(info)) {
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

void DeviceManager::addLshwinfoIntoStorageDevice(const QString &info)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {

        if ((*it).addInfoFromlshw(info)) {
            return;
        }
    }
}

void DeviceManager::setStorageInfoFromSmartctl(const QString &name, const QString &info)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        if ((*it).addInfoFromSmartctl(name, info)) {
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

void DeviceManager::setStorageDeviceAttribute(const QString &name)
{
    QList<DeviceStorage>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
//        (*it).setLshwInfo();
    }
}

void DeviceManager::addGpuDevice(const DeviceGpu &device)
{
    m_ListDeviceGPU.append(device);
}
const QList<DeviceGpu> &DeviceManager::getGPUDevices()
{
    return m_ListDeviceGPU;
}

void DeviceManager::setGpuInfoFromLshw(const QString &info)
{
    QList<DeviceGpu>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        (*it).setLshwInfo(info);
    }
}

void DeviceManager::setGpuInfoFromXrandr(const QString &info)
{
    QList<DeviceGpu>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        (*it).setXrandrInfo(info);
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
void DeviceManager::setMemoryInfoFromDmidecode(const QString &info)
{
    QList<DeviceMemory>::iterator it = m_ListDeviceMemory.begin();
    for (; it != m_ListDeviceMemory.end(); ++it) {
        if ((*it).setInfoFromDmidecode(info)) {
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

void DeviceManager::setMonitorRefreshRate(const QString &main, const QString &rate)
{
    QList<DeviceMonitor>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        if ((*it).setRateInfoFromXradr(main, rate)) {
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


void DeviceManager::addBluetoothDevice(const DeviceBluetooth &device)
{
    m_ListBluetooth.append(device);
}
const QList<DeviceBluetooth> &DeviceManager::getBluetoothDevices()
{
    return m_ListBluetooth;
}

void DeviceManager::setBluetoothInfoFromLshw(const QString &info)
{
    QList<DeviceBluetooth>::iterator it = m_ListBluetooth.begin();
    for (; it != m_ListBluetooth.end(); ++it) {
        if ((*it).setInfoFromLshw(info)) {
            return;
        }
    }
}

void DeviceManager::setBluetoothInfoFromHwinfo(const QString &info)
{
    QList<DeviceBluetooth>::iterator it = m_ListBluetooth.begin();
    for (; it != m_ListBluetooth.end(); ++it) {
        if ((*it).setInfoFromHwinfo(info)) {
            return;
        }
    }
}

void DeviceManager::addAudioDevice(const DeviceAudio &device)
{
    m_ListDeviceAudio.append(device);
}
const QList<DeviceAudio> &DeviceManager::getAudioDevices()
{
    return m_ListDeviceAudio;
}
void DeviceManager::setAudioInfoFromLshw(const QString &info)
{
    QList<DeviceAudio>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        if ((*it).setInfoFromLshw(info)) {
            return;
        }
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

void DeviceManager::setNetworkInfoFromHwinfo(const QString &info)
{
    QList<DeviceNetwork>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {
        if ((*it).setInfoFromHwinfo(info)) {
            return;
        }
    }
}

void DeviceManager::addImageDevice(const DeviceImage &device)
{
    m_ListDeviceImage.append(device);
}

void DeviceManager::setCameraInfoFromLshw(const QString &info)
{
    QList<DeviceImage>::iterator it = m_ListDeviceImage.begin();
    for (; it != m_ListDeviceImage.end(); ++it) {
        (*it).setInfoFromLshw(info);
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

void DeviceManager::setKeyboardInfoFromCatDevices(const QString &info)
{
    QList<DeviceKeyboard>::iterator it = m_ListDeviceKeyboard.begin();
    for (; it != m_ListDeviceKeyboard.end(); ++it) {
        if ((*it).setInfoFromCatInputDevices(info)) {
            return;
        }
    }
}

void DeviceManager::setKeyboardInfoFromLshw(const QString &info)
{
    QList<DeviceKeyboard>::iterator it = m_ListDeviceKeyboard.begin();
    for (; it != m_ListDeviceKeyboard.end(); ++it) {
        if ((*it).setInfoFromlshw(info)) {
            return;
        }
    }
}

void DeviceManager::addOthersDevice(const DeviceOthers &device)
{
    bool isOtherDevice = true;
    foreach (const DeviceStorage &disk, m_ListDeviceStorage) {
        qDebug() << disk.keyFromStorage() << "**********************************" << device.logicalName();
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
void DeviceManager::addPowerDevice(const DevicePower &device)
{
    m_ListDevicePower.append(device);
}
const QList<DevicePower> &DeviceManager::getPowerDevices()
{
    return m_ListDevicePower;
}
void DeviceManager::setPowerDaemonInfo(const QString &info)
{
    QList<DevicePower>::iterator it = m_ListDevicePower.begin();
    for (; it != m_ListDevicePower.end(); ++it) {
        (*it).setDaemonInfo(info);
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

void DeviceManager::setPrinterInfoFromHwinfo(const QString &info)
{
    QList<DevicePrint>::iterator it = m_ListDevicePrint.begin();
    for (; it != m_ListDevicePrint.end(); ++it) {
        (*it).setInfoFromHwinfo(info);
    }
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
void DeviceManager::setComputerBoardInfoFromCatBoardinfo(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setInfoFromCatBoardInfo(info);
    }
}
void DeviceManager::setComputerUOSInfoFromDSysInfo(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setUOSInfo(info);
    }
}


void DeviceManager::setComputerInfoFromCatVersion(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setInfoFromCatDevice(info);
    }
}

void DeviceManager::setComputerInfoFromDmidecode(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setInfoFromDmidecode(info);
    }
}


void DeviceManager::setComputerChassisInfoFromDmidecode(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setChassisInfoFromDmidecode(info);
    }
}

void DeviceManager::setComputerInfoFromLshw(const QString &info)
{
    QList<DeviceComputer>::iterator it = m_ListDeviceComputer.begin();
    for (; it != m_ListDeviceComputer.end(); ++it) {
        (*it).setInfoFromLshw(info);
    }
}

void DeviceManager::addCdromDevice(const DeviceCdrom &device)
{
    m_ListDeviceCdrom.append(device);
}
const QList<DeviceCdrom> DeviceManager::getCdromDevices()
{
    return m_ListDeviceCdrom;
}
void DeviceManager::setCdromInfoFromLshw(const QString &info)
{
    QList<DeviceCdrom>::iterator it = m_ListDeviceCdrom.begin();
    for (; it != m_ListDeviceCdrom.end(); ++it) {
        if ((*it).setInfoFromLshw(info)) {
            break;
        }
    }
}


void DeviceManager::addField(const QString &key, const QString &address)
{
    m_TsField[address].insert(key);
}
void DeviceManager::generateTs()
{
    QFile file(QString("./ManulTrack.txt"));
    QTextStream *out = nullptr;
    if (file.exists())
        file.remove();
    if (!file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        return;
    }
    out = new QTextStream(&file);
    foreach (const QString &key, m_TsField.keys()) {
        *out << "<context>\n"
             << "<name>" << key << "</name>\n";
        const QSet<QString> &values = m_TsField[key];
        foreach (const QString &value, values) {
            if (out != nullptr) {
                *out << "   <message>\n"
                     "       <source>"  << value.trimmed().toStdString().data() << "</source>\n"
                     "       <translation type =\"unfinished\"></translation>\n"
                     "   </message>\n";
            }
        }
        *out << "</context>\n";
    }
    file.close();
    delete out;
}
