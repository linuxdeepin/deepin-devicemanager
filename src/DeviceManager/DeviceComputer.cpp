#include "DeviceComputer.h"
#include <QDebug>

DeviceComputer::DeviceComputer()
    : m_PrettyName(""), m_Name(""), m_OsVersionID(""), m_OsVersion("")
    , m_ID(""), m_BugReportUrl(""), m_HomeUrl(""), m_DistributorID(""), m_Description("")
    , m_Release(""), m_CodeName(""), m_OS(""), m_Vendor(""), m_ProductName("")
    , m_Version(""), m_Family(""), m_Type("")
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
    setAttribute(mapInfo, "PRETTY_NAME", m_PrettyName);
    setAttribute(mapInfo, "NAME", m_Name);
    setAttribute(mapInfo, "VERSION_ID", m_OsVersionID);
    setAttribute(mapInfo, "VERSION", m_OsVersion);
    setAttribute(mapInfo, "ID", m_ID);
    setAttribute(mapInfo, "BUG_REPORT_URL", m_BugReportUrl);
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

    setAttribute(mapInfo, "description", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "product", m_ProductName, false);
}

const QString &DeviceComputer::prettyName()const
{
    return m_PrettyName;
}
const QString &DeviceComputer::name()const
{
    return m_Name;
}
const QString &DeviceComputer::osVersionID()const
{
    return m_OsVersionID;
}
const QString &DeviceComputer::osVersion()const
{
    return m_OsVersion;
}
const QString &DeviceComputer::id()const
{
    return m_ID;
}
const QString &DeviceComputer::bugReportUrl()const
{
    return m_BugReportUrl;
}
const QString &DeviceComputer::homeUrl()const
{
    return m_HomeUrl;
}
const QString &DeviceComputer::distributorID()
{
    return m_DistributorID;
}
const QString &DeviceComputer::description()
{
    return m_Description;
}
const QString &DeviceComputer::release()
{
    return m_Release;
}
const QString &DeviceComputer::codeName()
{
    return m_CodeName;
}

const QString &DeviceComputer::os()
{
    return m_OS;
}
const QString &DeviceComputer::vendor()
{
    return m_Vendor;
}
const QString &DeviceComputer::prodectName()
{
    return m_ProductName;
}
const QString &DeviceComputer::version()
{
    return m_Version;
}
const QString &DeviceComputer::family()
{
    return m_Family;
}

const QString &DeviceComputer::type()
{
    return m_Type;
}
void DeviceComputer::initFilterKey()
{

}
