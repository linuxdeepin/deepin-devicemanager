#include "DeviceBluetooth.h"
#include<QDebug>

DeviceBluetooth::DeviceBluetooth()
    : DeviceBaseInfo (), m_Name(""), m_Vendor(""), m_Version(""), m_Model("")
    , m_MAC(""), m_LogicalName(""), m_BusInfo(""), m_Capabilities("")
    , m_Driver(""), m_DriverVersion(""), m_MaximumPower(""), m_Speed("")
{
    initFilterKey();
}

void DeviceBluetooth::setInfoFromHciconfig(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHciconfig(mapInfo, info);
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    loadOtherDeviceInfo(mapInfo);
}

bool DeviceBluetooth::setInfoFromHwinfo(const QString &info)
{
    // 先过滤一定不是蓝牙的信息
    if (info.contains("Unclassified device") || info.contains("Bluetooth Device")) {

        QMap<QString, QString> mapInfo;
        getMapInfoFromHwinfo(mapInfo, info);

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

        addHwinfoUniqueID(mapInfo["Unique ID"]);
        addHwinfoBusID(mapInfo["SysFS BusID"]);

        loadOtherDeviceInfo(mapInfo);
        return true;
    }
    return false;
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

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    loadOtherDeviceInfo(mapInfo);
    return true;
}


bool DeviceBluetooth::setInfoFromLshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromLshw(mapInfo, info);

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

void DeviceBluetooth::getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QStringList pairs = line.trimmed().split("  ");
        if (pairs.size() < 1) {
            continue;
        }
        foreach (const QString &pair, pairs) {
            QStringList keyValue = pair.trimmed().split(": ");
            if (keyValue.size() == 2) {
                mapInfo[keyValue[0].trimmed()] = keyValue[1].trimmed();
            }
        }
    }
}
