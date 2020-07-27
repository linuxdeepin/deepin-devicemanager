#include "DeviceCdrom.h"

DeviceCdrom::DeviceCdrom()
{
    initFilterKey();
}

bool DeviceCdrom::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("bus info") == mapInfo.end()) {
        return false;
    }
    QStringList words = mapInfo["bus info"].split("@");
    if (words.size() != 2) {
        return false;
    }
    QString busInfo = words[1].replace(".", ":").trimmed();
    if (m_KeyToLshw != busInfo) {
        return false;
    }

    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor, false);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "version", m_Version, false);
    setAttribute(mapInfo, "bus info", m_BusInfo, false);
    setAttribute(mapInfo, "capabilities", m_Capabilities, false);
    setAttribute(mapInfo, "", m_Driver);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Speed);

    getOtherMapInfo(mapInfo);
    return true;
}

void DeviceCdrom::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Type);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    m_KeyToLshw = mapInfo["SysFS BusID"];

    getOtherMapInfo(mapInfo);
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

QString DeviceCdrom::subTitle()
{
    return m_Name;
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

    addFilterKey(QObject::tr("status"));
    addFilterKey(QObject::tr("logical name"));
//    addFilterKey(QObject::tr("bus info"));
    addFilterKey(QObject::tr("ansiversion"));
}

void DeviceCdrom::loadBaseDeviceInfo()
{
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Type);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Maximum Power"), m_MaxPower);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
}

void DeviceCdrom::loadOtherDeviceInfo()
{

}

void DeviceCdrom::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Type);
}

