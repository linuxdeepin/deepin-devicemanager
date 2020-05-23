#include "DeviceComputer.h"
#include <QDebug>

DeviceComputer::DeviceComputer()
    : m_HomeUrl(""), m_OsDescription("")
    , m_OS(""), m_Vendor(""), m_Name("")
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
    if (m_Name.contains("None")) {
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
void DeviceComputer::initFilterKey()
{

}
