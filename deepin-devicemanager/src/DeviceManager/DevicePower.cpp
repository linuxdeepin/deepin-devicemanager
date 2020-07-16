#include "DevicePower.h"
#include<QFileInfo>

DevicePower::DevicePower()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Type("")
    , m_SerialNumber("")
    , m_ElectricType("")
    , m_MaxPower("")
    , m_Status("")
    , m_Enabled("")
    , m_HotSwitch("")
    , m_Capacity("")
    , m_Voltage("")
    , m_Slot("")
    , m_DesignCapacity("")
    , m_DesignVoltage("")
    , m_SBDSChemistry("")
    , m_SBDSManufactureDate("")
    , m_SBDSSerialNumber("")
    , m_SBDSVersion("")
{
    initFilterKey();
}

bool DevicePower::setInfoFromUpower(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo["Device"].contains("line_power", Qt::CaseInsensitive)) {
        return false;
    }
    m_Name = QObject::tr("battery");

    setAttribute(mapInfo, "", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "serial", m_SerialNumber);
    setAttribute(mapInfo, "", m_ElectricType);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Status);
    setAttribute(mapInfo, "", m_Enabled);
    setAttribute(mapInfo, "", m_HotSwitch);
    setAttribute(mapInfo, "capacity", m_Capacity);
    setAttribute(mapInfo, "voltage", m_Voltage);
    setAttribute(mapInfo, "", m_Slot);
//    setAttribute(mapInfo, "capacity", m_DesignCapacity);
//    setAttribute(mapInfo, "voltage", m_DesignVoltage);
    setAttribute(mapInfo, "", m_SBDSChemistry);
    setAttribute(mapInfo, "", m_SBDSManufactureDate);
    setAttribute(mapInfo, "", m_SBDSSerialNumber);
    setAttribute(mapInfo, "", m_SBDSVersion);
    getOtherMapInfo(mapInfo);
    return true;
}

void DevicePower::setDaemonInfo(const QMap<QString, QString> &mapInfo)
{
    if (m_Name == QObject::tr("battery"))
        getOtherMapInfo(mapInfo);
}

const QString &DevicePower::name()const
{
    return m_Name;
}

const QString &DevicePower::vendor()const
{
    return m_Vendor;
}

const QString &DevicePower::model()const
{
    return m_Model;
}

const QString &DevicePower::type()const
{
    return m_Type;
}

const QString &DevicePower::serialNumber()const
{
    return m_SerialNumber;
}

const QString &DevicePower::electircType()const
{
    return m_ElectricType;
}

const QString &DevicePower::maxPower()const
{
    return m_MaxPower;
}

const QString &DevicePower::status()const
{
    return m_Status;
}

const QString &DevicePower::enabled()const
{
    return m_Enabled;
}

const QString &DevicePower::hotSwitch()const
{
    return m_HotSwitch;
}

const QString &DevicePower::capacity()const
{
    return  m_Capacity;
}

const QString &DevicePower::voltage()const
{
    return m_Voltage;
}

const QString &DevicePower::slot()const
{
    return m_Slot;
}

const QString &DevicePower::designCapacity()const
{
    return m_DesignCapacity;
}

const QString &DevicePower::designVoltage()const
{
    return m_DesignVoltage;
}

const QString &DevicePower::SBDSChemistry()const
{
    return m_SBDSChemistry;
}

const QString &DevicePower::SBDSManufactureDate()const
{
    return m_SBDSManufactureDate;
}

const QString &DevicePower::SBDSSerialNumber()const
{
    return m_SBDSSerialNumber;
}

const QString &DevicePower::SBDSVersion()const
{
    return m_SBDSVersion;
}

void DevicePower::initFilterKey()
{
    addFilterKey(QObject::tr("native-path"));
    addFilterKey(QObject::tr("power supply"));
    addFilterKey(QObject::tr("updated"));
    addFilterKey(QObject::tr("has history"));
    addFilterKey(QObject::tr("has statistics"));
    addFilterKey(QObject::tr("rechargeable"));
    addFilterKey(QObject::tr("state"));
    addFilterKey(QObject::tr("warning-level"));
    addFilterKey(QObject::tr("energy"));
    addFilterKey(QObject::tr("energy-empty"));
    addFilterKey(QObject::tr("energy-full"));
    addFilterKey(QObject::tr("energy-full-design"));
    addFilterKey(QObject::tr("energy-rate"));
    addFilterKey(QObject::tr("voltage"));
    addFilterKey(QObject::tr("percentage"));
    addFilterKey(QObject::tr("temperature"));
    addFilterKey(QObject::tr("technology"));
    addFilterKey(QObject::tr("icon-name"));
    addFilterKey(QObject::tr("online"));
    addFilterKey(QObject::tr("daemon-version"));
    addFilterKey(QObject::tr("on-battery"));
    addFilterKey(QObject::tr("lid-is-closed"));
    addFilterKey(QObject::tr("lid-is-present"));
    addFilterKey(QObject::tr("critical-action"));
}

void DevicePower::loadBaseDeviceInfo()
{
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Serial Number"), m_SerialNumber);
    addBaseDeviceInfo(tr("Type"), m_Type);
    addBaseDeviceInfo(tr("Status"), m_Status);
    addBaseDeviceInfo(tr("Capacity"), m_Capacity);
    addBaseDeviceInfo(tr("Voltage"), m_Voltage);
    addBaseDeviceInfo(tr("Slot"), m_Slot);
    addBaseDeviceInfo(tr("Design Capacity"), m_DesignCapacity);
    addBaseDeviceInfo(tr("Design Voltage"), m_DesignVoltage);
    addBaseDeviceInfo(tr("SBDS Version"), m_SBDSVersion);
    addBaseDeviceInfo(tr("SBDS Serial Number"), m_SBDSSerialNumber);
    addBaseDeviceInfo(tr("SBDS Manufacture Date"), m_SBDSManufactureDate);
    addBaseDeviceInfo(tr("SBDS Chemistry"), m_SBDSChemistry);
//    addBaseDeviceInfo(tr(""), m_ElectricType);
//    addBaseDeviceInfo(tr(""), m_MaxPower);
//    addBaseDeviceInfo(tr(""), m_Enabled);
//    addBaseDeviceInfo(tr(""), m_HotSwitch);
    m_SubTitle = m_Name;
}

void DevicePower::loadOtherDeviceInfo()
{

}

void DevicePower::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
