// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceOthers.h"
#include "DBusEnableInterface.h"
#include "DDLog.h"
#include <QRegularExpression>

using namespace DDLog;

DeviceOthers::DeviceOthers()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_LogicalName("")
{
    qCDebug(appLog) << "DeviceOthers::DeviceOthers()";
    initFilterKey();
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceOthers::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceOthers::setInfoFromLshw";
    if (!matchToLshw(mapInfo)) {
        qCDebug(appLog) << "DeviceOthers::setInfoFromLshw, not match to lshw";
        return;
    }
    QString            tmp_Name = m_Name;
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

    if(m_Driver.isEmpty() && !m_Avail.compare("yes", Qt::CaseInsensitive)){
        qCDebug(appLog) << "DeviceOthers::setInfoFromLshw, driver is empty and avail is yes";
        setForcedDisplay(true);
        setCanEnale(false);
        if(!(tmp_Name.contains("fingerprint", Qt::CaseInsensitive) || tmp_Name. contains("MOH", Qt::CaseInsensitive)))
            setCanUninstall(false);
    }

    // 核内驱动不显示卸载菜单
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "DeviceOthers::setInfoFromLshw, driver is kernel in";
        m_CanUninstall = false;
    }
}

TomlFixMethod DeviceOthers::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceOthers::setInfoFromTomlOneByOne";
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Maximum Power", m_MaximumPower);
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
////Others
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialID);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceOthers::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo";
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    /* 禁用时提示获取序列号失败*/
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    // 防止Serial ID为空
    if (m_SerialID.isEmpty()) {
        qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo, serial id is empty";
        m_SerialID = m_UniqueID;
    }

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    if (mapInfo["Hardware Class"] != "fingerprint") {
        qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo, hardware class is not fingerprint";
        m_HardwareClass = "others";
    } else {
        qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo, hardware class is fingerprint";
        // 机器自带指纹模块不支持禁用,和卸载驱动
        setAttribute(mapInfo, "Hardware Class", m_HardwareClass);
        m_CanEnable = false;
        m_CanUninstall = false;
    }
    setAttribute(mapInfo, "cfg_avail", m_Avail);
    if(!m_Avail.compare("yes", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo, avail is yes";
        setForcedDisplay(true);
    }

    getOtherMapInfo(mapInfo);
    // 核内驱动不显示卸载菜单
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "DeviceOthers::setInfoFromHwinfo, driver is kernel in";
        m_CanUninstall = false;
    }

    m_BusID = mapInfo["SysFS BusID"];
    m_BusID.replace(QRegularExpression("\\.\\d+$"), "");

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);
}

EnableDeviceStatus DeviceOthers::setEnable(bool e)
{
    qCDebug(appLog) << "DeviceOthers::setEnable, enable:" << e;
    if (m_SerialID.isEmpty()) {
        qCDebug(appLog) << "DeviceOthers::setEnable, serial id is empty";
        return EDS_NoSerial;
    }

    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        qCDebug(appLog) << "DeviceOthers::setEnable, unique id or sys path is empty";
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_SysPath, m_UniqueID, e);
    if (res) {
        qCDebug(appLog) << "DeviceOthers::setEnable, enable success";
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

const QString &DeviceOthers::name()const
{
    // qCDebug(appLog) << "DeviceOthers::name";
    return m_Name;
}

const QString &DeviceOthers::vendor() const
{
    // qCDebug(appLog) << "DeviceOthers::vendor";
    return m_Vendor;
}

const QString &DeviceOthers::busInfo()const
{
    // qCDebug(appLog) << "DeviceOthers::busInfo";
    return m_BusInfo;
}

const QString &DeviceOthers::driver()const
{
    // qCDebug(appLog) << "DeviceOthers::driver";
    return m_Driver;
}

const QString &DeviceOthers::logicalName()const
{
    // qCDebug(appLog) << "DeviceOthers::logicalName";
    return m_LogicalName;
}

QString DeviceOthers::subTitle()
{
    // qCDebug(appLog) << "DeviceOthers::subTitle";
    return m_Model;
}

const QString DeviceOthers::getOverviewInfo()
{
    // qCDebug(appLog) << "DeviceOthers::getOverviewInfo";
    return m_Name.isEmpty() ? m_Model : m_Name;
}

bool DeviceOthers::available()
{
    qCDebug(appLog) << "DeviceOthers::available";
    if (driver().isEmpty() && m_HardwareClass == "others") {
        qCDebug(appLog) << "DeviceOthers::available, driver is empty and hardware class is others";
        m_Available = false;
    }
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

void DeviceOthers::initFilterKey()
{
    // qCDebug(appLog) << "DeviceOthers::initFilterKey";
    addFilterKey("Device File");
    addFilterKey("Hardware Class");
}

void DeviceOthers::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceOthers::loadBaseDeviceInfo";
    // 添加基本信息
    addBaseDeviceInfo("Name", m_Name);
    addBaseDeviceInfo("Vendor", m_Vendor);
    addBaseDeviceInfo("Model", m_Model);
    addBaseDeviceInfo("Version", m_Version);
    addBaseDeviceInfo("Bus Info", m_BusInfo);
    addBaseDeviceInfo("Capabilities", m_Capabilities);
    addBaseDeviceInfo("Driver", m_Driver);
    addBaseDeviceInfo("Maximum Power", m_MaximumPower);
    addBaseDeviceInfo("Speed", m_Speed);
}

void DeviceOthers::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceOthers::loadOtherDeviceInfo";
    if (m_SerialID != m_UniqueID)
        addOtherDeviceInfo("Serial Number", m_SerialID);
    mapInfoToList();
}

void DeviceOthers::loadTableData()
{
    qCDebug(appLog) << "DeviceOthers::loadTableData";
    // 加载表格数据
    QString tName = m_Name;
    if (!available()) {
        qCDebug(appLog) << "DeviceOthers::loadTableData, not available";
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }
    if (!enable()) {
        qCDebug(appLog) << "DeviceOthers::loadTableData, not enable";
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
