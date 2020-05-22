#include "DeviceComputer.h"
#include <QDebug>

DeviceComputer::DeviceComputer()
    : m_HomeUrl(""), m_OsDescription("")
    , m_OS(""), m_Vendor(""), m_Name("")
    ,  m_Type("")
{

}

void DeviceComputer::setInfoFromCatOsRelease(const QString &info)
{
    // 解析信息
    QMap<QString, QString> mapInfo;
    QStringList lines = info.split("\n");
    foreach (QString line, lines) {
        line.replace("\"", "");
        QStringList words = line.trimmed().split("=");
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }

    // 设置属性
    setAttribute(mapInfo, "HOME_URL", m_HomeUrl);
}

void DeviceComputer::setInfoFromCatBoardInfo(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfo(mapInfo, info);
    setAttribute(mapInfo, "Board name", m_Name);
}

void DeviceComputer::setUOSInfo(const QString &info)
{
    m_OsDescription = info;
}

void DeviceComputer::setInfoFromCatDevice(const QString &info)
{
    m_OS = info.trimmed();



    QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
    int index = reg.indexIn(m_OS);
    if (index != -1) {
        QString tmp = reg.cap(0);
        m_OS.remove(tmp);
        m_OS.insert(index, reg.cap(1));
    }
}

void DeviceComputer::setChassisInfoFromDmidecode(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfo(mapInfo, info);
    setAttribute(mapInfo, "Type", m_Type);
}

void DeviceComputer::setInfoFromLshw(const QString &info)
{
    QMap<QString, QString> mapInfo;
    getMapInfoFromLshw(mapInfo, info);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "product", m_Name, false);
}


const QString &DeviceComputer::homeUrl()const
{
    return m_HomeUrl;
}

const QString &DeviceComputer::osDescription()
{
    return m_OsDescription;
}

const QString &DeviceComputer::os()
{
    return m_OS;
}
const QString &DeviceComputer::vendor()
{
    return m_Vendor;
}
const QString &DeviceComputer::name()
{
    if (m_Name.contains("None")) {
        m_Name = "";
    }
    return m_Name;
}

const QString &DeviceComputer::type()
{
    return m_Type;
}
void DeviceComputer::setHomeUrl(const QString &value)
{
    m_HomeUrl = value;
}
void DeviceComputer::setOsDescription(const QString &value)
{
    m_OsDescription = value;
}
void DeviceComputer::setOS(const QString &value)
{
    m_OS = value;
}
void DeviceComputer::setVendor(const QString &value)
{
    m_Vendor = value;
}
void DeviceComputer::setName(const QString &value)
{
    m_Name = value;
}
void DeviceComputer::setType(const QString &value)
{
    m_Type = value;
}
void DeviceComputer::initFilterKey()
{

}
