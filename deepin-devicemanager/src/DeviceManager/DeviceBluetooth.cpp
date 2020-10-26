// 项目自身文件
#include "DeviceBluetooth.h"

// Qt库文件
#include <QDebug>

// 其它头文件
#include "EnableManager.h"

DeviceBluetooth::DeviceBluetooth()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Version("")
    , m_Model("")
    , m_MAC("")
    , m_LogicalName("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Driver("btusb")
    , m_DriverVersion("")
    , m_MaximumPower("")
    , m_Speed("")
{
    // 初始化可显示属性
    initFilterKey();

    // 设置可禁用
    m_CanEnable = true;
}

void DeviceBluetooth::setInfoFromHciconfig(const QMap<QString, QString> &mapInfo)
{
    // 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Manufacturer", m_Vendor);

    // 获取设备其他信息
    getOtherMapInfo(mapInfo);
}

bool DeviceBluetooth::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 判断是不是同一个蓝牙设备，由于条件限制，现在只能通过厂商判断
    QStringList vendor = mapInfo["Vendor"].split(" ");
    if (vendor.size() < 1) {
        return false;
    }
    if (!m_Vendor.contains(vendor[0])) {
        return false;
    }

    // 获取设备基本信息
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "", m_MAC);
    setAttribute(mapInfo, "", m_LogicalName);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "", m_DriverVersion);
    setAttribute(mapInfo, "", m_MaximumPower);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 设置关联到lshw信息的key值,设备的唯一标志
    parseKeyToLshw(mapInfo["SysFS BusID"]);

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBluetooth::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 根据 总线信息 与 设备信息中的唯一key值 判断是否是同一台设备
    if (mapInfo["bus info"] != m_UniqueKey) {
        return false;
    }

    // 获取基本信息
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "", m_MAC);
    setAttribute(mapInfo, "product", m_LogicalName);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "", m_DriverVersion);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);

    return true;
}

const QString &DeviceBluetooth::name()const
{
    return m_Name;
}

const QString &DeviceBluetooth::driver()const
{
    return m_Driver;
}

QString DeviceBluetooth::subTitle()
{
    return m_Name;
}

const QString DeviceBluetooth::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DeviceBluetooth::setEnable(bool e)
{
    // 设置设备状态
    EnableDeviceStatus res = EnableManager::instance()->enableDeviceByDriver(e, m_Driver);
    if (e != enable()) {
        res = EDS_Faild;
    }
    return res;
}

bool DeviceBluetooth::enable()
{
    // 获取设备状态
    m_Enable = EnableManager::instance()->isDeviceEnableByDriver(m_Driver);
    return m_Enable;
}

void DeviceBluetooth::initFilterKey()
{
    // 添加可显示的属性
    addFilterKey(QObject::tr("Bus"));
    addFilterKey(QObject::tr("BD Address"));
    addFilterKey(QObject::tr("ACL MTU"));
    addFilterKey(QObject::tr("SCO MTU"));
    addFilterKey(QObject::tr("Features"));
    addFilterKey(QObject::tr("Packet type"));
    addFilterKey(QObject::tr("Link policy"));
    addFilterKey(QObject::tr("Link mode"));
    addFilterKey(QObject::tr("Class"));
    addFilterKey(QObject::tr("Service Classes"));
    addFilterKey(QObject::tr("Device Class"));
    addFilterKey(QObject::tr("HCI Version"));
    //addFilterKey(QObject::tr("Revision"));
    addFilterKey(QObject::tr("LMP Version"));
    addFilterKey(QObject::tr("Subversion"));

    addFilterKey(QObject::tr("Device"));
    addFilterKey(QObject::tr("Serial ID"));

    addFilterKey(QObject::tr("product"));
    addFilterKey(QObject::tr("description"));
    addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("Class"));
    addFilterKey(QObject::tr("Powered"));
    addFilterKey(QObject::tr("Discoverable"));
    addFilterKey(QObject::tr("Pairable"));
    addFilterKey(QObject::tr("UUID"));
    addFilterKey(QObject::tr("Modalias"));
    addFilterKey(QObject::tr("Discovering"));
}

void DeviceBluetooth::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
}

void DeviceBluetooth::parseKeyToLshw(const QString &info)
{
    // 解析映射到lshw的唯一值

    //1-2:1.0
    QStringList words = info.split(":");
    if (words.size() != 2) {
        return;
    }

    QStringList chs = words[0].split("-");
    if (chs.size() != 2) {
        return;
    }

    // usb@%1:%2
    m_UniqueKey = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
}

void DeviceBluetooth::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver Version"), m_DriverVersion);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Bus Info"), m_BusInfo);
    addOtherDeviceInfo(tr("Logical Name"), m_LogicalName);
    addOtherDeviceInfo(tr("MAC Address"), m_MAC);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceBluetooth::loadTableData()
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
