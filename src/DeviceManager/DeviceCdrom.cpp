#include "DeviceCdrom.h"

DeviceCdrom::DeviceCdrom()
{
    initFilterKey();
}

bool DeviceCdrom::setInfoFromLshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromLshw(mapInfo, info);

    if (mapInfo.find("bus info") == mapInfo.end()) {
        return false;
    }
    QStringList words = mapInfo["bus info"].split("@");
    if (words.size() != 2) {
        return false;
    }
    if (m_UnikeyKey != words[1].replace(":", ".").trimmed()) {
        return false;
    }

    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "", m_Driver);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Speed);

    loadOtherDeviceInfo(mapInfo);
    return true;
}


void DeviceCdrom::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Type);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "Speed", m_Speed);

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    loadOtherDeviceInfo(mapInfo);
}
const QString &DeviceCdrom::name()const
{
    return m_Name;
}
const QString &DeviceCdrom::vendor()const
{
    return m_Vendor;
}
const QString &DeviceCdrom::type()const
{
    return m_Type;
}
const QString &DeviceCdrom::version()const
{
    return m_Version;
}
const QString &DeviceCdrom::busInfo()const
{
    return m_BusInfo;
}
const QString &DeviceCdrom::capabilities()const
{
    return m_Capabilities;
}
const QString &DeviceCdrom::driver()const
{
    return m_Driver;
}
const QString &DeviceCdrom::maxPower()const
{
    return m_MaxPower;
}
const QString &DeviceCdrom::speed()const
{
    return m_Speed;
}

void DeviceCdrom::initFilterKey()
{
    addFilterKey(QObject::tr("Serial ID"));
    addFilterKey(QObject::tr("Driver Modules"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Device Files"));
    addFilterKey(QObject::tr("Device Number"));
    addFilterKey(QObject::tr("Module Alias"));
    addFilterKey(QObject::tr("Config Status"));
    addFilterKey(QObject::tr("Application"));
    addFilterKey(QObject::tr("physical id"));
}
