#include "DeviceMemory.h"

DeviceMemory::DeviceMemory()
    : DeviceBaseInfo()
{
    initFilterKey();
}

void DeviceMemory::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "slot", m_Locator);
    setAttribute(mapInfo, "size", m_Size);
    if (m_Size.contains("GiB")) {
        m_Size.replace("GiB", "GB");
    }
    if (m_Size.contains("MiB")) {
        m_Size.replace("MiB", "");
        double size = m_Size.toDouble() / 1024.0;
        m_Size = QString::number(size, 'g', 0) + QString("GB");
    }
    setAttribute(mapInfo, "clock", m_Speed);
    if (m_Speed.contains("MT/s")) {
        m_Speed.replace("MT/s", "MHz");
    }

    setAttribute(mapInfo, "width", m_TotalBandwidth);
    setAttribute(mapInfo, "width", m_DataBandwidth);
    setAttribute(mapInfo, "serial", m_SerialNumber);

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceMemory::setInfoFromDmidecode(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo["Locator"] != m_Locator) {
        return false;
    }

    setAttribute(mapInfo, "Serial Number", m_SerialNumber);
    setAttribute(mapInfo, "Configured Memory Speed", m_ConfiguredSpeed);
    if (m_ConfiguredSpeed.contains("MT/s")) {
        m_ConfiguredSpeed.replace("MT/s", "MHz");
    }
    setAttribute(mapInfo, "Minimum Voltage", m_MinimumVoltage);
    setAttribute(mapInfo, "Maximum Voltage", m_MaximumVoltage);
    setAttribute(mapInfo, "Configured Voltage", m_ConfiguredVoltage);
    setAttribute(mapInfo, "Total Width", m_TotalBandwidth);
    setAttribute(mapInfo, "Data Width", m_DataBandwidth);
    setAttribute(mapInfo, "Type", m_Type);
    if (m_Type == "<OUT OF SPEC>") {
        m_Type = "";
    }

    loadOtherDeviceInfo(mapInfo);

    return true;
}

void DeviceMemory::initFilterKey()
{

}

const QString &DeviceMemory::name()const
{
    return m_Name;
}
const QString &DeviceMemory::vendor()const
{
    return m_Vendor;
}
const QString &DeviceMemory::size()const
{
    return m_Size;
}
const QString &DeviceMemory::type()const
{
    return m_Type;
}
const QString &DeviceMemory::speed()const
{
    return m_Speed;
}
const QString &DeviceMemory::totalBandwidth()const
{
    return m_TotalBandwidth;
}
const QString &DeviceMemory::dataBandwidth()const
{
    return m_DataBandwidth;
}
const QString &DeviceMemory::locator()const
{
    return m_Locator;
}
const QString &DeviceMemory::serialNumber()const
{
    return m_SerialNumber;
}
const QString &DeviceMemory::configuredSpeed()const
{
    return m_ConfiguredSpeed;
}
const QString &DeviceMemory::minimumVoltage()const
{
    return m_MinimumVoltage;
}
const QString &DeviceMemory::maximumVoltage()const
{
    return m_MaximumVoltage;
}
const QString &DeviceMemory::configuredVoltage()const
{
    return m_ConfiguredVoltage;
}
