#include "DeviceComputer.h"
#include <QDebug>

DeviceComputer::DeviceComputer()
    : m_HomeUrl(""), m_Description("")
    , m_OS(""), m_Vendor(""), m_ProductName("")
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
    setAttribute(mapInfo, "Board name", m_ProductName);
}

void DeviceComputer::setUOSInfo(const QString &info)
{
    m_Description = info;
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
    setAttribute(mapInfo, "product", m_ProductName, false);
}


const QString &DeviceComputer::homeUrl()const
{
    return m_HomeUrl;
}

const QString &DeviceComputer::description()
{
    return m_Description;
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
    return m_ProductName;
}

const QString &DeviceComputer::type()
{
    return m_Type;
}
void DeviceComputer::setHomeUrl(const QString &value)
{
    m_HomeUrl = value;
}
void DeviceComputer::setDescription(const QString &value)
{
    m_Description = value;
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
    m_ProductName = value;
}
void DeviceComputer::setType(const QString &value)
{
    m_Type = value;
}
void DeviceComputer::initFilterKey()
{

}
