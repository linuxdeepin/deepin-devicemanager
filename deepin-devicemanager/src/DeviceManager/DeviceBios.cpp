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
    m_Name = "BIOS Information";
    m_tomlName = "BIOS Information";
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
    m_Name = "Base Board Information";
    m_tomlName = "Base Board Information";
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
    m_Name = "System Information";
    m_tomlName = "System Information";
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
    m_Name = "Chassis Information";
    m_tomlName = "Chassis Information";
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
    m_Name = "Physical Memory Array";
    m_tomlName = "Physical Memory Array";
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
    return translateStr(m_Name);;
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
    addFilterKey("Release Date");
    addFilterKey("Address");
    addFilterKey("Runtime Size");
    addFilterKey("ROM Size");
    addFilterKey("Characteristics");
    addFilterKey("BIOS Revision");
    addFilterKey("Firmware Revision");

    addFilterKey("Product Name");
    addFilterKey("Serial Number");
    addFilterKey("Asset Tag");
    addFilterKey("Features");
    addFilterKey("Location In Chassis");
    addFilterKey("Chassis Handle");
    addFilterKey("Type");
    addFilterKey("Contained Object Handles");

    addFilterKey("Product Name");
    addFilterKey("Serial Number");
    addFilterKey("UUID");
    addFilterKey("Wake-up Type");
    addFilterKey("SKU Number");
    addFilterKey("Family");

    addFilterKey("Type");
    addFilterKey("Lock");
    addFilterKey("Serial Number");
    addFilterKey("Asset Tag");
    addFilterKey("Boot-up State");
    addFilterKey("Power Supply State");
    addFilterKey("Thermal State");
    addFilterKey("Security Status");
    addFilterKey("OEM Information");
    addFilterKey("Height");
    addFilterKey("Number Of Power Cords");
    addFilterKey("Contained Elements");
    addFilterKey("SKU Number");

    addFilterKey("Location");
    addFilterKey("Error Correction Type");
    addFilterKey("Maximum Capacity");
    addFilterKey("Error Information Handle");
    addFilterKey("Number Of Devices");

    addFilterKey("BIOS ROMSIZE");
    addFilterKey("Release date");
    addFilterKey("Board name");
    addFilterKey("Family");

    addFilterKey("BIOS Information");
    addFilterKey("Base Board Information");
    addFilterKey("System Information");
    addFilterKey("Chassis Information");
    addFilterKey("Physical Memory Array");

    addFilterKey("SMBIOS Version");

    addFilterKey("Language Description Format");
    addFilterKey("Installable Languages");
    addFilterKey("Currently Installed Language");
    qCDebug(appLog) << "Filter keys initialized.";
}

void DeviceBios::loadBaseDeviceInfo()
{
    qCDebug(appLog) << "DeviceBios::loadBaseDeviceInfo called.";
    // 添加基本信息
    addBaseDeviceInfo("Vendor", m_Vendor);
    addBaseDeviceInfo("Version", m_Version);
    addBaseDeviceInfo("Chipset", m_ChipsetFamily);
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
