#include "DeviceGpu.h"
#include<QDebug>

DeviceGpu::DeviceGpu()
    : DeviceBaseInfo (), m_Name(""), m_Vendor(""), m_Model(""), m_Version(""), m_GraphicsMemory(""), m_Width(""), m_DisplayPort("Unable")
    , m_Clock(""), m_IRQ(""), m_Capabilities(""), m_DisplayOutput(""), m_VGA("Unable"), m_HDMI("Unable"), m_eDP("Unable")
    , m_Description(""), m_Driver(""), m_CurrentResolution(""), m_MinimumResolution(""), m_MaximumResolution(""), m_UniqueKey("")
{
    initFilterKey();
}

void DeviceGpu::initFilterKey()
{
    addFilterKey(QObject::tr("Device"));
    addFilterKey(QObject::tr("SubVendor"));
    addFilterKey(QObject::tr("SubDevice"));
    addFilterKey(QObject::tr("Driver Modules"));
//    addFilterKey(QObject::tr("I/O Ports"));
    addFilterKey(QObject::tr("Config Status"));
    addFilterKey(QObject::tr("latency"));
}

void DeviceGpu::setLshwInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromLshw(mapInfo, info);

    // 判断是否是同一个gpu
    QRegExp re(":[0-9]{2}:[0-9]{2}");
    int index = mapInfo["bus info"].indexOf(re);
    QString uniqueKey = mapInfo["bus info"].mid(index + 1);
    if (!uniqueKey.contains(m_UniqueKey)) {
        return;
    }

    // 设置属性
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_GraphicsMemory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "", m_DisplayPort);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "irq", m_IRQ);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "", m_DisplayOutput);
    setAttribute(mapInfo, "", m_VGA);
    setAttribute(mapInfo, "", m_HDMI);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "", m_CurrentResolution);
    setAttribute(mapInfo, "", m_MinimumResolution);
    setAttribute(mapInfo, "", m_MaximumResolution);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "ioport", m_IOPort);
    setAttribute(mapInfo, "memory", m_MemAddress);
    setAttribute(mapInfo, "physical id", m_PhysID);

    loadOtherDeviceInfo(mapInfo);
}

void DeviceGpu::setLshwInfo(const QMap<QString, QString> &mapInfo)
{

    // 判断是否是同一个gpu
    QRegExp re(":[0-9]{2}:[0-9]{2}");
    int index = mapInfo["bus info"].indexOf(re);
    QString uniqueKey = mapInfo["bus info"].mid(index + 1);
    if (!uniqueKey.contains(m_UniqueKey)) {
        return;
    }

    // 设置属性
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_GraphicsMemory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "", m_DisplayPort);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "irq", m_IRQ);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "", m_DisplayOutput);
    setAttribute(mapInfo, "", m_VGA);
    setAttribute(mapInfo, "", m_HDMI);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "", m_CurrentResolution);
    setAttribute(mapInfo, "", m_MinimumResolution);
    setAttribute(mapInfo, "", m_MaximumResolution);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "ioport", m_IOPort);
    setAttribute(mapInfo, "memory", m_MemAddress);
    setAttribute(mapInfo, "physical id", m_PhysID);

    loadOtherDeviceInfo(mapInfo);
}

void DeviceGpu::setHwinfoInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

    // 设置属性
    setAttribute(mapInfo, "Vendor", m_Vendor, false);
    setAttribute(mapInfo, "Model", m_Name, false);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version, false);
    setAttribute(mapInfo, "IRQ", m_IRQ, false);
    setAttribute(mapInfo, "Driver", m_Driver, false);
    setAttribute(mapInfo, "", m_GraphicsMemory);
    setAttribute(mapInfo, "", m_DisplayOutput);
    setAttribute(mapInfo, "", m_VGA);
    setAttribute(mapInfo, "", m_HDMI);
    setAttribute(mapInfo, "", m_DisplayPort);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_CurrentResolution);
    setAttribute(mapInfo, "", m_MinimumResolution);
    setAttribute(mapInfo, "", m_MaximumResolution);

    // 获取 m_UniqueKey
    QRegExp re(":[0-9]{2}:[0-9]{2}");
    int index = mapInfo["SysFS BusID"].indexOf(re);
    m_UniqueKey = mapInfo["SysFS BusID"].mid(index + 1);

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    loadOtherDeviceInfo(mapInfo);
}

bool DeviceGpu::setHwinfoInfo(const QMap<QString, QString> &mapInfo)
{
    // 设置属性
    setAttribute(mapInfo, "Vendor", m_Vendor, false);
    setAttribute(mapInfo, "Model", m_Name, false);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version, false);
    setAttribute(mapInfo, "IRQ", m_IRQ, false);
    setAttribute(mapInfo, "Driver", m_Driver, false);
    setAttribute(mapInfo, "", m_GraphicsMemory);
    setAttribute(mapInfo, "", m_DisplayOutput);
    setAttribute(mapInfo, "", m_VGA);
    setAttribute(mapInfo, "", m_HDMI);
    setAttribute(mapInfo, "", m_DisplayPort);
    setAttribute(mapInfo, "", m_Clock);
    setAttribute(mapInfo, "", m_CurrentResolution);
    setAttribute(mapInfo, "", m_MinimumResolution);
    setAttribute(mapInfo, "", m_MaximumResolution);

    // 获取 m_UniqueKey
    QRegExp re(":[0-9]{2}:[0-9]{2}");
    int index = mapInfo["SysFS BusID"].indexOf(re);
    m_UniqueKey = mapInfo["SysFS BusID"].mid(index + 1);

    addHwinfoUniqueID(mapInfo["Unique ID"]);
    addHwinfoBusID(mapInfo["SysFS BusID"]);

    loadOtherDeviceInfo(mapInfo);
}

void DeviceGpu::setXrandrInfo(const QString &info)
{
    QStringList lines = info.split("\n");
    foreach (const QString &line, lines) {
        QRegExp reline("[a-zA-Z]{1,3}.*");
        if (!reline.exactMatch(line)) {
            continue;
        }
        if (line.startsWith("Screen")) {
            QRegExp re(".*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*([0-9]{3,5}\\sx\\s[0-9]{3,5}).*");
            if (re.exactMatch(info)) {
                m_MinimumResolution = re.cap(1);
                m_CurrentResolution = re.cap(2);
                m_MaximumResolution = re.cap(3);
            }
        } else if (line.startsWith("HDMI")) {
            m_HDMI = "Enable";
        } else if (line.startsWith("VGA")) {
            m_VGA = "Enable";
        } else if (line.startsWith("DP")) {
            m_DisplayPort = "Enable";
        } else if (line.startsWith("eDP")) {
            m_eDP = "Enable";
        }
    }
}

void DeviceGpu::setXrandrInfo(const QMap<QString, QString> &mapInfo)
{
    m_MinimumResolution = mapInfo["minResolution"];
    m_CurrentResolution = mapInfo["curResolution"];
    m_MaximumResolution = mapInfo["maxResolution"];
    if (mapInfo.find("HDMI") != mapInfo.end()) {
        m_HDMI = mapInfo["HDMI"];
    }
    if (mapInfo.find("VGA") != mapInfo.end()) {
        m_HDMI = mapInfo["VGA"];
    }
    if (mapInfo.find("DP") != mapInfo.end()) {
        m_HDMI = mapInfo["DP"];
    }
    if (mapInfo.find("eDP") != mapInfo.end()) {
        m_HDMI = mapInfo["eDP"];
    }
}

void DeviceGpu::setDmesgInfo(const QString &info)
{
    m_GraphicsMemory = info;
}

const QString &DeviceGpu::name() const
{
    return m_Name;
}
const QString &DeviceGpu::vendor() const
{
    return m_Vendor;
}
const QString &DeviceGpu::model() const
{
    return m_Model;
}
const QString &DeviceGpu::version() const
{
    return m_Version;
}
const QString &DeviceGpu::graphicsMemory() const
{
    return m_GraphicsMemory;
}
const QString &DeviceGpu::busWidth() const
{
    return m_Width;
}
const QString &DeviceGpu::IRQ() const
{
    return m_IRQ;
}
const QString &DeviceGpu::capabilities() const
{
    return m_Capabilities;
}
const QString &DeviceGpu::displayOutput() const
{
    return m_DisplayOutput;
}
const QString &DeviceGpu::VGA() const
{
    return m_VGA;
}
const QString &DeviceGpu::HDMI() const
{
    return m_HDMI;
}
const QString &DeviceGpu::eDP() const
{
    return m_eDP;
}
const QString &DeviceGpu::displayPort() const
{
    return m_DisplayPort;
}
const QString &DeviceGpu::clock() const
{
    return m_Clock;
}
const QString &DeviceGpu::description() const
{
    return m_Description;
}
const QString &DeviceGpu::driver() const
{
    return m_Driver;
}
const QString &DeviceGpu::currentResolution() const
{
    return m_CurrentResolution;
}
const QString &DeviceGpu::minimumResolution() const
{
    return m_MinimumResolution;
}
const QString &DeviceGpu::maximumResolution() const
{
    return m_MaximumResolution;
}

const QString &DeviceGpu::busInfo() const
{
    return m_BusInfo;
}
const QString &DeviceGpu::ioPort() const
{
    return  m_IOPort;
}
const QString &DeviceGpu::memory() const
{
    return m_MemAddress;
}
const QString &DeviceGpu::physID() const
{
    return m_PhysID;
}
