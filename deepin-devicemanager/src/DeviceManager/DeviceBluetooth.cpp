#include "DeviceBluetooth.h"
#include<QDebug>

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
    , m_Driver("")
    , m_DriverVersion("")
    , m_MaximumPower("")
    , m_Speed("")
{
    initFilterKey();
}

void DeviceBluetooth::setInfoFromHciconfig(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
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

    parseKeyToLshw(mapInfo["SysFS BusID"]);

    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBluetooth::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo["bus info"] != m_UniqueKey) {
        return false;
    }

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

const QString &DeviceBluetooth::vendor()const
{
    return m_Vendor;
}

const QString &DeviceBluetooth::version()const
{
    return m_Version;
}

const QString &DeviceBluetooth::model()const
{
    return m_Model;
}

const QString &DeviceBluetooth::MAC()const
{
    return m_MAC;
}

const QString &DeviceBluetooth::logicalName()const
{
    return m_LogicalName;
}

const QString &DeviceBluetooth::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceBluetooth::capabilities()const
{
    return m_Capabilities;
}

const QString &DeviceBluetooth::driver()const
{
    return m_Driver;
}

const QString &DeviceBluetooth::driverVersion()const
{
    return m_DriverVersion;
}

const QString &DeviceBluetooth::maximumPower()const
{
    return m_MaximumPower;
}

const QString &DeviceBluetooth::speed()const
{
    return m_Speed;
}

void DeviceBluetooth::initFilterKey()
{
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
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("MAC Address"), m_MAC);
    addBaseDeviceInfo(tr("Logical Name"), m_LogicalName);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Driver Version"), m_DriverVersion);
    addBaseDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
}

void DeviceBluetooth::parseKeyToLshw(const QString &info)
{
    //1-2:1.0
    QStringList words = info.split(":");
    if (words.size() != 2) {
        return;
    }

    QStringList chs = words[0].split("-");
    if (chs.size() != 2) {
        return;
    }
    m_UniqueKey = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
}

void DeviceBluetooth::loadOtherDeviceInfo()
{

}

void DeviceBluetooth::loadTableData()
{

}
