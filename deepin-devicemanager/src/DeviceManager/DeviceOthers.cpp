// 项目自身文件
#include "DeviceOthers.h"

DeviceOthers::DeviceOthers()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_LogicalName("")
{

}

void DeviceOthers::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (m_BusInfo.isEmpty() || m_BusInfo != mapInfo["bus info"]) {
        return;
    }

    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor, false);
    setAttribute(mapInfo, "product", m_Model, false);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    setAttribute(mapInfo, "logical name", m_LogicalName);
}

void DeviceOthers::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Unique ID", m_UniqID);

    m_BusID = mapInfo["SysFS BusID"];
    m_BusID.replace(QRegExp("\\.[0-9]*$"), "");

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            m_BusInfo = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }
}

const QString &DeviceOthers::name()const
{
    return m_Name;
}

const QString &DeviceOthers::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceOthers::driver()const
{
    return m_Driver;
}

const QString &DeviceOthers::logicalName()const
{
    return m_LogicalName;
}

QString DeviceOthers::subTitle()
{
    return m_Model;
}

const QString DeviceOthers::getOverviewInfo()
{
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceOthers::initFilterKey()
{

}

void DeviceOthers::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
}

void DeviceOthers::loadOtherDeviceInfo()
{
    mapInfoToList();
}

void DeviceOthers::loadTableData()
{
    // 加载表格数据
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
