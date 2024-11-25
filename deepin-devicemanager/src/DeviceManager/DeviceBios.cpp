// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceBios.h"

// Qt库文件
#include <QLoggingCategory>

DeviceBios::DeviceBios()
    : DeviceBaseInfo()
    , m_ProductName("")
    , m_ChipsetFamily("")
    , m_IsBoard(false)
{
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DeviceBios::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;

    ret = setTomlAttribute(mapInfo, "Version", m_Version, true);
    ret = setTomlAttribute(mapInfo, "Product Name", m_ProductName, true);
    ret = setTomlAttribute(mapInfo, "Chipset", m_ChipsetFamily, true);
    // ret = setTomlAttribute(mapInfo, "Vendor", m_Vendor,true);
    // m_IsBoard = true;

    //3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

bool DeviceBios::setBiosInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2)
        return false;

    // 获取BIOS信息
    m_Name = ("BIOS Information");
    m_tomlName = ("BIOS Information");
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);

    // 获取BIOS其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setBiosLanguageInfo(const QMap<QString, QString> &mapInfo)
{
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setBaseBoardInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2)
        return false;

    // 获取主板信息
    m_Name = ("Base Board Information");
    m_tomlName = ("Base Board Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Product Name", m_ProductName);
    setAttribute(mapInfo, "Board name", m_ProductName, false);
    setAttribute(mapInfo, "chipset", m_ChipsetFamily);

    // 该信息为主板信息
    m_IsBoard = true;

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setSystemInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2)
        return false;

    // 获取系统信息
    m_Name = ("System Information");
    m_tomlName = ("System Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setChassisInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2)
        return false;

    // 获取机箱信息
    m_Name = ("Chassis Information");
    m_tomlName = ("Chassis Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

bool DeviceBios::setMemoryInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2)
        return false;

    // 获取内存插槽信息
    m_Name = ("Physical Memory Array");
    m_tomlName = ("Physical Memory Array");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);

    // 获取其他信息
    getOtherMapInfo(mapInfo);
    return true;
}

const QString &DeviceBios::name()const
{
    return m_Name;
}

const QString &DeviceBios::tomlname()
{
    return m_tomlName;
}

const QString &DeviceBios::driver() const
{
    return m_Driver;
}

const QString &DeviceBios::vendor()const
{
    return m_Vendor;
}

const QString &DeviceBios::version()const
{
    return m_Version;
}

bool DeviceBios::isBoard()const
{
    return m_IsBoard;
}

QString DeviceBios::subTitle()
{
    return QObject::tr(m_Name.toLatin1());
}

const QString DeviceBios::getOverviewInfo()
{
    // 获取主板概况信息
    if (isBoard())
        return m_ProductName;
    else
        return QString("");
}

void DeviceBios::initFilterKey()
{

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

}

void DeviceBios::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(("Vendor"), m_Vendor);
    addBaseDeviceInfo(("Version"), m_Version);
    addBaseDeviceInfo(("Chipset"), m_ChipsetFamily);
}

void DeviceBios::loadOtherDeviceInfo()
{
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceBios::loadTableHeader()
{
    // 主板界面无表格,清空表头
    m_TableHeader.clear();
}

void DeviceBios::loadTableData()
{

}
