#include "DeviceImage.h"

DeviceImage::DeviceImage()
    : DeviceBaseInfo (), m_Name(""), m_Vendor(""), m_Model(""), m_Version("")
    , m_BusInfo(""), m_Capabilities(""), m_Driver(""), m_MaximumPower(""), m_Speed("")
{

}

void DeviceImage::setInfoFromLshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfo(mapInfo, info);

    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "product", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
}

void DeviceImage::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "", m_MaximumPower);
    setAttribute(mapInfo, "Speed", m_Speed);

    addHwinfoUniqueID(mapInfo["Unique ID"]);
}

const QString &DeviceImage::name()const
{
    return m_Name;
}
const QString &DeviceImage::vendor()const
{
    return m_Vendor;
}
const QString &DeviceImage::model()const
{
    return m_Model;
}
const QString &DeviceImage::version()const
{
    return m_Version;
}
const QString &DeviceImage::busInfo()const
{
    return m_BusInfo;
}
const QString &DeviceImage::capabilities()const
{
    return m_Capabilities;
}
const QString &DeviceImage::driver()const
{
    return m_Driver;
}
const QString &DeviceImage::maxinumPower()const
{
    return m_MaximumPower;
}
const QString &DeviceImage::speed()const
{
    return m_Speed;
}

void DeviceImage::initFilterKey()
{

}
