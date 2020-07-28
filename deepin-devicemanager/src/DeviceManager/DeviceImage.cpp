#include "DeviceImage.h"

DeviceImage::DeviceImage()
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
{

}

void DeviceImage::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (m_KeyToLshw != mapInfo["bus info"]) {
        return;
    }
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
}

void DeviceImage::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "", m_Capabilities);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "", m_MaximumPower);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2) {
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
        }
    }
}

const QString &DeviceImage::name()const
{
    return m_Name;
}

const QString &DeviceImage::vendor()const
{
    return m_Vendor;
}

const QString &DeviceImage::model()const
{
    return m_Model;
}

const QString &DeviceImage::version()const
{
    return m_Version;
}

const QString &DeviceImage::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceImage::capabilities()const
{
    return m_Capabilities;
}

const QString &DeviceImage::driver()const
{
    return m_Driver;
}

const QString &DeviceImage::maxinumPower()const
{
    return m_MaximumPower;
}

const QString &DeviceImage::speed()const
{
    return m_Speed;
}

const QString &DeviceImage::keyToLshw()const
{
    return m_KeyToLshw;
}

QString DeviceImage::subTitle()
{
    return m_Name;
}

const QString DeviceImage::getOverviewInfo()
{
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceImage::initFilterKey()
{

}

void DeviceImage::loadBaseDeviceInfo()
{
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);

//    m_SubTitle = m_Name;
}

void DeviceImage::loadOtherDeviceInfo()
{
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);

    mapInfoToList();
}

void DeviceImage::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
