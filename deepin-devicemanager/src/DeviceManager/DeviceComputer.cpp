// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceComputer.h"
#include "DDLog.h"

// Qt库文件
#include <QMap>
#include <QLoggingCategory>

using namespace DDLog;

DeviceComputer::DeviceComputer()
    : m_HomeUrl("")
    , m_OsDescription("")
    , m_OS("")
    , m_Type("")
    //, m_Driver("")
{
    qCDebug(appLog) << "DeviceComputer constructor initialized";
}

TomlFixMethod DeviceComputer::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting computer info from toml";

        TomlFixMethod ret = TOML_None;
//  must cover the  loadOtherDeviceInfo
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "HOME_URL", m_HomeUrl);
    ret = setTomlAttribute(mapInfo, "Type", m_Type);
    ret = setTomlAttribute(mapInfo, "vendor", m_Vendor);
    ret = setTomlAttribute(mapInfo, "OS", m_OS);
    ret = setTomlAttribute(mapInfo, "OsDescription", m_OsDescription);
    qCDebug(appLog) << "Toml attributes set: HomeUrl=" << m_HomeUrl << ", Type=" << m_Type << ", Vendor=" << m_Vendor << ", OS=" << m_OS << ", OsDescription=" << m_OsDescription;
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceComputer::setInfoFromTomlOneByOne finished, return: " << ret;
    return ret;
}

const QString &DeviceComputer::name() const
{
    // qCDebug(appLog) << "DeviceComputer::name called, returning: " << m_Name;
    return m_Name;
}

const QString &DeviceComputer::vendor() const
{
    // qCDebug(appLog) << "DeviceComputer::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceComputer::driver() const
{
    // qCDebug(appLog) << "DeviceComputer::driver called, returning: " << m_Driver;
    return m_Driver;
}

void DeviceComputer::setHomeUrl(const QString &value)
{
    // qCDebug(appLog) << "DeviceComputer::setHomeUrl called with value: " << value;
    // 设置主页网站
    m_HomeUrl = value;
}

void DeviceComputer::setOsDescription(const QString &value)
{
    // qCDebug(appLog) << "DeviceComputer::setOsDescription called with value: " << value;
    // 设置操作系统描述
    m_OsDescription = value;
}

void DeviceComputer::setOS(const QString &value)
{
    // qCDebug(appLog) << "DeviceComputer::setOS called with value: " << value;
    // 设置操作系统
    m_OS = value;
}

void DeviceComputer::setVendor(const QString &value)
{
    // qCDebug(appLog) << "DeviceComputer::setVendor called with value: " << value;
    // 设置制造商
    m_Vendor = value;
}

void DeviceComputer::setName(const QString &value)
{
    qCDebug(appLog) << "DeviceComputer::setName called with value: " << value;
    // 设置计算机名称
    m_Name = value;
    if (m_Name.contains("None", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "Name contains 'None', setting to empty.";
        m_Name = "";
    }
    qCDebug(appLog) << "Name set to: " << m_Name;
}

void DeviceComputer::setType(const QString &value)
{
    // qCDebug(appLog) << "DeviceComputer::setType called with value: " << value;
    // 设置设备类型
    m_Type = value;
}

void DeviceComputer::setVendor(const QString &dm1Vendor, const QString &dm2Vendor)
{
    qCDebug(appLog) << "DeviceComputer::setVendor (overload) called with dm1Vendor: " << dm1Vendor << ", dm2Vendor: " << dm2Vendor;
    // 设置制造商
    if (dm1Vendor.contains("System manufacturer")) {
        qCDebug(appLog) << "dm1Vendor contains 'System manufacturer', setting vendor to dm2Vendor: " << dm2Vendor;
        m_Vendor = dm2Vendor;
    } else {
        qCDebug(appLog) << "dm1Vendor does not contain 'System manufacturer', setting vendor to dm1Vendor: " << dm1Vendor;
        m_Vendor = dm1Vendor;
    }
    qCDebug(appLog) << "Final vendor set to: " << m_Vendor;
}

void DeviceComputer::setName(const QString &dm1Name, const QString &dm2Name, const QString &dm1Family, const QString &dm1Version)
{
    qCDebug(appLog) << "DeviceComputer::setName (overload) called with dm1Name: " << dm1Name << ", dm2Name: " << dm2Name << ", dm1Family: " << dm1Family << ", dm1Version: " << dm1Version;
    // name
    QString pname;
    if (dm1Name.contains("System Product Name")) {
        qCDebug(appLog) << "dm1Name contains 'System Product Name', setting pname to dm2Name: " << dm2Name;
        pname = dm2Name;
    } else {
        qCDebug(appLog) << "dm1Name does not contain 'System Product Name', setting pname to dm1Name: " << dm1Name;
        pname = dm1Name;
    }

    // family
    QString family = dm1Family;
    if (dm1Family.contains("unknown", Qt::CaseInsensitive) \
            || dm1Family.contains("System Version") \
            || dm1Family.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Family.contains("x.x", Qt::CaseInsensitive) \
            || dm1Family.contains("Not Applicable", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "dm1Family matches exclusion criteria, setting family to empty.";
        family = "";
    }

    // version
    QString version = dm1Version;
    if (dm1Version.contains("unknown", Qt::CaseInsensitive) \
            || dm1Version.contains("System Version") \
            || dm1Version.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Version.contains("x.x", Qt::CaseInsensitive) \
            || dm1Version.contains("Not Applicable", Qt::CaseInsensitive)) {
        qCDebug(appLog) << "dm1Version matches exclusion criteria, setting version to empty.";
        version = "";
    }

    // 去除 name,vendor,family中的重复字段.再拼接
    pname = pname.remove(m_Vendor, Qt::CaseInsensitive);
    pname = pname.remove(version, Qt::CaseInsensitive);
    version = version.remove(m_Vendor, Qt::CaseInsensitive);
    version = version.remove(family, Qt::CaseInsensitive);
    family = family.remove(m_Vendor, Qt::CaseInsensitive);
    family = family.remove(version, Qt::CaseInsensitive);

    m_Name = family + " " + version + " " + pname;
    qCDebug(appLog) << "Final name set to: " << m_Name;
}

const QString DeviceComputer::getOverviewInfo()
{
    // 获取概况信息
    qCDebug(appLog) << "Getting computer overview info";
    QString model;
    model += m_Vendor + QString(" ");
    model += m_Name + QString(" ");
    model += m_Type + QString(" ");

    qCInfo(appLog) << "Computer overview:" << model;
    return model;
}

const QString DeviceComputer::getOSInfo()
{
    // qCDebug(appLog) << "DeviceComputer::getOSInfo called.";
    QString osInfo = m_OsDescription + " " + m_OS;
    // qCDebug(appLog) << "OS Info: " << osInfo;
    return osInfo;
}

void DeviceComputer::initFilterKey()
{
    qCDebug(appLog) << "DeviceComputer::initFilterKey called.";
    // initFilterKey is intentionally empty based on existing code.
}

void DeviceComputer::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceComputer::loadBaseDeviceInfo called.";
    // 添加基本信息
    addBaseDeviceInfo(("Name"), m_Name);
}

void DeviceComputer::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "DeviceComputer::loadOtherDeviceInfo called.";
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceComputer::loadTableData()
{
    // qCDebug(appLog) << "DeviceComputer::loadTableData called. No data to load based on existing function body.";

}
