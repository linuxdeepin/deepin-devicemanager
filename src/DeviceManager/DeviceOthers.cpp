#include "DeviceOthers.h"
#include "deviceinfoparser.h"

DeviceOthers::DeviceOthers()
    : DeviceBaseInfo (), m_Name(""), m_Vendor(""), m_Model(""), m_Version("")
    , m_BusInfo(""), m_Capabilities(""), m_Driver(""), m_MaximumPower(""), m_Speed(""), m_LogicalName("")
{

}

void DeviceOthers::setInfoFromLshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfo(mapInfo, info);

    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "product", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    setAttribute(mapInfo, "logical name", m_LogicalName);
}

void DeviceOthers::setInfoFromHwinfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromHwinfo(mapInfo, info);

    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Unique ID", m_UniqID);

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

bool DeviceOthers::isExist()
{
    if (DeviceInfoParser::Instance().s_usbDeiveUniq.contains(m_UniqID)) {
        return true;
    }

    if (m_UniqID.isEmpty()) {
        return true;
    }

    return false;

}

const QString &DeviceOthers::name()const
{
    return m_Name;
}
const QString &DeviceOthers::vendor()const
{
    return m_Vendor;
}
const QString &DeviceOthers::model()const
{
    return m_Model;
}
const QString &DeviceOthers::version()const
{
    return m_Version;
}
const QString &DeviceOthers::busInfo()const
{
    return m_BusInfo;
}
const QString &DeviceOthers::capabilities()const
{
    return m_Capabilities;
}
const QString &DeviceOthers::driver()const
{
    return m_Driver;
}
const QString &DeviceOthers::maxinumPower()const
{
    return m_MaximumPower;
}
const QString &DeviceOthers::speed()const
{
    return m_Speed;
}

const QString &DeviceOthers::logicalName()const
{
    return m_LogicalName;
}
void DeviceOthers::initFilterKey()
{

}
