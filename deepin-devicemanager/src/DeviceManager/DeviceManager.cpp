#include "DeviceManager.h"
#include "DeviceCpu.h"
#include "DeviceGpu.h"
#include "DeviceMouse.h"
#include "DeviceStorage.h"
#include "DeviceMemory.h"
#include "DeviceMonitor.h"
#include "DeviceBios.h"
#include "DeviceBluetooth.h"
#include "DeviceAudio.h"
#include "DeviceNetwork.h"
#include "DeviceImage.h"
#include "DeviceKeyboard.h"
#include "DeviceOthers.h"
#include "DevicePower.h"
#include "DevicePrint.h"
#include "DeviceOtherPCI.h"
#include "DeviceComputer.h"
#include "DeviceCdrom.h"
#include "MacroDefinition.h"

#include <QDebug>
#include <QFile>

DeviceManager    *DeviceManager::sInstance = nullptr;
int DeviceManager::m_CurrentXlsRow = 1;

DeviceManager::DeviceManager()
{

}

DeviceManager::~DeviceManager()
{
    clear();
}

void DeviceManager::clear()
{
    m_cmdInfo.clear();

    foreach (auto device, m_ListDeviceMouse) {
        delete device;
    }

    foreach (auto device, m_ListDeviceStorage) {
        delete device;
    }

    foreach (auto device, m_ListDeviceMonitor) {
        delete device;
    }

    foreach (auto device, m_ListDeviceAudio) {
        delete device;
    }

    foreach (auto device, m_ListDeviceImage) {
        delete device;
    }

    foreach (auto device, m_ListDeviceKeyboard) {
        delete device;
    }

    foreach (auto device, m_ListDeviceOthers) {
        delete device;
    }

    foreach (auto device, m_ListDevicePower) {
        delete device;
    }

    foreach (auto device, m_ListDevicePrint) {
        delete device;
    }

    foreach (auto device, m_ListDeviceOtherPCI) {
        delete device;
    }

    foreach (auto device, m_ListDeviceCdrom) {
        delete device;
    }

    foreach (auto device, m_ListDeviceComputer) {
        delete device;
    }

    foreach (auto device, m_ListDeviceNetwork) {
        delete device;
    }

    foreach (auto device, m_ListDeviceBios) {
        delete device;
    }

    foreach (auto device, m_ListDeviceGPU) {
        delete device;
    }

    foreach (auto device, m_ListDeviceMemory) {
        delete device;
    }

    foreach (auto device, m_ListDeviceCPU) {
        delete device;
    }

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
    m_ListDeviceType.clear();

    if (true) m_ListDeviceType.append(QPair<QString, QString>(tr("Overview"), "overview##Overview"));

    if (m_ListDeviceCPU.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("CPU"), "cpu##CPU"));

    // 板载接口设备
    if (m_ListDeviceBios.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Motherboard"), "motherboard##Bios"));
    if (m_ListDeviceMemory.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Memory"), "memory##Memory"));
    if (m_ListDeviceGPU.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Display Adapter"), "displayadapter##GPU"));
    if (m_ListDeviceAudio.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Sound Adapter"), "audiodevice##Audio"));
    if (m_ListDeviceStorage.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Storage"), "storage##Storage"));
    if (m_ListDeviceOtherPCI.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Other PCI Devices"), "otherpcidevices##OtherPCI"));
    if (m_ListDevicePower.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Battery"), "battery##Power"));

    // 网络设备
    if (m_ListDeviceBluetooth.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Bluetooth"), "bluetooth##Bluetooth"));
    if (m_ListDeviceNetwork.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Network Adapter"), "networkadapter##Network"));

    // 输入设备
    if (m_ListDeviceMouse.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Mouse"), "mouse##Mouse"));
    if (m_ListDeviceKeyboard.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Keyboard"), "keyboard##Keyboard"));

    // 外设设备
    if (m_ListDeviceMonitor.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Monitor"), "monitor##Monitor"));
    if (m_ListDeviceCdrom.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("CD-ROM"), "cdrom##Cdrom"));
    if (m_ListDevicePrint.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Printer"), "printer##Print"));
    if (m_ListDeviceImage.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Camera"), "camera##Image"));
    if (m_ListDeviceOthers.size() > 0) m_ListDeviceType.append(QPair<QString, QString>(tr("Other Devices", "Other Input Devices"), "otherdevices##Others"));

    return m_ListDeviceType;
}

void DeviceManager::setDeviceListClass()
{
    m_DeviceClassMap["CPU"] = m_ListDeviceCPU;
    m_DeviceClassMap["Bios"] =  m_ListDeviceBios;
    m_DeviceClassMap["Memory"]  =  m_ListDeviceMemory;
    m_DeviceClassMap["GPU"] =  m_ListDeviceGPU;
    m_DeviceClassMap["Audio"] =  m_ListDeviceAudio;
    m_DeviceClassMap["Storage"]  =  m_ListDeviceStorage;
    m_DeviceClassMap["OtherPCI"] =  m_ListDeviceOtherPCI;
    m_DeviceClassMap["Power"] =  m_ListDevicePower;
    m_DeviceClassMap["Bluetooth"] =  m_ListDeviceBluetooth;
    m_DeviceClassMap["Network"]  =  m_ListDeviceNetwork;
    m_DeviceClassMap["Mouse"] =  m_ListDeviceMouse;
    m_DeviceClassMap["Keyboard"]  =  m_ListDeviceKeyboard;
    m_DeviceClassMap["Monitor"] =  m_ListDeviceMonitor;
    m_DeviceClassMap["Cdrom"] =  m_ListDeviceCdrom;
    m_DeviceClassMap["Print"] =  m_ListDevicePrint;
    m_DeviceClassMap["Image"] =  m_ListDeviceImage;
    m_DeviceClassMap["Others"] =  m_ListDeviceOthers;
}

bool DeviceManager::getDeviceList(const QString &name, QList<DeviceBaseInfo *> &lst)
{
    if (name == "Overview") {return false;}

    if (m_DeviceClassMap.find(name) != m_DeviceClassMap.end()) {
        lst = m_DeviceClassMap[name];
    }

    return true;
}

void DeviceManager::addMouseDevice(DeviceMouse    *const device)
{
    // 如果不是重复设备则添加到设备列表
    m_ListDeviceMouse.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getMouseDevices()
{
    return m_ListDeviceMouse;
}

bool DeviceManager::addMouseInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMouse.begin();
    for (; it != m_ListDeviceMouse.end(); ++it) {
        DeviceMouse *device = dynamic_cast<DeviceMouse *>(*it);
        if (!device) continue;
        if (device->setInfoFromlshw(mapInfo)) {
            return true;
        }
    }
    return false;
}

void DeviceManager::addCpuDevice(DeviceCpu *const device)
{
    m_ListDeviceCPU.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getCPUDevices()
{
    return m_ListDeviceCPU;
}

void DeviceManager::addStorageDeivce(DeviceStorage *const device)
{
    m_ListDeviceStorage.append(device);
}

void DeviceManager::addLshwinfoIntoStorageDevice(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device) continue;
        if (device->addInfoFromlshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::setStorageInfoFromSmartctl(const QString &name, const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device) continue;
        if (device->addInfoFromSmartctl(name, mapInfo)) {
            return;
        }
    }
}

const QList<DeviceBaseInfo *> &DeviceManager::getStorageDevices()
{
    return m_ListDeviceStorage;
}

bool DeviceManager::setStorageDeviceMediaType(const QString &name, const QString &value)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device) continue;
        if (device->setMediaType(name, value)) {
            return true;
        }
    }
    return  false;
}

bool DeviceManager::setKLUStorageDeviceMediaType(const QString &name, const QString &value)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceStorage.begin();
    for (; it != m_ListDeviceStorage.end(); ++it) {
        DeviceStorage *device = dynamic_cast<DeviceStorage *>(*it);
        if (!device) continue;
        if (device->setKLUMediaType(name, value)) {
            return true;
        }
    }
    return  false;
}

void DeviceManager::addGpuDevice(DeviceGpu *const device)
{
    m_ListDeviceGPU.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getGPUDevices()
{
    return m_ListDeviceGPU;
}

void DeviceManager::setGpuInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device) continue;
        device->setLshwInfo(mapInfo);
    }
}

void DeviceManager::setGpuInfoFromXrandr(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device) continue;
        device->setXrandrInfo(mapInfo);
    }
}

void DeviceManager::setGpuSizeFromDmesg(const QString &info)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceGPU.begin();
    for (; it != m_ListDeviceGPU.end(); ++it) {
        DeviceGpu *device = dynamic_cast<DeviceGpu *>(*it);
        if (!device) continue;
        device->setDmesgInfo(info);
    }
}

void DeviceManager::addMemoryDevice(DeviceMemory *const device)
{
    m_ListDeviceMemory.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getMemoryDevices()
{
    return m_ListDeviceMemory;
}

void DeviceManager::setMemoryInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMemory.begin();
    for (; it != m_ListDeviceMemory.end(); ++it) {
        DeviceMemory *device = dynamic_cast<DeviceMemory *>(*it);
        if (!device) continue;
        if (device->setInfoFromDmidecode(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addMonitor(DeviceMonitor *const device)
{
    m_ListDeviceMonitor.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getMonitorDevices()
{
    return m_ListDeviceMonitor;
}

void DeviceManager::setMonitorInfoFromXrandr(const QString &main, const QString &edid)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        DeviceMonitor *device = dynamic_cast<DeviceMonitor *>(*it);
        if (!device) continue;
        if (device->setInfoFromXradr(main, edid)) {
            return;
        }
    }
}

void DeviceManager::setCurrentResolution(const QString &resolution, const QString &rate)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceMonitor.begin();
    for (; it != m_ListDeviceMonitor.end(); ++it) {
        DeviceMonitor *device = dynamic_cast<DeviceMonitor *>(*it);
        if (!device) continue;
        if (device->setCurrentResolution(resolution, rate)) {
            return;
        }
    }
}

void DeviceManager::addBiosDevice(DeviceBios *const device)
{
    m_ListDeviceBios.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getBiosDevices()
{
    return m_ListDeviceBios;
}

void DeviceManager::setLanguageInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBios.begin();
    for (; it != m_ListDeviceBios.end(); ++it) {
        DeviceBios *device = dynamic_cast<DeviceBios *>(*it);
        if (!device) continue;
        if (device->setBiosLanguageInfo(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addBluetoothDevice(DeviceBluetooth *const device)
{
    m_ListDeviceBluetooth.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getBluetoothDevices()
{
    return m_ListDeviceBluetooth;
}

void DeviceManager::setBluetoothInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin();
    for (; it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(*it);
        if (!device) continue;
        if (device->setInfoFromLshw(mapInfo)) {
            return;
        }
    }
}

bool DeviceManager::setBluetoothInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceBluetooth.begin();
    for (; it != m_ListDeviceBluetooth.end(); ++it) {
        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(*it);
        if (!device) continue;
        if (device->setInfoFromHwinfo(mapInfo)) {
            return true;
        }
    }
    return false;
}

void DeviceManager::addAudioDevice(DeviceAudio *const device)
{
    m_ListDeviceAudio.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getAudioDevices()
{
    return m_ListDeviceAudio;
}

void DeviceManager::setAudioInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(*it);
        if (!device) continue;
        if (device->setInfoFromLshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::setAudioChipFromDmesg(const QString &info)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceAudio.begin();
    for (; it != m_ListDeviceAudio.end(); ++it) {
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(*it);
        if (!device) continue;
        device->setAudioChipFromDmesg(info);
    }
}

void DeviceManager::addNetworkDevice(DeviceNetwork *const device)
{
    m_ListDeviceNetwork.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getNetworkDevices()
{
    return m_ListDeviceNetwork;
}

void DeviceManager::setNetworkInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceNetwork.begin();
    for (; it != m_ListDeviceNetwork.end(); ++it) {
        DeviceNetwork *device = dynamic_cast<DeviceNetwork *>(*it);
        if (!device) continue;
        if (device->setInfoFromHwinfo(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addImageDevice(DeviceImage *const device)
{
    m_ListDeviceImage.append(device);
}

void DeviceManager::setCameraInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceImage.begin();
    for (; it != m_ListDeviceImage.end(); ++it) {
        DeviceNetwork *device = dynamic_cast<DeviceNetwork *>(*it);
        if (!device) continue;
        device->setInfoFromLshw(mapInfo);
    }
}

const QList<DeviceBaseInfo *> &DeviceManager::getImageDevices()
{
    return m_ListDeviceImage;
}

void DeviceManager::addKeyboardDevice(DeviceKeyboard *const device)
{
    m_ListDeviceKeyboard.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getKeyboardDevices()
{
    return m_ListDeviceKeyboard;
}

void DeviceManager::setKeyboardInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceKeyboard.begin();
    for (; it != m_ListDeviceKeyboard.end(); ++it) {
        DeviceKeyboard *device = dynamic_cast<DeviceKeyboard *>(*it);
        if (!device) continue;
        if (device->setInfoFromlshw(mapInfo)) {
            return;
        }
    }
}

void DeviceManager::addOthersDevice(DeviceOthers *const device)
{
    bool isOtherDevice = true;
    foreach (auto disk, m_ListDeviceStorage) {
        DeviceStorage *deviceDisk = dynamic_cast<DeviceStorage *>(disk);
        if (!deviceDisk) continue;
        if (deviceDisk->keyFromStorage() == device->logicalName() && device->logicalName() != "") {
            isOtherDevice = false;
            break;
        }
    }
    if (isOtherDevice) {
        m_ListDeviceOthers.append(device);
    }
}

void DeviceManager::addOthersDeviceFromHwinfo(DeviceOthers *const device)
{
    foreach (auto cur, m_ListDeviceOthers) {
        DeviceOthers *deviceOthers = dynamic_cast<DeviceOthers *>(cur);
        if (!deviceOthers) continue;
        if (deviceOthers->busInfo() == device->busInfo() && deviceOthers->busInfo() != "") {
            return;
        }
    }
    m_ListDeviceOthers.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getOthersDevices()
{
    return m_ListDeviceOthers;
}

void DeviceManager::setOthersDeviceInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceOthers.begin();
    for (; it != m_ListDeviceOthers.end(); ++it) {
        DeviceOthers *device = dynamic_cast<DeviceOthers *>(*it);
        if (!device) continue;
        device->setInfoFromLshw(mapInfo);
    }
}

void DeviceManager::addPowerDevice(DevicePower *const device)
{
    m_ListDevicePower.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getPowerDevices()
{
    return m_ListDevicePower;
}

void DeviceManager::setPowerDaemonInfo(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDevicePower.begin();
    for (; it != m_ListDevicePower.end(); ++it) {
        DevicePower *device = dynamic_cast<DevicePower *>(*it);
        if (!device) continue;
        device->setDaemonInfo(mapInfo);
    }
}

void DeviceManager::addPrintDevice(DevicePrint *const device)
{
    m_ListDevicePrint.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getPrintDevices()
{
    return m_ListDevicePrint;
}

void DeviceManager::addOtherPCIDevice(DeviceOtherPCI *const device)
{
    m_ListDeviceOtherPCI.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getOtherPCIDevices()
{
    return m_ListDeviceOtherPCI;
}

void DeviceManager::addComputerDevice(DeviceComputer *const device)
{
    m_ListDeviceComputer.append(device);
}

const QList<DeviceBaseInfo *> &DeviceManager::getComputerDevices()
{
    return m_ListDeviceComputer;
}

void DeviceManager::addCdromDevice(DeviceCdrom *const device)
{
    m_ListDeviceCdrom.append(device);
}

const QList<DeviceBaseInfo *> DeviceManager::getCdromDevices()
{
    return m_ListDeviceCdrom;
}

void DeviceManager::addLshwinfoIntoCdromDevice(const QMap<QString, QString> &mapInfo)
{
    QList<DeviceBaseInfo *>::iterator it = m_ListDeviceCdrom.begin();
    for (; it != m_ListDeviceCdrom.end(); ++it) {
        DeviceCdrom *device = dynamic_cast<DeviceCdrom *>(*it);
        if (!device) continue;
        device->setInfoFromLshw(mapInfo);
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

bool DeviceManager::exportToTxt(const QString &filePath)
{
    QFile txtFile(filePath);
    if (false == txtFile.open(QIODevice::WriteOnly)) {
        return false;
    }

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
    QFile html(filePath);
    if (false == html.open(QIODevice::WriteOnly)) {
        return false;
    }
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
    out << "[" << tr("Overview") << "]\n-------------------------------------------------";
    out << "\n";

    out.setFieldWidth(21);
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
    out << tr("Device") + ": ";
    out.setFieldWidth(0);
    out << m_OveriewMap["Overview"];
    out << "\n";

    out.setFieldWidth(21);
    out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
    out << tr("OS") + ": ";
    out.setFieldWidth(0);
    out << m_OveriewMap["OS"];
    out << "\n";

    foreach (auto iter, m_ListDeviceType) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            continue;
        }

        if (strList[1] == "Overview") {
            continue;
        }

        if (m_OveriewMap.find(strList[1]) != m_OveriewMap.end()) {
            out.setFieldWidth(21);
            out.setFieldAlignment(QTextStream::FieldAlignment::AlignLeft);
            out << tr(strList[1].toStdString().c_str()) + ": ";
            out.setFieldWidth(0);
            out << m_OveriewMap[strList[1]];
            out << "\n";
        }
    }
    out << "\n";
}



void DeviceManager::overviewToHtml(QFile &html)
{
    html.write((QString("<h2>") + "[" + tr("Overview") + "]" + "</h2>").toUtf8());
    QDomDocument doc;
    infoToHtml(doc, "Device", m_OveriewMap["Overview"]);
    infoToHtml(doc, "OS", m_OveriewMap["OS"]);


    foreach (auto iter, m_ListDeviceType) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            continue;
        }

        if (strList[1] == "Overview") {
            continue;
        }

        if (m_OveriewMap.find(strList[1]) != m_OveriewMap.end()) {
            infoToHtml(doc, strList[1], m_OveriewMap[strList[1]]);
        }
    }

    doc.appendChild(doc.createElement("br"));
    html.write(doc.toString().toStdString().data());
}

void DeviceManager::overviewToDoc(Docx::Document &doc)
{
    doc.addHeading("[" + tr("Overview") + "]");
    doc.addParagraph("-------------------------------------------------");

    QString line = tr("Device") + ":  " + m_OveriewMap["Overview"];
    doc.addParagraph(line);

    line = tr("OS") + ":  " + m_OveriewMap["OS"];
    doc.addParagraph(line);

    foreach (auto iter, m_ListDeviceType) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            continue;
        }

        if (strList[1] == "Overview") {
            continue;
        }

        if (m_OveriewMap.find(strList[1]) != m_OveriewMap.end()) {
            line = tr(strList[1].toStdString().c_str()) + ":  " + m_OveriewMap[strList[1]];
            doc.addParagraph(line);
        }
    }
    doc.addParagraph("\n");
}

void DeviceManager::overviewToXlsx(QXlsx::Document &xlsx, QXlsx::Format &boldFont)
{
    boldFont.setFontBold(true);
    xlsx.write(m_CurrentXlsRow++, 1, "[" + tr("Overview") + "]", boldFont);

    boldFont.setFontBold(false);
    boldFont.setFontSize(10);

    xlsx.write(m_CurrentXlsRow, 1, tr("Device"), boldFont);
    xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap["Overview"], boldFont);
    xlsx.write(m_CurrentXlsRow, 1, tr("OS"), boldFont);
    xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap["OS"], boldFont);

    foreach (auto iter, m_ListDeviceType) {
        QStringList strList = iter.second.split("##");

        if (strList.size() != 2) {
            continue;
        }

        if (strList[1] == "Overview") {
            continue;
        }

        if (m_OveriewMap.find(strList[1]) != m_OveriewMap.end()) {

            xlsx.write(m_CurrentXlsRow, 1, tr(strList[1].toStdString().c_str()), boldFont);
            xlsx.write(m_CurrentXlsRow++, 2, m_OveriewMap[strList[1]], boldFont);
        }
    }
    m_CurrentXlsRow++;
}

void DeviceManager::infoToHtml(QDomDocument &doc, const QString &key, const QString &value)
{
    QDomElement table = doc.createElement("table");
    table.setAttribute("border", "0");
    table.setAttribute("width", "100%");
    table.setAttribute("cellpadding", "3");

    QDomElement tr = doc.createElement("tr");

    QDomElement td = doc.createElement("td");
    td.setAttribute("width", "15%");
    td.setAttribute("style", "text-align:left;");

    QDomText nameText = doc.createTextNode(QObject::tr(key.toStdString().c_str()) + ": ");
    td.appendChild(nameText);
    tr.appendChild(td);

    QDomElement td2 = doc.createElement("td");
    td2.setAttribute("width", "85%");

    QDomText valueText;
    valueText = doc.createTextNode(value);
    td2.appendChild(valueText);

    tr.appendChild(td2);

    table.appendChild(tr);

    doc.appendChild(table);
}

const QMap<QString, QString>  &DeviceManager::getDeviceOverview()
{
    m_OveriewMap.clear();

    auto iter = m_DeviceClassMap.begin();
    for (; iter != m_DeviceClassMap.end(); ++iter) {
        if (m_OveriewMap.find(iter.key()) == m_OveriewMap.end()) {
            foreach (auto device, iter.value()) {
                QString ov = device->getOverviewInfo();
                if (ov.isEmpty() == false) {
                    if (m_OveriewMap.find(iter.key()) == m_OveriewMap.end()) {
                        m_OveriewMap[iter.key()] = ov;
                    } else {
                        m_OveriewMap[iter.key()] += "/";
                        m_OveriewMap[iter.key()] += ov;
                    }
                }
            }
        }
    }

    // 设备名称
    m_OveriewMap["Overview"] = m_ListDeviceComputer[0]->getOverviewInfo();

    // 操作系统
    m_OveriewMap["OS"] = dynamic_cast<DeviceComputer *>(m_ListDeviceComputer[0])->getOSInfo();

    // CPU 概况显示 样式"Intel(R) Core(TM) i3-9100F CPU @ 3.60GHz (四核 / 四逻辑处理器)"
    m_OveriewMap["CPU"] = m_ListDeviceCPU[0] ->getOverviewInfo();

    return m_OveriewMap;
}

