// 项目自身文件
#include "DeviceImage.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"

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
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceImage::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (m_KeyToLshw != mapInfo["bus info"])
        return;

    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver, false);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    if(driverIsKernelIn(m_Driver)){
        m_CanUninstall = false;
    }
}

void DeviceImage::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if(mapInfo.find("unique_id") != mapInfo.end()){
        m_UniqueID = mapInfo["unique_id"];
        m_Name = mapInfo["name"];
        m_SysPath =mapInfo["path"];
        m_HardwareClass = mapInfo["Hardware Class"];
        m_Enable = false;
        return;
    }
    if(mapInfo.find("Enable") != mapInfo.end()){
        m_Enable = false;
    }
    setAttribute(mapInfo, "Module Alias", m_UniqueID);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver, true);//
    setAttribute(mapInfo, "Driver Modules", m_Driver, true);
    setAttribute(mapInfo, "Speed", m_Speed);
    if(driverIsKernelIn(m_Driver)){
        m_CanUninstall = false;
    }

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2)
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
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
    if(m_UniqueID.isEmpty() || m_SysPath.isEmpty()){
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable("camera",m_Name,m_SysPath,m_UniqueID,e);
    if(res){
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceImage::enable()
{
    // 获取设备状态
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
    QString tName = m_Name;

    if (!available()){
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }

    if(!enable()){
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}

void DeviceImage::setInfoFromInput()
{
    // 设置设备名称
    const QMap<QString, QString> &mapInfo = DeviceManager::instance()->inputInfo(m_KeysToCatDevices);
    setAttribute(mapInfo, "Name", m_Name, true);
}
