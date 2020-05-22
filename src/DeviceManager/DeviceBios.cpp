#include "DeviceBios.h"
#include <QDebug>

DeviceBios::DeviceBios()
    : DeviceBaseInfo(), m_Name(""), m_ProductName(""), m_Vendor(""), m_Version(""), m_ChipsetFamily(""), m_IsBoard(false)
{
    initFilterKey();
}

bool DeviceBios::setBiosInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromDmidecode(mapInfo, info);
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("BIOS Information");
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setBiosInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("BIOS Information");
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setBaseBoardInfo(const QString &info, const QString &chipsetFamliy)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromDmidecode(mapInfo, info);
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Base Board Information");
    m_ChipsetFamily = chipsetFamliy;
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Product Name", m_ProductName);
    setAttribute(mapInfo, "Board name", m_ProductName, false);
    m_IsBoard = true;
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setBaseBoardInfo(const QMap<QString, QString> &mapInfo, const QString &chipsetFamliy)
{
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Base Board Information");
    m_ChipsetFamily = chipsetFamliy;
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    setAttribute(mapInfo, "Product Name", m_ProductName);
    setAttribute(mapInfo, "Board name", m_ProductName, false);
    m_IsBoard = true;
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setSystemInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromDmidecode(mapInfo, info);
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("System Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}


bool DeviceBios::setSystemInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("System Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setChassisInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromDmidecode(mapInfo, info);
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Chassis Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}


bool DeviceBios::setChassisInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Chassis Information");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setMemoryInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromDmidecode(mapInfo, info);
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Physical Memory Array");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

bool DeviceBios::setMemoryInfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.size() < 2) {
        return false;
    }
    m_Name = QObject::tr("Physical Memory Array");
    setAttribute(mapInfo, "Manufacturer", m_Vendor);
    setAttribute(mapInfo, "Version", m_Version);
    loadOtherDeviceInfo(mapInfo);
    return true;
}

const QString &DeviceBios::name()const
{
    return m_Name;
}
const QString &DeviceBios::productName()const
{
    return m_ProductName;
}
const QString &DeviceBios::vendor()const
{
    return m_Vendor;
}
const QString &DeviceBios::version()const
{
    return m_Version;
}
const QString &DeviceBios::chipsetFamily()const
{
    return m_ChipsetFamily;
}
bool DeviceBios::isBoard()const
{
    return m_IsBoard;
}

void DeviceBios::initFilterKey()
{
    addFilterKey(QObject::tr("Release Date"));
    addFilterKey(QObject::tr("Address"));
    addFilterKey(QObject::tr("Runtime Size"));
    addFilterKey(QObject::tr("ROM Size"));
    addFilterKey(QObject::tr("Characteristics"));
    addFilterKey(QObject::tr("BIOS Revision"));

    addFilterKey(QObject::tr("Product Name"));
    addFilterKey(QObject::tr("Serial Number"));
    addFilterKey(QObject::tr("Asset Tag"));
    addFilterKey(QObject::tr("Features"));
    addFilterKey(QObject::tr("Location In Chassis"));
    addFilterKey(QObject::tr("Chassis Handle"));
    addFilterKey(QObject::tr("Type"));
    addFilterKey(QObject::tr("Contained Object Handles"));

    addFilterKey(QObject::tr("Product Name"));
    addFilterKey(QObject::tr("Version"));
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
//    addFilterKey(QObject::tr("Use"));
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
}

void DeviceBios::getMapInfoFromDmidecode(QMap<QString, QString> &mapInfo, const QString &info)
{
    QStringList lines = info.split("\n");
    QString lasKey;
    foreach (const QString &line, lines) {
        if (line.isEmpty()) {
            continue;
        }
        QStringList words = line.split(": ");
        if (words.size() == 1 && words[0].endsWith(":")) {
            lasKey = words[0].replace(QRegExp(":$"), "");
            mapInfo.insert(lasKey.trimmed(), " ");
        } else if (words.size() == 1 && !lasKey.isEmpty()) {
            mapInfo[lasKey.trimmed()] += words[0];
            mapInfo[lasKey.trimmed()] += "  /  ";
        } else if (words.size() == 2) {
            lasKey = "";
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}
