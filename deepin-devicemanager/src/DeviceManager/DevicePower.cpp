// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DevicePower.h"
#include "commonfunction.h"

// Qt库文件
#include<QFileInfo>

// Dtk头文件
#include <DApplication>

DWIDGET_USE_NAMESPACE

DevicePower::DevicePower()
    : DeviceBaseInfo()
    , m_Model("")
    , m_Type("")
    , m_SerialNumber("")
    , m_ElectricType("")
    , m_MaxPower("")
    , m_Status("")
    , m_Enabled("")
    , m_HotSwitch("")
    , m_Capacity("")
    , m_Voltage("")
    , m_Slot("")
    , m_DesignCapacity("")
    , m_DesignVoltage("")
    , m_SBDSChemistry("")
    , m_SBDSManufactureDate("")
    , m_SBDSSerialNumber("")
    , m_SBDSVersion("")
    , m_Temp("")

{
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DevicePower::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息    
    setTomlAttribute(mapInfo, "Model", m_Model);
    setTomlAttribute(mapInfo, "Serial Number", m_SerialNumber);
    setTomlAttribute(mapInfo, "Type", m_Type);
    setTomlAttribute(mapInfo, "Status", m_Status);
    setTomlAttribute(mapInfo, "Capacity", m_Capacity);
    setTomlAttribute(mapInfo, "Voltage", m_Voltage);
    setTomlAttribute(mapInfo, "Slot", m_Slot);
    setTomlAttribute(mapInfo, "Design Capacity", m_DesignCapacity);
    setTomlAttribute(mapInfo, "Design Voltage", m_DesignVoltage);
    setTomlAttribute(mapInfo, "SBDS Version", m_SBDSVersion);
    setTomlAttribute(mapInfo, "SBDS Serial Number", m_SBDSSerialNumber);
    setTomlAttribute(mapInfo, "SBDS Manufacture Date", m_SBDSManufactureDate);
    setTomlAttribute(mapInfo, "SBDS Chemistry", m_SBDSChemistry);
    ret = setTomlAttribute(mapInfo, "Temperature", m_Temp);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

bool DevicePower::setInfoFromUpower(const QMap<QString, QString> &mapInfo)
{
    // 设置upower中获取的信息
    if (mapInfo["Device"].contains("line_power", Qt::CaseInsensitive) ||
        mapInfo["state"].contains("empty", Qt::CaseInsensitive) ||
        mapInfo["state"].contains("unknown", Qt::CaseInsensitive)) {
        return false;
    }

    setAttribute(mapInfo, "", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "serial", m_SerialNumber);
    setAttribute(mapInfo, "", m_ElectricType);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Status);
    setAttribute(mapInfo, "", m_Enabled);
    setAttribute(mapInfo, "", m_HotSwitch);
    setAttribute(mapInfo, "capacity", m_Capacity);
    setAttribute(mapInfo, "voltage", m_Voltage);
    setAttribute(mapInfo, "", m_Slot);
    setAttribute(mapInfo, "", m_SBDSChemistry);
    setAttribute(mapInfo, "", m_SBDSManufactureDate);
    setAttribute(mapInfo, "", m_SBDSSerialNumber);
    setAttribute(mapInfo, "", m_SBDSVersion);

    // 添加电池温度
    setAttribute(mapInfo, "temperature", m_Temp);
    /*
     * 温度底层的获取方式是/sys/class/power_supply/BAT0下面的temp文件
     * 以前向temp文件写温度时如果是28度 会写2.8(需要做*10的处理)  但是现在会直接写28
     * 处理方法：取消*10的操作
     * if (!m_Temp.isEmpty()) {
        double temp = m_Temp.replace("degrees C", "").trimmed().toDouble();
        temp = temp * 10;
        m_Temp = QString("%1 degrees C").arg(temp);
    }*/

    getOtherMapInfo(mapInfo);
    return true;
}

void DevicePower::setDaemonInfo(const QMap<QString, QString> &mapInfo)
{
    // 设置守护进程信息
    if (m_Name == translateStr("battery"))
        getOtherMapInfo(mapInfo);
}

const QString &DevicePower::name()const
{
    return m_Name;
}

const QString &DevicePower::vendor() const
{
    return m_Vendor;
}

const QString &DevicePower::driver() const
{
    return m_Driver;
}

bool DevicePower::available()
{
    return true;
}

QString DevicePower::subTitle()
{
    return m_Name;
}

const QString DevicePower::getOverviewInfo()
{
    // 获取概况信息
    return DApplication::translate("ManulTrack", m_Name.trimmed().toStdString().data(), "");
}

void DevicePower::initFilterKey()
{
    // 初始化可显示属性
//    addFilterKey("native-path");
    addFilterKey("power supply");
//    addFilterKey("updated");
    addFilterKey("has history");
    addFilterKey("has statistics");
    addFilterKey("rechargeable");
    addFilterKey("state");
    addFilterKey("warning-level");
    addFilterKey("energy");
    addFilterKey("energy-empty");

    QString type = Common::specialVendorType();
    if (type != Common::specialHString()) {
        addFilterKey("energy-full");
        addFilterKey("energy-full-design");
    }
    addFilterKey("energy-rate");
    // addFilterKey("voltage");
    addFilterKey("percentage");
//    addFilterKey("temperature"));    // 温度已经常规显示-2
    addFilterKey("technology");
//    addFilterKey("icon-name");
    addFilterKey("online");
    addFilterKey("daemon-version");
    addFilterKey("on-battery");
    addFilterKey("lid-is-closed");
    addFilterKey("lid-is-present");
    addFilterKey("critical-action");
}

void DevicePower::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(("Name"), m_Name);
    addBaseDeviceInfo(("Model"), m_Model);
    addBaseDeviceInfo(("Vendor"), m_Vendor);
    addBaseDeviceInfo(("Serial Number"), m_SerialNumber);
    addBaseDeviceInfo(("Type"), m_Type);
    addBaseDeviceInfo(("Status"), m_Status);
    addBaseDeviceInfo(("Capacity"), m_Capacity);
    addBaseDeviceInfo(("Voltage"), m_Voltage);
    addBaseDeviceInfo(("Slot"), m_Slot);
    addBaseDeviceInfo(("Design Capacity"), m_DesignCapacity);
    addBaseDeviceInfo(("Design Voltage"), m_DesignVoltage);
    addBaseDeviceInfo(("SBDS Version"), m_SBDSVersion);
    addBaseDeviceInfo(("SBDS Serial Number"), m_SBDSSerialNumber);
    addBaseDeviceInfo(("SBDS Manufacture Date"), m_SBDSManufactureDate);
    addBaseDeviceInfo(("SBDS Chemistry"), m_SBDSChemistry);
    addBaseDeviceInfo(("Temperature"), m_Temp.replace("degrees C", "℃", Qt::CaseInsensitive));
}

void DevicePower::loadOtherDeviceInfo()
{
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DevicePower::loadTableData()
{
    // 加载表格信息
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
