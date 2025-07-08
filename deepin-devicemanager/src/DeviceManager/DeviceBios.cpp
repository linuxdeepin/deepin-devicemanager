// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceBios.h"
#include "DDLog.h"

// Qt库文件
#include <QLoggingCategory>

using namespace DDLog;

DeviceBios::DeviceBios()
    : DeviceBaseInfo()
    , m_ProductName("")
    , m_ChipsetFamily("")
    , m_IsBoard(false)
{
    qCDebug(appLog) << "DeviceBios constructor initialized";
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DeviceBios::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBios::setInfoFromTomlOneByOne started.";
    TomlFixMethod ret = TOML_None;

    ret = setTomlAttribute(mapInfo, "Version", m_Version, true);
    ret = setTomlAttribute(mapInfo, "Product Name", m_ProductName, true);
    ret = setTomlAttribute(mapInfo, "Chipset", m_ChipsetFamily, true);
    // ret = setTomlAttribute(mapInfo, "Vendor", m_Vendor,true);
    // m_IsBoard = true;
    qCDebug(appLog) << "Toml attributes set for Version, Product Name, Chipset.";

    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setInfoFromTomlOneByOne finished, return: " << ret;
    return ret;
}

bool DeviceBios::setBiosInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting BIOS info";

    if (mapInfo.size() < 2) {
        qCDebug(appLog) << "mapInfo size less than 2, returning false.";
        return false;
    }

    // 获取BIOS信息
    m_Name = QObject::tr("BIOS Information");
    m_tomlName = ("BIOS Information");
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "BIOS info attributes set.";

    // 获取BIOS其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setBiosInfo finished.";
    return true;
}

bool DeviceBios::setBiosLanguageInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting bios language info";
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setBaseBoardInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Start setting base board info";

    if (mapInfo.size() < 2) {
        qCDebug(appLog) << "mapInfo size less than 2, returning false.";
        return false;
    }

    // 获取主板信息
    m_Name = QObject::tr("Base Board Information");
    m_tomlName = ("Base Board Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Product Name", m_ProductName);
    setAttribute(mapInfo, "Board name", m_ProductName, false);
    setAttribute(mapInfo, "chipset", m_ChipsetFamily);
    qCDebug(appLog) << "Base board info attributes set.";

    // 该信息为主板信息
    m_IsBoard = true;
    qCDebug(appLog) << "m_IsBoard set to true.";

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setBaseBoardInfo finished.";
    return true;
}

bool DeviceBios::setSystemInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBios::setSystemInfo started.";
    if (mapInfo.size() < 2) {
        qCDebug(appLog) << "mapInfo size less than 2, returning false.";
        return false;
    }

    // 获取系统信息
    m_Name = QObject::tr("System Information");
    m_tomlName = ("System Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "System info attributes set.";

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setSystemInfo finished.";
    return true;
}

bool DeviceBios::setChassisInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBios::setChassisInfo started.";
    if (mapInfo.size() < 2) {
        qCDebug(appLog) << "mapInfo size less than 2, returning false.";
        return false;
    }

    // 获取机箱信息
    m_Name = QObject::tr("Chassis Information");
    m_tomlName = ("Chassis Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "Chassis info attributes set.";

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setChassisInfo finished.";
    return true;
}

bool DeviceBios::setMemoryInfo(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "DeviceBios::setMemoryInfo started.";
    if (mapInfo.size() < 2) {
        qCDebug(appLog) << "mapInfo size less than 2, returning false.";
        return false;
    }

    // 获取内存插槽信息
    m_Name = QObject::tr("Physical Memory Array");
    m_tomlName = ("Physical Memory Array");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    qCDebug(appLog) << "Memory info attributes set.";

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    qCDebug(appLog) << "DeviceBios::setMemoryInfo finished.";
    return true;
}

const QString &DeviceBios::name()const
{
    // qCDebug(appLog) << "DeviceBios::name called, returning: " << m_Name;
    return m_Name;
}

const QString &DeviceBios::tomlname()
{
    // qCDebug(appLog) << "DeviceBios::tomlname called, returning: " << m_tomlName;
    return m_tomlName;
}

const QString &DeviceBios::driver() const
{
    // qCDebug(appLog) << "DeviceBios::driver called, returning: " << m_Driver;
    return m_Driver;
}

const QString &DeviceBios::vendor()const
{
    // qCDebug(appLog) << "DeviceBios::vendor called, returning: " << m_Vendor;
    return m_Vendor;
}

const QString &DeviceBios::version()const
{
    // qCDebug(appLog) << "DeviceBios::version called, returning: " << m_Version;
    return m_Version;
}

bool DeviceBios::isBoard()const
{
    // qCDebug(appLog) << "DeviceBios::isBoard called, returning: " << m_IsBoard;
    return m_IsBoard;
}

QString DeviceBios::subTitle()
{
    // qCDebug(appLog) << "DeviceBios::subTitle called, returning: " << m_Name;
    return m_Name;
}

const QString DeviceBios::getOverviewInfo()
{
    qCDebug(appLog) << "Getting BIOS overview info";

    // 获取主板概况信息
    if (isBoard()) {
        qCDebug(appLog) << "Is board, returning product name: " << m_ProductName;
        return m_ProductName;
    } else {
        qCDebug(appLog) << "Not board, returning empty string.";
        return QString("");
    }
}

void DeviceBios::initFilterKey()
{
    qCDebug(appLog) << "DeviceBios::initFilterKey called.";

    // 添加可显示属性
    addFilterKey(QObject::tr("Release Date"));
    addFilterKey(QObject::tr("Address"));
    addFilterKey(QObject::tr("Runtime Size"));
    addFilterKey(QObject::tr("ROM Size"));
    addFilterKey(QObject::tr("Characteristics"));
    addFilterKey(QObject::tr("BIOS Revision"));
    addFilterKey(QObject::tr("Firmware Revision"));

    addFilterKey(QObject::tr("Product Name"));
    addFilterKey(QObject::tr("Serial Number"));
    addFilterKey(QObject::tr("Asset Tag"));
    addFilterKey(QObject::tr("Features"));
    addFilterKey(QObject::tr("Location In Chassis"));
    addFilterKey(QObject::tr("Chassis Handle"));
    addFilterKey(QObject::tr("Type"));
    addFilterKey(QObject::tr("Contained Object Handles"));

    addFilterKey(QObject::tr("Product Name"));
    addFilterKey(QObject::tr("Serial Number"));
    addFilterKey(QObject::tr("UUID"));
    addFilterKey(QObject::tr("Wake-up Type"));
    addFilterKey(QObject::tr("SKU Number"));
    addFilterKey(QObject::tr("Family"));


    addFilterKey(QObject::tr("Type"));
    addFilterKey(QObject::tr("Lock"));
    addFilterKey(QObject::tr("Serial Number"));
    addFilterKey(QObject::tr("Asset Tag"));
    addFilterKey(QObject::tr("Boot-up State"));
    addFilterKey(QObject::tr("Power Supply State"));
    addFilterKey(QObject::tr("Thermal State"));
    addFilterKey(QObject::tr("Security Status"));
    addFilterKey(QObject::tr("OEM Information"));
    addFilterKey(QObject::tr("Height"));
    addFilterKey(QObject::tr("Number Of Power Cords"));
    addFilterKey(QObject::tr("Contained Elements"));
    addFilterKey(QObject::tr("SKU Number"));

    addFilterKey(QObject::tr("Location"));
    addFilterKey(QObject::tr("Error Correction Type"));
    addFilterKey(QObject::tr("Maximum Capacity"));
    addFilterKey(QObject::tr("Error Information Handle"));
    addFilterKey(QObject::tr("Number Of Devices"));

    addFilterKey(QObject::tr("BIOS ROMSIZE"));
    addFilterKey(QObject::tr("Release date"));
    addFilterKey(QObject::tr("Board name"));
    addFilterKey(QObject::tr("Family"));

    addFilterKey(QObject::tr("BIOS Information"));
    addFilterKey(QObject::tr("Base Board Information"));
    addFilterKey(QObject::tr("System Information"));
    addFilterKey(QObject::tr("Chassis Information"));
    addFilterKey(QObject::tr("Physical Memory Array"));

    addFilterKey(QObject::tr("SMBIOS Version"));

    addFilterKey(QObject::tr("Language Description Format"));
    addFilterKey(QObject::tr("Installable Languages"));
    addFilterKey(QObject::tr("Currently Installed Language"));
    qCDebug(appLog) << "Filter keys initialized.";
}

void DeviceBios::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceBios::loadBaseDeviceInfo called.";
    // 添加基本信息
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Chipset"), m_ChipsetFamily);
}

void DeviceBios::loadOtherDeviceInfo()
{
    qCDebug(appLog) << "Loading other device info";
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceBios::loadTableHeader()
{
    // qCDebug(appLog) << "Loading table header";
    // 主板界面无表格,清空表头
    m_TableHeader.clear();
}

void DeviceBios::loadTableData()
{
    // qCDebug(appLog) << "Loading table data";
}
