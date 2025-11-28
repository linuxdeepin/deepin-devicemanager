// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceImage.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"
#include "DBusInterface.h"
#include "DDLog.h"

using namespace DDLog;

DeviceImage::DeviceImage()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_MaximumPower("")
    , m_Speed("")
{
    qCDebug(appLog) << "DeviceImage constructor initialized.";
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceImage::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromLshw";
    if (!matchToLshw(mapInfo)) {
        qCDebug(appLog) << "not match to lshw";
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
    qCDebug(appLog) << "Basic attributes set from Lshw.";
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "Driver is kernel in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }
    qCDebug(appLog) << "DeviceImage::setInfoFromLshw finished.";
}

TomlFixMethod DeviceImage::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromTomlOneByOne";

    TomlFixMethod ret = TOML_None;

    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Version", m_Version);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Maximum Power", m_MaximumPower);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialID);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceImage::setInfoFromTomlOneByOne finished, return: " << ret;
    return ret;
}

void DeviceImage::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "setInfoFromHwinfo";

    if (mapInfo.find("unique_id") != mapInfo.end()) {
        qCDebug(appLog) << "Unique ID found in mapInfo.";
        m_UniqueID = mapInfo["unique_id"];
        m_Name = mapInfo["name"];
        m_SysPath = mapInfo["path"];
        m_HardwareClass = mapInfo["Hardware Class"];
        m_Enable = false;
        setAttribute(mapInfo, "driver", m_Driver);
        //设备禁用的情况，没必要再继续向下执行，直接return
        m_CanUninstall = !driverIsKernelIn(m_Driver);
        qCDebug(appLog) << "info found unique_id:" << m_UniqueID << " name:" << m_Name << " path:" << m_SysPath << " class:" << m_HardwareClass;
        return;
    }
    if (mapInfo.find("Enable") != mapInfo.end()) {
        qCDebug(appLog) << "Enable flag found in mapInfo, setting m_Enable to false.";
        m_Enable = false;
    }
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    // 防止Serial ID为空
    if (m_SerialID.isEmpty()) {
        qCDebug(appLog) << "Serial ID is empty, setting from Unique ID.";
        m_SerialID = m_UniqueID;
    }
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver, true);//
    setAttribute(mapInfo, "Driver Modules", m_Driver, true);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;
    if (driverIsKernelIn(m_Driver)) {
        qCDebug(appLog) << "Driver is kernel in, setting m_CanUninstall to false.";
        m_CanUninstall = false;
    }
    qCDebug(appLog) << "Hwinfo attributes set. Name=" << m_Name << ", Vendor=" << m_Vendor << ", Model=" << m_Model << ", Version=" << m_Version << ", SysPath=" << m_SysPath << ", Driver=" << m_Driver;

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);
    qCDebug(appLog) << "DeviceImage::setInfoFromHwinfo finished.";
}

const QString &DeviceImage::name()const
{
    // qCDebug(appLog) << "DeviceImage::name called, returning: " << m_Name;
    return m_Name;
}

const QString &DeviceImage::vendor() const
{
    // qCDebug(appLog) << "DeviceImage::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceImage::driver()const
{
    // qCDebug(appLog) << "DeviceImage::driver called, returning: " << m_Driver;
    return m_Driver;
}

QString DeviceImage::subTitle()
{
    // qCDebug(appLog) << "DeviceImage::subTitle called, returning: " << m_Name;
    return m_Name;
}

const QString DeviceImage::getOverviewInfo()
{
    // qCDebug(appLog) << "DeviceImage::getOverviewInfo called.";
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);
    // qCDebug(appLog) << "Overview info: " << ov;
    return ov;
}

EnableDeviceStatus DeviceImage::setEnable(bool e)
{
    qCDebug(appLog) << "DeviceImage::setEnable called with status: " << e;
    if (m_SerialID.isEmpty()) {
        qCWarning(appLog) << "SerialID is empty, returning EDS_NoSerial.";
        return EDS_NoSerial;
    }

    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        qCWarning(appLog) << "UniqueID or SysPath empty, returning EDS_Faild.";
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable("camera", m_Name, m_SysPath, m_UniqueID, e, m_Driver);
    if (res) {
        qCDebug(appLog) << "DBus enable call successful, setting m_Enable to " << e;
        m_Enable = e;
        if(e) {
            qCDebug(appLog) << "Enable is true, refreshing info.";
            DBusInterface::getInstance()->refreshInfo();
        }
    } else {
        qCDebug(appLog) << "DBus enable call failed.";
    }
    // 设置设备状态
    qCDebug(appLog) << "Set enable status:" << e << ", result:" << res;
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceImage::enable()
{
    // qCDebug(appLog) << "DeviceImage::enable called, returning current enable status: " << m_Enable;
    // 获取设备状态
    return m_Enable;
}

void DeviceImage::initFilterKey()
{
    // qCDebug(appLog) << "DeviceImage::initFilterKey called. No filter keys added based on existing function body.";

}

void DeviceImage::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceImage::loadBaseDeviceInfo called.";
    // 添加基本信息
    addBaseDeviceInfo("Name", m_Name);
    addBaseDeviceInfo("Vendor", m_Vendor);
    addBaseDeviceInfo("Version", m_Version);
    addBaseDeviceInfo("Model", m_Model);
    addBaseDeviceInfo("Bus Info", m_BusInfo);
    qCDebug(appLog) << "Base device info loaded.";
}

void DeviceImage::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceImage::loadOtherDeviceInfo called.";
    // 添加其他信息,成员变量
    addOtherDeviceInfo("Module Alias", m_Modalias);
    addOtherDeviceInfo("Physical ID", m_PhysID);
    addOtherDeviceInfo("Speed", m_Speed);
    addOtherDeviceInfo("Maximum Power", m_MaximumPower);
    addOtherDeviceInfo("Driver", m_Driver);
    addOtherDeviceInfo("Capabilities", m_Capabilities);
    if (m_SerialID != m_UniqueID) {
        qCDebug(appLog) << "SerialID is different from UniqueID, adding Serial Number.";
        addOtherDeviceInfo("Serial Number", m_SerialID);
    }

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
    qCDebug(appLog) << "Other device info loaded.";
}

void DeviceImage::loadTableData()
{
    qCDebug(appLog) << "DeviceImage::loadTableData called.";
    // 记载表格内容
    QString tName = m_Name;

    if (!available()) {
        qCDebug(appLog) << "Device not available, updating tName.";
        tName = "(" + translateStr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        qCDebug(appLog) << "Device disabled, updating tName.";
        tName = "(" + translateStr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
    qCDebug(appLog) << "Table data loaded.";
}
