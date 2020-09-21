// 项目自身文件
#include "DeviceNetwork.h"

// 其它头文件
#include "EnableManager.h"

DeviceNetwork::DeviceNetwork()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model(""), m_Version("")
    , m_BusInfo("")
    , m_LogicalName("")
    , m_MACAddress("")
    , m_Irq("")
    , m_Memory("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Autonegotiation("")
    , m_Broadcast("")
    , m_Driver("")
    , m_DriverVersion("")
    , m_Duplex("")
    , m_Firmware("")
    , m_Port("")
    , m_Link("")
    , m_Ip("")
    , m_Speed("")
    , m_Capacity("")
    , m_Latency("")
    , m_Multicast("")
{
    initFilterKey();
    m_CanEnable = true;
}

void DeviceNetwork::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    //
    setAttribute(mapInfo, "description", m_Model);
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "description", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "logical name", m_LogicalName);
    setAttribute(mapInfo, "serial", m_MACAddress);
    setAttribute(mapInfo, "irq", m_Irq);
    setAttribute(mapInfo, "memory", m_Memory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "autonegotiation", m_Autonegotiation);
    setAttribute(mapInfo, "broadcast", m_Broadcast);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "driverversion", m_DriverVersion);
    setAttribute(mapInfo, "duplex", m_Duplex);
    setAttribute(mapInfo, "firmware", m_Firmware);
    setAttribute(mapInfo, "port", m_Port);
    setAttribute(mapInfo, "link", m_Link);
    setAttribute(mapInfo, "ip", m_Ip);
    setAttribute(mapInfo, "size", m_Speed);
    setAttribute(mapInfo, "capacity", m_Capacity);
    setAttribute(mapInfo, "latency", m_Latency);
    setAttribute(mapInfo, "multicast", m_Multicast);

    getOtherMapInfo(mapInfo);
}

bool DeviceNetwork::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo["Device File"] != m_LogicalName) {
        return false;
    }

    setAttribute(mapInfo, "Model", m_Name);
    return true;
}

const QString &DeviceNetwork::name()const
{
    return m_Name;
}

const QString &DeviceNetwork::vendor()const
{
    return m_Vendor;
}

const QString &DeviceNetwork::model()const
{
    return m_Model;
}

const QString &DeviceNetwork::version()const
{
    return m_Version;
}

const QString &DeviceNetwork::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceNetwork::logicalName()const
{
    return m_LogicalName;
}

const QString &DeviceNetwork::MACAddress()const
{
    return m_MACAddress;
}

const QString &DeviceNetwork::irq()const
{
    return m_Irq;
}

const QString &DeviceNetwork::memory()const
{
    return m_Memory;
}

const QString &DeviceNetwork::width()const
{
    return m_Width;
}

const QString &DeviceNetwork::clock()const
{
    return m_Clock;
}

const QString &DeviceNetwork::capabilities()const
{
    return m_Capabilities;
}

const QString &DeviceNetwork::autoNegotiation()const
{
    return m_Autonegotiation;
}

const QString &DeviceNetwork::broadcast()const
{
    return m_Broadcast;
}

const QString &DeviceNetwork::driver()const
{
    return m_Driver;
}

const QString &DeviceNetwork::driverVersion()const
{
    return m_DriverVersion;
}

const QString &DeviceNetwork::duplex()const
{
    return m_Duplex;
}

const QString &DeviceNetwork::firmware()const
{
    return m_Firmware;
}

const QString &DeviceNetwork::port()const
{
    return m_Port;
}

const QString &DeviceNetwork::link()const
{
    return m_Link;
}

const QString &DeviceNetwork::ip()const
{
    return m_Ip;
}

const QString &DeviceNetwork::speed()const
{
    return m_Speed;
}

const QString &DeviceNetwork::capacity()const
{
    return m_Capacity;
}

const QString &DeviceNetwork::latency()const
{
    return m_Latency;
}

const QString &DeviceNetwork::multicast()const
{
    return m_Multicast;
}

QString DeviceNetwork::subTitle()
{
    return m_Name;
}

const QString DeviceNetwork::getOverviewInfo()
{
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DeviceNetwork::setEnable(bool e)
{
    return EnableManager::instance()->enableNetworkByIfconfig(m_LogicalName, e);
}

bool DeviceNetwork::enable()
{
//    m_Enable = EnableManager::instance()->isDeviceEnableByDriver(m_Driver);
    m_Enable = EnableManager::instance()->isNetworkEnableByIfconfig(m_LogicalName);
    return m_Enable;
}

void DeviceNetwork::initFilterKey()
{
    addFilterKey(QObject::tr("ioport"));
    addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("network"));
}

void DeviceNetwork::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Type"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Driver Version"), m_DriverVersion);
}

void DeviceNetwork::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Capacity"), m_Capacity);
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Port"), m_Port);
    addOtherDeviceInfo(tr("Multicast"), m_Multicast);
    addOtherDeviceInfo(tr("Link"), m_Link);
    addOtherDeviceInfo(tr("Latency"), m_Latency);
    addOtherDeviceInfo(tr("IP"), m_Ip);
    addOtherDeviceInfo(tr("Firmware"), m_Firmware);
    addOtherDeviceInfo(tr("Duplex"), m_Duplex);
    addOtherDeviceInfo(tr("Broadcast"), m_Broadcast);
    addOtherDeviceInfo(tr("Auto Negotiation"), m_Autonegotiation);
    addOtherDeviceInfo(tr("Clock"), m_Clock);
    addOtherDeviceInfo(tr("Width"), m_Width);
    addOtherDeviceInfo(tr("Memory"), m_Memory);
    addOtherDeviceInfo(tr("IRQ"), m_Irq);
    addOtherDeviceInfo(tr("MAC Address"), m_MACAddress);
    addOtherDeviceInfo(tr("Logical Name"), m_LogicalName);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceNetwork::loadTableData()
{
    QString name;
    if (!enable()) {
        name = "(" + tr("Disable") + ") " + m_Name;
    } else {
        name = m_Name;
    }
    m_TableData.append(name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
