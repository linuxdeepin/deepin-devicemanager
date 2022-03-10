#include "DeviceComputer.h"
#include <QDebug>

DeviceComputer::DeviceComputer()
    : m_HomeUrl("")
    , m_OsDescription("")
    , m_OS("")
    , m_Vendor("")
    , m_Name("")
    ,  m_Type("")
{

}

const QString &DeviceComputer::homeUrl()const
{
    return m_HomeUrl;
}

const QString &DeviceComputer::osDescription()
{
    return m_OsDescription;
}

const QString &DeviceComputer::os()
{
    return m_OS;
}

const QString &DeviceComputer::vendor()
{
    return m_Vendor;
}

const QString &DeviceComputer::name()
{
    if (m_Name.contains("None", Qt::CaseInsensitive)) {
        m_Name = "";
    }
    return m_Name;
}

const QString &DeviceComputer::type()
{
    return m_Type;
}

void DeviceComputer::setHomeUrl(const QString &value)
{
    m_HomeUrl = value;
}

void DeviceComputer::setOsDescription(const QString &value)
{
    m_OsDescription = value;
}

void DeviceComputer::setOS(const QString &value)
{
    m_OS = value;
}

void DeviceComputer::setVendor(const QString &value)
{
    m_Vendor = value;
}

void DeviceComputer::setName(const QString &value)
{
    m_Name = value;
}

void DeviceComputer::setType(const QString &value)
{
    m_Type = value;
}

void DeviceComputer::setVendor(const QString &dm1Vendor, const QString &dm2Vendor)
{
    if (dm1Vendor.contains("System manufacturer")) {
        m_Vendor = dm2Vendor;
    } else {
        m_Vendor = dm1Vendor;
    }
}

void DeviceComputer::setName(const QString &dm1Name, const QString &dm2Name, const QString &dm1Family, const QString &dm1Version)
{
    // name
    QString pname;
    if (dm1Name.contains("System Product Name")) {
        pname = dm2Name;
    } else {
        pname = dm1Name;
    }

    // family
    QString family = dm1Family;
    if (dm1Family.contains("unknown", Qt::CaseInsensitive) \
            || dm1Family.contains("System Version") \
            || dm1Family.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Family.contains("x.x", Qt::CaseInsensitive) \
            || dm1Family.contains("Not Applicable", Qt::CaseInsensitive)) {
        family = "";
    }

    // version
    QString version = dm1Version;
    if (dm1Version.contains("unknown", Qt::CaseInsensitive) \
            || dm1Version.contains("System Version") \
            || dm1Version.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Version.contains("x.x", Qt::CaseInsensitive) \
            || dm1Version.contains("Not Applicable", Qt::CaseInsensitive)) {
        version = "";
    }

    pname = pname.remove(m_Vendor, Qt::CaseInsensitive);
    pname = pname.remove(version, Qt::CaseInsensitive);
    version = version.remove(m_Vendor, Qt::CaseInsensitive);
    version = version.remove(family, Qt::CaseInsensitive);
    family = family.remove(m_Vendor, Qt::CaseInsensitive);
    family = family.remove(version, Qt::CaseInsensitive);

    m_Name = family + " " + version + " " + pname;
}

void DeviceComputer::initFilterKey()
{

}
