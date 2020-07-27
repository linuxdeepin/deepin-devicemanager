#include "DeviceOtherPCI.h"

DeviceOtherPCI::DeviceOtherPCI()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_BusInfo("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Description("")
    , m_Irq("")
    , m_Memory("")
    , m_Driver("")
    , m_Latency("")
    , m_InputOutput("")
{

}

const QString &DeviceOtherPCI::name()const
{
    return m_Name;
}

const QString &DeviceOtherPCI::vendor()const
{
    return m_Vendor;
}

const QString &DeviceOtherPCI::model()const
{
    return m_Model;
}

const QString &DeviceOtherPCI::version()const
{
    return m_Version;
}

const QString &DeviceOtherPCI::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceOtherPCI::irq()const
{
    return m_Irq;
}

const QString &DeviceOtherPCI::memory()const
{
    return m_Memory;
}

const QString &DeviceOtherPCI::width()const
{
    return m_Width;
}

const QString &DeviceOtherPCI::clock()const
{
    return m_Clock;
}

const QString &DeviceOtherPCI::capabilities()const
{
    return m_Capabilities;
}

const QString &DeviceOtherPCI::description()const
{
    return m_Description;
}

const QString &DeviceOtherPCI::driver()const
{
    return m_Driver;
}

const QString &DeviceOtherPCI::latency()const
{
    return m_Latency;
}

const QString &DeviceOtherPCI::inputOutput()const
{
    return m_InputOutput;
}

QString DeviceOtherPCI::subTitle()
{
    return m_Model;
}

void DeviceOtherPCI::initFilterKey()
{

}

void DeviceOtherPCI::loadBaseDeviceInfo()
{
    //    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Version);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Width"), m_Width);
    addBaseDeviceInfo(tr("Clock"), m_Clock);
    addBaseDeviceInfo(tr("Latency"), m_Latency);
    addBaseDeviceInfo(tr("IRQ"), m_Irq);
    addBaseDeviceInfo(tr("Memory"), m_Memory);
    addBaseDeviceInfo(tr("Input/Output"), m_InputOutput);
    //    addBaseDeviceInfo(tr(""), m_Description);
    m_SubTitle = m_Name;
}

void DeviceOtherPCI::loadOtherDeviceInfo()
{

}

void DeviceOtherPCI::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
