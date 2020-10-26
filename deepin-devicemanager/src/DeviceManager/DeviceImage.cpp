// 项目自身文件
#include "DeviceImage.h"

// 其它头文件
#include "DeviceManager.h"
#include "EnableManager.h"

DeviceImage::DeviceImage()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Driver("uvcvideo")
    , m_MaximumPower("")
    , m_Speed("")
{
    m_CanEnable = true;
}

void DeviceImage::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (m_KeyToLshw != mapInfo["bus info"]) {
        return;
    }

    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver, false);
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
    setAttribute(mapInfo, "Driver", m_Driver, true);//
    setAttribute(mapInfo, "Driver Modules", m_Driver, true);
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

const QString &DeviceImage::driver()const
{
    return m_Driver;
}

QString DeviceImage::subTitle()
{
    return m_Name;
}

const QString DeviceImage::getOverviewInfo()
{
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

EnableDeviceStatus DeviceImage::setEnable(bool e)
{
    // 设置设备状态
    EnableDeviceStatus res = EnableManager::instance()->enableDeviceByDriver(e, m_Driver);
    if (e != enable()) {
        res = EDS_Faild;
    }

    return res;
}

bool DeviceImage::enable()
{
    // 获取设备状态
    m_Enable = EnableManager::instance()->isDeviceEnableByDriver(m_Driver);
    return m_Enable;
}

void DeviceImage::initFilterKey()
{

}

void DeviceImage::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
}

void DeviceImage::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceImage::loadTableData()
{
    // 记载表格内容
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

void DeviceImage::setInfoFromInput()
{
    // 设置设备名称
    const QMap<QString, QString> &mapInfo = DeviceManager::instance()->inputInfo(m_KeysToCatDevices);
    setAttribute(mapInfo, "Name", m_Name, true);

    // 设置是否可禁用
    m_Enable = EnableManager::instance()->isDeviceEnable(m_Name);
}
