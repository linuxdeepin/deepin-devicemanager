// 项目自身文件
#include "DeviceInput.h"

// Qt库文件
#include <QDebug>

// 其它头文件
#include "EnableManager.h"
#include "DeviceManager.h"

DeviceInput::DeviceInput()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_Interface("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Description("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeyToLshw("")
{
    initFilterKey();
    m_CanEnable = true;
}

bool DeviceInput::setInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    // 根据bus info属性值与m_KeyToLshw对比,判断是否为同一设备
    if (m_KeyToLshw != mapInfo["bus info"]) {
        QString key = mapInfo["bus info"];
        key.replace("a", "10");

        if (m_KeyToLshw != key) {
            return false;
        }
    }

    // 设置基础设备信息
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_Interface);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);

    return true;
}

void DeviceInput::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);

    // 获取键盘的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model") != mapInfo.end() && mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }
    if (mapInfo.find("Device") != mapInfo.end() && mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }

    // 获取映射到  cat /proc/bus/input/devices 里面的关键字
    QRegExp re = QRegExp(".*(event[0-9]{1,2}).*");
    if (re.exactMatch(mapInfo["Device File"])) {
        m_KeysToCatDevices = re.cap(1);
    } else {
        QRegExp rem = QRegExp(".*(mouse[0-9]{1,2}).*");
        if (rem.exactMatch(mapInfo["Device File"])) {
            m_KeysToCatDevices = rem.cap(1);
        }
    }

    // 由cat /proc/bus/devices/input设置设备信息
    setInfoFromInput();

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

void DeviceInput::setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }

    // 获取映射到  cat /proc/bus/input/devices 里面的关键字
    QRegExp re = QRegExp(".*(event[0-9]{1,2}).*");
    if (re.exactMatch(mapInfo["Device File"])) {
        m_KeysToCatDevices = re.cap(1);
    } else {
        QRegExp rem = QRegExp(".*(mouse[0-9]{1,2}).*");
        if (rem.exactMatch(mapInfo["Device File"])) {
            m_KeysToCatDevices = rem.cap(1);
        }
    }

    // 由cat /proc/bus/devices/input设置设备信息
    setInfoFromInput();

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

void DeviceInput::setInfoFromInput()
{
    // 获取对应的由cat /proc/bus/devices/input读取的设备信息
    const QMap<QString, QString> &mapInfo = DeviceManager::instance()->inputInfo(m_KeysToCatDevices);

    // 设置Name属性
    setAttribute(mapInfo, "Name", m_Name, true);

    // Uniq属性标识蓝牙设备Mac地址
    m_keysToPairedDevice = mapInfo["Uniq"].toUpper();

    // 设置设备是否可用
    m_Enable = EnableManager::instance()->isDeviceEnable(m_Name);
}

void DeviceInput::setInfoFromBluetoothctl()
{
    // 判断该设备信息是否存在于Bluetoothctl中
    if (isValueValid(m_keysToPairedDevice)) {
        bool isExist = DeviceManager::instance()->isDeviceExistInPairedDevice(m_keysToPairedDevice.toUpper());

        if (isExist) {
            m_Interface = "Bluetooth";
        }
    }
}


const QString &DeviceInput::name() const
{
    return m_Name;
}

const QString &DeviceInput::driver() const
{
    return m_Driver;
}

QString DeviceInput::subTitle()
{
    // 获取子标题
    if (!m_Name.isEmpty())
        return m_Name;
    return m_Model;
}

const QString DeviceInput::getOverviewInfo()
{
    // 获取概况信息
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

EnableDeviceStatus DeviceInput::setEnable(bool e)
{
    // 设置设备状态
    EnableDeviceStatus res = EnableManager::instance()->enableDeviceByInput(m_Name, e, m_Index);
    if (res == EDS_Success) {
        m_Enable = e;
    }

    return res;
}

bool DeviceInput::enable()
{
    return EnableManager::instance()->isDeviceEnable(m_Name);
}

void DeviceInput::initFilterKey()
{
    // 添加可显示的设备信息
    addFilterKey(QObject::tr("Uniq"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    addFilterKey(QObject::tr("physical id"));
}

void DeviceInput::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Interface"), m_Interface);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
}

void DeviceInput::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Version"), m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceInput::loadTableData()
{
    // 加载表格数据
    QString name;
    if (!enable()) {
        name = "(" + tr("Disable") + ") " + m_Name;
    } else {
        name = m_Name;
    }

    m_TableData.append(name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}

