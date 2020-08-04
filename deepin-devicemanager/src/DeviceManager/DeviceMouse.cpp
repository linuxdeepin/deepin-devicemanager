#include "DeviceMouse.h"
#include <QDebug>
//*******************************************************************************************************************************


DeviceMouse::DeviceMouse()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Interface("")
    , m_BusInfo("")
    , m_Version("")
    , m_Capabilities("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeysToCatDevices("")
    , m_KeyToLshw("")
{
    initFilterKey();
}

void DeviceMouse::setInfoFromHwinfo(QMap<QString, QString> mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);

    // 获取鼠标的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end()) {
        setAttribute(mapInfo, "Hotplug", m_Interface);
    } else {
        m_Interface = "PS/2";
    }

    // 上面的方法不适合蓝牙鼠标的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive)) {
        m_Interface = "Bluetooth";
    }

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver);
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

    getOtherMapInfo(mapInfo);
}

bool DeviceMouse::setInfoFromlshw(QMap<QString, QString> mapInfo)
{
    QString key = mapInfo["bus info"];
    key.replace("a", "10");
    if (m_KeyToLshw != key) {
        return false;
    }

    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_Interface);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    getOtherMapInfo(mapInfo);
    return true;
}

const QString &DeviceMouse::name()const
{
    return m_Name;
}

const QString &DeviceMouse::vendor()const
{
    return m_Vendor;
}

const QString &DeviceMouse::model()const
{
    return m_Model;
}

const QString &DeviceMouse::interface()const
{
    return m_Interface;
}

const QString &DeviceMouse::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceMouse::version()const
{
    return m_Version;
}

const QString &DeviceMouse::capability()const
{
    return m_Capabilities;
}

const QString &DeviceMouse::driver()const
{
    return m_Driver;
}

const QString &DeviceMouse::maxPower()const
{
    return m_MaximumPower;
}

const QString &DeviceMouse::speed()const
{
    return m_Speed;
}

QString DeviceMouse::subTitle()
{
    return m_Name;
}

const QString DeviceMouse::getOverviewInfo()
{
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

void DeviceMouse::initFilterKey()
{
    // from cat devices
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    addFilterKey(QObject::tr("physical id"));

}

void DeviceMouse::loadBaseDeviceInfo()
{
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Interface"), m_Interface);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);

//    m_SubTitle = m_Name;
}

void DeviceMouse::loadOtherDeviceInfo()
{
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Version"), m_Version);

    mapInfoToList();
}

void DeviceMouse::loadTableData()
{
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
