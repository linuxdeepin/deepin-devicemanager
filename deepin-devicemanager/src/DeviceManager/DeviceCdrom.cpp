// 项目自身文件
#include "DeviceCdrom.h"

DeviceCdrom::DeviceCdrom()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Type("")
    , m_Version("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Driver("")
    , m_MaxPower("")
    , m_Speed("")
    , m_UnikeyKey("")
    , m_KeyToLshw("")
{
    // 初始化可显示属性
    initFilterKey();
}

bool DeviceCdrom::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    // 通过总线信息判断是否是同一台设备
    if (mapInfo.find("bus info") == mapInfo.end()) {
        return false;
    }

    // 解析总线信息
    QStringList words = mapInfo["bus info"].split("@");
    if (words.size() != 2) {
        return false;
    }

    QString busInfo = words[1].replace(".", ":").trimmed();
    if (m_KeyToLshw != busInfo) {
        return false;
    }

    // 获取设备的基本信息
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor, false);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "version", m_Version, false);
    setAttribute(mapInfo, "bus info", m_BusInfo, false);
    setAttribute(mapInfo, "capabilities", m_Capabilities, false);
    setAttribute(mapInfo, "", m_Driver);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Speed);

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
    return true;
}

void DeviceCdrom::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 获取设备的基本信息
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

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

const QString &DeviceCdrom::name()const
{
    return m_Name;
}

const QString &DeviceCdrom::driver()const
{
    return m_Driver;
}

QString DeviceCdrom::subTitle()
{
    return m_Name;
}

const QString DeviceCdrom::getOverviewInfo()
{
    return m_Name;
}


void DeviceCdrom::initFilterKey()
{
    // 添加可显示的属性
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
    // 添加基本信息
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
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceCdrom::loadTableData()
{
    // 加载表格内容
    QString name;
    if (!enable()) {
        name = "(" + tr("Disable") + ") " + m_Name;
    } else {
        name = m_Name;
    }

    m_TableData.append(name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Type);
}

