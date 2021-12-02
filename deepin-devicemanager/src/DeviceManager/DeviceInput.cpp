// 项目自身文件
#include "DeviceInput.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"

// Qt库文件
#include <QDebug>

DeviceInput::DeviceInput()
    : DeviceBaseInfo()
    , m_Name("")
    , m_Vendor("")
    , m_Model("")
    , m_Version("")
    , m_Interface("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_Description("")
    , m_Driver("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_KeyToLshw("")
    , m_SerialID("")
{
    initFilterKey();
    m_CanEnable = true;
    m_CanUninstall = true;
}

bool DeviceInput::setInfoFromlshw(const QMap<QString, QString> &mapInfo)
{
    // 根据bus info属性值与m_KeyToLshw对比,判断是否为同一设备
    if (!matchToLshw(mapInfo)) {
            return false;
    }

    // 设置基础设备信息
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "", m_Interface);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "description", m_Description);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    if(driverIsKernelIn(m_Driver)){
        m_CanUninstall = false;
    }


    // 获取其他设备信息
    getOtherMapInfo(mapInfo);

    return true;
}

void DeviceInput::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if(mapInfo.find("Enable") != mapInfo.end()){
        m_Enable = false;
    }
    // 设置设备基本属性
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "name", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Serial ID", m_UniqueID);

    // 获取键盘的接口类型
    if (mapInfo.find("Hotplug") != mapInfo.end())
        setAttribute(mapInfo, "Hotplug", m_Interface);
    else
        m_Interface = "PS/2";

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model") != mapInfo.end() && mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive))
        m_Interface = "Bluetooth";

    if (mapInfo.find("Device") != mapInfo.end() && mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive))
        m_Interface = "Bluetooth";

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    if(driverIsKernelIn(m_Driver)){
        m_CanUninstall = false;
    }

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

void DeviceInput::setKLUInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    if (mapInfo.find("Hotplug") != mapInfo.end())
        setAttribute(mapInfo, "Hotplug", m_Interface);
    else
        m_Interface = "PS/2";

    // 上面的方法不适合蓝牙键盘的获取方法
    if (mapInfo.find("Model")->contains("Bluetooth", Qt::CaseInsensitive) || mapInfo.find("Device")->contains("Bluetooth", Qt::CaseInsensitive))
        m_Interface = "Bluetooth";

    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Hardware Class", m_Description);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    QStringList words = mapInfo["SysFS BusID"].split(":");
    if (words.size() == 2) {
        QStringList chs = words[0].split("-");
        if (chs.size() == 2)
            m_KeyToLshw = QString("usb@%1:%2").arg(chs[0]).arg(chs[1]);
    }

    // 由bluetoothctl paired-devices设置设备接口
    setInfoFromBluetoothctl();

    // 获取其他设备信息
    getOtherMapInfo(mapInfo);
}

void DeviceInput::setInfoFromBluetoothctl()
{
    // 判断该设备信息是否存在于Bluetoothctl中
    if (isValueValid(m_keysToPairedDevice)) {
        bool isExist = DeviceManager::instance()->isDeviceExistInPairedDevice(m_keysToPairedDevice.toUpper());

        if (isExist)
            m_Interface = "Bluetooth";
    }
}


const QString &DeviceInput::name() const
{
    return m_Name;
}

const QString &DeviceInput::driver() const
{
    return m_Driver;
}

bool DeviceInput::available()
{
    if(driver().isEmpty()){
        m_Available = false;
    }
    if("PS/2" == m_Interface){
        m_Available = true;
    }
    return m_Available;
}

QString DeviceInput::subTitle()
{
    // 获取子标题
    if (!m_Name.isEmpty())
        return m_Name;
    return m_Model;
}

const QString DeviceInput::getOverviewInfo()
{
    // 获取概况信息
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

EnableDeviceStatus DeviceInput::setEnable(bool e)
{
    if(m_SerialID.isEmpty()){
        return EDS_NoSerial;
    }

    if(m_UniqueID.isEmpty() || m_SysPath.isEmpty()){
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass,m_Name,m_SysPath,m_UniqueID,e);
    if(res){
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceInput::enable()
{
    // 键盘不可禁用
    if(m_HardwareClass == "keyboard"){
        m_Enable = true;
    }
    return m_Enable;
}

void DeviceInput::initFilterKey()
{
    // 添加可显示的设备信息
    addFilterKey(QObject::tr("Uniq"));
    addFilterKey(QObject::tr("PROP"));
    addFilterKey(QObject::tr("EV"));
    addFilterKey(QObject::tr("KEY"));
    addFilterKey(QObject::tr("MSC"));
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));

    addFilterKey(QObject::tr("physical id"));
}

void DeviceInput::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Interface"), m_Interface);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
}

void DeviceInput::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Current"), m_MaximumPower);   // 1050需求将最大功率改为最大电流
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Version"), m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceInput::loadTableData()
{
    // 加载表格数据
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

