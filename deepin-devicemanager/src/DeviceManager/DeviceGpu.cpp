// 项目自身文件
#include "DeviceGpu.h"

// Qt库文件
#include<QDebug>

DeviceGpu::DeviceGpu()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_GraphicsMemory("")
    , m_Width("")
    , m_DisplayPort("Unable")
    , m_Clock("")
    , m_IRQ("")
    , m_Capabilities("")
    , m_DisplayOutput("")
    , m_VGA("Unable")
    , m_HDMI("Unable")
    , m_eDP("Unable")
    , m_DVI("Unable")
    , m_Description("")
    , m_Driver("")
    , m_CurrentResolution("")
    , m_MinimumResolution("")
    , m_MaximumResolution("")
    , m_UniqueKey("")
{
    // 初始化可显示属性
    initFilterKey();
}

void DeviceGpu::initFilterKey()
{
    // 添加可显示属性
    addFilterKey(QObject::tr("Device"));
    addFilterKey(QObject::tr("SubVendor"));
    addFilterKey(QObject::tr("SubDevice"));
    addFilterKey(QObject::tr("Driver Modules"));
    addFilterKey(QObject::tr("Config Status"));
    addFilterKey(QObject::tr("latency"));

    // gpuinfo 华为KLU和PanGuV
    addFilterKey(QObject::tr("GDDR capacity"));
    addFilterKey(QObject::tr("GPU vendor"));
    addFilterKey(QObject::tr("GPU type"));
    addFilterKey(QObject::tr("EGL version"));
    addFilterKey(QObject::tr("EGL client APIs"));
    addFilterKey(QObject::tr("GL version"));
    addFilterKey(QObject::tr("GLSL version"));
}

void DeviceGpu::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Graphics Memory"), m_GraphicsMemory);
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

    // 获取其他属性
    getOtherMapInfo(mapInfo);
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

    getOtherMapInfo(mapInfo);
    return true;
}

void DeviceGpu::setXrandrInfo(const QMap<QString, QString> &mapInfo)
{
    // 设置分辨率属性
    m_MinimumResolution = mapInfo["minResolution"];
    m_CurrentResolution = mapInfo["curResolution"];
    m_MaximumResolution = mapInfo["maxResolution"];

    // 设置显卡支持的接口
    if (mapInfo.find("HDMI") != mapInfo.end()) {
        m_HDMI = mapInfo["HDMI"];
    }

    if (mapInfo.find("VGA") != mapInfo.end()) {
        m_VGA = mapInfo["VGA"];
    }

    if (mapInfo.find("DP") != mapInfo.end()) {
        m_DisplayPort = mapInfo["DP"];
    }

    if (mapInfo.find("eDP") != mapInfo.end()) {
        m_eDP = mapInfo["eDP"];
    }

    if (mapInfo.find("DVI") != mapInfo.end()) {
        m_DVI = mapInfo["DVI"];
    }
}

void DeviceGpu::setDmesgInfo(const QString &info)
{
    // 设置显存大小
    m_GraphicsMemory = info;
}

void DeviceGpu::setGpuInfo(const QMap<QString, QString> &mapInfo)
{
    // 华为KLU和PanGuV机器中不需要显示以下信息
    m_HDMI = "";
    m_VGA = "";
    m_DisplayPort = "";
    m_eDP = "";
    m_DVI = "";

    setAttribute(mapInfo, "Name", m_Name);

    // 获取其他属性
    getOtherMapInfo(mapInfo);
}

const QString &DeviceGpu::name() const
{
    return m_Name;
}

const QString &DeviceGpu::driver() const
{
    return m_Driver;
}

QString DeviceGpu::subTitle()
{
    return m_Model;
}

const QString DeviceGpu::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceGpu::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Memory"), m_MemAddress);
    addOtherDeviceInfo(tr("IO Port"), m_IOPort);
    addOtherDeviceInfo(tr("Bus Info"), m_BusInfo);
    addOtherDeviceInfo(tr("Maximum Resolution"), m_MaximumResolution);
    addOtherDeviceInfo(tr("Minimum Resolution"), m_MinimumResolution);
    addOtherDeviceInfo(tr("Current Resolution"), m_CurrentResolution);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Description"), m_Description);
    addOtherDeviceInfo(tr("Clock"), m_Clock);
    addOtherDeviceInfo(tr("DP"), m_DisplayPort);
    addOtherDeviceInfo(tr("eDP"), m_eDP);
    addOtherDeviceInfo(tr("HDMI"), m_HDMI);
    addOtherDeviceInfo(tr("VGA"), m_VGA);
    addOtherDeviceInfo(tr("DVI"), m_DVI);
    addOtherDeviceInfo(tr("Display Output"), m_DisplayOutput);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("IRQ"), m_IRQ);
    addOtherDeviceInfo(tr("Width"), m_Width);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceGpu::loadTableData()
{
    // 加载表格内容
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
