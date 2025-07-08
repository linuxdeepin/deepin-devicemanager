// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceBluetooth.h"
#include "DBusEnableInterface.h"
#include "DDLog.h"

// Qt库文件
#include <QLoggingCategory>

using namespace DDLog;

DeviceBluetooth::DeviceBluetooth()
    : DeviceBaseInfo()
    , m_Model("")
    , m_MAC("")
    , m_LogicalName("")
    , m_BusInfo("")
    , m_Capabilities("")
      //, m_Driver(""), m_Driver("btusb")
    , m_DriverVersion("")
    , m_MaximumPower("")
    , m_Speed("")
{
    qCDebug(appLog) << "DeviceBluetooth constructor initialized";

    // 初始化可显示属性
    initFilterKey();

    // 设置可禁用
    m_CanEnable = true;
}

void DeviceBluetooth::setInfoFromHciconfig(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromHciconfig started.";
    // 获取设备的基本信息
    setAttribute(mapInfo, "Name", m_Name);
    setAttribute(mapInfo, "Alias", m_Alias);
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "HCI Version", m_Version, true);
    qCDebug(appLog) << "Basic attributes set from Hciconfig.";

    // 获取设备其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromHciconfig finished.";
}

bool DeviceBluetooth::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting bluetooth info from hwinfo";

    if (mapInfo.find("path") != mapInfo.end()) {
        qCDebug(appLog) << "Path found in mapInfo, setting basic attributes.";
        setAttribute(mapInfo, "name", m_Name);
        setAttribute(mapInfo, "driver", m_Driver);
        m_SysPath = mapInfo["path"];
        m_HardwareClass = mapInfo["Hardware Class"];
        m_Enable = false;
        m_UniqueID = mapInfo["unique_id"];
        //设备禁用的情况，没必要再继续向下执行，直接return
        m_CanUninstall = !driverIsKernelIn(m_Driver);
        qCDebug(appLog) << "Device disabled or kernel driver, returning true.";
        return true;
    }

    // 设置设备基本属性
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    // 获取设备基本信息
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    // 防止Serial ID为空
    if (m_SerialID.isEmpty()) {
        qCDebug(appLog) << "Serial ID is empty, setting from Unique ID.";
        m_SerialID = m_UniqueID;
    }
    qCDebug(appLog) << "Basic device attributes set from Hwinfo.";

    m_HardwareClass = "bluetooth";

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    // 判断是否核内驱动
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "Driver is kernel in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }

    // 设置关联到lshw信息的key值,设备的唯一标志
    setHwinfoLshwKey(mapInfo);

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromHwinfo finished.";
    return true;
}

bool DeviceBluetooth::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting bluetooth info from lshw";

    // 根据 总线信息 与 设备信息中的唯一key值 判断是否是同一台设备
    if (!matchToLshw(mapInfo)) {
        qCDebug(appLog) << "Device not matched in lshw info, returning false.";
        return false;
    }

    // 获取基本信息
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "product", m_LogicalName);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    qCDebug(appLog) << "Basic attributes set from Lshw.";
    // 判断是否核内驱动
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "Driver is kernel in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromLshw finished.";
    return true;
}

TomlFixMethod DeviceBluetooth::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromTomlOneByOne started.";
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Name", m_Name);
    ret = setTomlAttribute(mapInfo, "Vendor", m_Vendor);
    ret = setTomlAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "Basic toml attributes set.";

    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Maximum Power", m_MaximumPower);
    ret = setTomlAttribute(mapInfo, "Driver Version", m_DriverVersion);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Logical Name", m_LogicalName);
    ret = setTomlAttribute(mapInfo, "MAC Address", m_MAC);
    qCDebug(appLog) << "Other toml attributes set.";
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromTomlOneByOne finished, return: " << ret;
    return ret;
}

bool DeviceBluetooth::setInfoFromWifiInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBluetooth::setInfoFromWifiInfo started.";
    // 机器自身蓝牙
    const QList<QPair<QString, QString> > &otherAttribs = getOtherAttribs();
    QMap<QString, QString> tmpMaps;
    for (QPair<QString, QString> attrib : otherAttribs) {
        tmpMaps[attrib.first] = attrib.second;
    }

    if ("UART" == tmpMaps[QObject::tr("Bus")]) {//内置：UART 外接USB：USB
        qCDebug(appLog) << "Bus is UART, setting chip type and vendor.";
        setAttribute(mapInfo, "Chip Type", m_Name);
        setAttribute(mapInfo, "Vendor", m_Vendor);
        qCDebug(appLog) << "DeviceBluetooth::setInfoFromWifiInfo finished, returning true.";
        return true;
    } else {
        qCDebug(appLog) << "Bus is not UART, returning false.";
        return false;
    }
}

const QString &DeviceBluetooth::name()const
{
    // qCDebug(appLog) << "DeviceBluetooth::name called, returning: " << m_Model;
    return m_Model;
}

const QString &DeviceBluetooth::vendor() const
{
    // qCDebug(appLog) << "DeviceBluetooth::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceBluetooth::driver()const
{
    // qCDebug(appLog) << "DeviceBluetooth::driver called, returning: " << m_Driver;
    return m_Driver;
}

QString DeviceBluetooth::subTitle()
{
    // qCDebug(appLog) << "DeviceBluetooth::subTitle called, returning: " << m_Name;
    return m_Name;
}

const QString DeviceBluetooth::getOverviewInfo()
{
    qCDebug(appLog) << "Getting bluetooth overview info";

    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DeviceBluetooth::setEnable(bool e)
{
    qCDebug(appLog) << "DeviceBluetooth::setEnable called with status: " << e;
    if (m_SerialID.isEmpty()) {
        qCWarning(appLog) << "SerialID is empty, returning EDS_NoSerial.";
        return EDS_NoSerial;
    }

    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        qCWarning(appLog) << "UniqueID or SysPath empty, returning EDS_Faild.";
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_SysPath, m_UniqueID, e, m_Driver);
    if (res) {
        qCDebug(appLog) << "DBus enable call successful, setting m_Enable to " << e;
        m_Enable = e;
    }
    // 设置设备状态
    qCDebug(appLog) << "Set enable status:" << e << ", result:" << res;
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceBluetooth::enable()
{
    // qCDebug(appLog) << "DeviceBluetooth::enable called, returning current enable status: " << m_Enable;
    // 获取设备状态
    return m_Enable;
}

void DeviceBluetooth::initFilterKey()
{
    qCDebug(appLog) << "DeviceBluetooth::initFilterKey called.";
    // 添加可显示的属性
    addFilterKey(QObject::tr("Bus"));
    addFilterKey(QObject::tr("BD Address"));
    addFilterKey(QObject::tr("ACL MTU"));
    addFilterKey(QObject::tr("SCO MTU"));
    addFilterKey(QObject::tr("Features"));
    addFilterKey(QObject::tr("Packet type"));
    addFilterKey(QObject::tr("Link policy"));
    addFilterKey(QObject::tr("Link mode"));
    addFilterKey(QObject::tr("Class"));
    addFilterKey(QObject::tr("Service Classes"));
    addFilterKey(QObject::tr("Device Class"));
    addFilterKey(QObject::tr("HCI Version"));
    addFilterKey(QObject::tr("LMP Version"));
    addFilterKey(QObject::tr("Subversion"));

    addFilterKey(QObject::tr("Device"));
    addFilterKey(QObject::tr("Serial ID"));

    addFilterKey(QObject::tr("product"));
    addFilterKey(QObject::tr("description"));
    // addFilterKey(QObject::tr("physical id"));
    addFilterKey(QObject::tr("Class"));
    addFilterKey(QObject::tr("Powered"));
    addFilterKey(QObject::tr("Discoverable"));
    addFilterKey(QObject::tr("Pairable"));
    addFilterKey(QObject::tr("UUID"));
    addFilterKey(QObject::tr("Modalias"));
    addFilterKey(QObject::tr("Discovering"));
    qCDebug(appLog) << "Filter keys initialized.";
}

void DeviceBluetooth::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceBluetooth::loadBaseDeviceInfo called.";
    // 添加基本信息
    addBaseDeviceInfo(tr("Alias"), m_Alias);
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
}

void DeviceBluetooth::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceBluetooth::loadOtherDeviceInfo called.";
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver Version"), m_DriverVersion);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    addOtherDeviceInfo(tr("Bus Info"), m_BusInfo);
    addOtherDeviceInfo(tr("Logical Name"), m_LogicalName);
    addOtherDeviceInfo(tr("MAC Address"), m_MAC);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceBluetooth::loadTableData()
{
    qCDebug(appLog) << "Loading table data";
    // 加载表格数据
    QString tName = m_Name;

    if (!available()) {
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
