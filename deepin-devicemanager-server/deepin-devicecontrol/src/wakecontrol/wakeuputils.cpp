// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wakeuputils.h"
#include "enablesqlmanager.h"
#include "DDLog.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QLoggingCategory>
#include <QCryptographicHash>
#include <QRegularExpression>
#define LEAST_NUM 10

using namespace DDLog;

WakeupUtils::WakeupUtils()
{

}

void WakeupUtils::updateWakeupDeviceInfo(const QString &info)
{
    qCDebug(appLog) << "Updating wakeup device info";
    QStringList items = info.split("\n\n");
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;

        QString uniqueID;
        if (mapItem.contains("Unique ID"))
            uniqueID = mapItem["Unique ID"];
        if (mapItem.contains("Vendor") && mapItem.contains("Device") && (mapItem.contains("SysFS ID") || mapItem.contains("SysFS Device Link"))
                && mapItem["Vendor"].contains("0x") && mapItem["Device"].contains("0x")) {
            QStringList vendorlist = mapItem["Vendor"].split(" ");
            QStringList devicelist = mapItem["Device"].split(" ");
            if (vendorlist.size() > 1 && devicelist.size() > 1  && ((mapItem.contains("SysFS ID") && !mapItem["SysFS ID"].isEmpty())
                                                                    || (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()))) {
                QString valueStr = vendorlist[1].trimmed() + devicelist[1].remove("0x", Qt::CaseSensitive).trimmed();
                QCryptographicHash Hash(QCryptographicHash::Md5);
                QByteArray buf;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                // 在 Qt5 中，QByteArray::append() 可以直接接受 QString 类型的参数，会自动进行类型转换,qt6 需要使用 toUtf8() 进行显式转换
                buf.append(valueStr);
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
                    buf.append(mapItem["SysFS Device Link"].trimmed());
                } else {
                    buf.append(mapItem["SysFS ID"].trimmed());
                }
#else
                buf.append(valueStr.toUtf8());
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
                    buf.append(mapItem["SysFS Device Link"].trimmed().toUtf8());
                } else {
                    buf.append(mapItem["SysFS ID"].trimmed().toUtf8());
                }
#endif
                Hash.addData(buf);
                uniqueID = QString::fromStdString(Hash.result().toBase64().toStdString());
            }
        }

        // Unique ID
        if (uniqueID.isEmpty())
            continue;

        // 查找数据库，判断是否存在相同 Unique ID
        if (!EnableSqlManager::getInstance()->isWakeupUniqueIdExisted(uniqueID))
            continue;

        // 判断数据库里面的记录状态更新信息
        QString wp;
        if (wakeupPath(mapItem["SysFS ID"].isEmpty() ? getPS2Syspath(mapItem["Device Files"]) : mapItem["SysFS ID"], wp)) {
            bool wakeup = EnableSqlManager::getInstance()->isWakeup(uniqueID);
            writeWakeupFile(wp, wakeup);
        }
    }
}

bool WakeupUtils::wakeupPath(const QString &syspath, QString &wakeuppath)
{
    qCDebug(appLog) << "Finding wakeup path for syspath:" << syspath;
    int index = syspath.lastIndexOf('/');
    if (index < 1) {
        return false;
    }

    wakeuppath = QString("/sys") + syspath.left(index) + QString("/power/wakeup");
    if (!QFile::exists(wakeuppath))
        return false;
    return true;
}

bool WakeupUtils::writeWakeupFile(const QString &path, bool wakeup)
{
    qCDebug(appLog) << "Writing wakeup file:" << path << "value:" << (wakeup ? "enabled" : "disabled");
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite)) {
        return false;
    }

    qint64 size = -1;
    if (wakeup)
        size = file.write("enabled");
    else
        size = file.write("disabled");
    file.close();

    if (size < 1)
        return false;
    return true;
}

#define UNPARSE_WOLOPTS(x) ((x & WAKE_PHY) || (x & WAKE_UCAST) || (x & WAKE_MCAST) || (x & WAKE_BCAST) || (x & WAKE_ARP) || (x & WAKE_MAGIC) || (x & WAKE_MAGICSECURE) || (x & WAKE_FILTER))
/* refer spec : ethtool.c
    https://lwn.net/Articles/931289/
    https://linuxconfig.org/introduction-to-wake-on-lan
    https://wiki.archlinux.org/title/Wake-on-LAN
*/
WakeupUtils::EthStatus WakeupUtils::wakeOnLanIsOpen(const QString &logicalName)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return ES_SOCKET_FAILED;
    }

    struct ifreq ifr;
    struct ethtool_wolinfo wolinfo;
    memset(&ifr, 0, sizeof(ifr));
    memset(&wolinfo, 0, sizeof(wolinfo));

    QByteArray nameBytes = logicalName.toLocal8Bit();
    strncpy(ifr.ifr_name, nameBytes.constData(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    wolinfo.cmd = ETHTOOL_GWOL;
    ifr.ifr_data = reinterpret_cast<char *>(&wolinfo);
    if (0 != ioctl(fd, SIOCETHTOOL, &ifr)) {
        return ES_IOCTL_ERROR;
    }
    qCInfo(appLog) << "wakeOnLan supported:" << wolinfo.supported << "wolopts:" << wolinfo.wolopts;

    if (wolinfo.supported && UNPARSE_WOLOPTS(wolinfo.supported)) {
        if (wolinfo.wolopts && UNPARSE_WOLOPTS(wolinfo.wolopts)) {
            return ES_WAKE_ON_OPEN;
        }
        else
            return ES_WAKE_ON_CLOSE;
    }
    else
        return ES_NOT_SUPPORT_WAKE_ON;

    return ES_WAKE_ON_UNKNOW;
}

bool WakeupUtils::setWakeOnLan(const QString &logicalName, bool open)
{
    qCDebug(appLog) << "Setting Wake-on-LAN for interface:" << logicalName << "enable:" << open;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
        return false;

    struct ifreq ifr;
    struct ethtool_wolinfo wolinfo;
    memset(&ifr, 0, sizeof(ifr));
    memset(&wolinfo, 0, sizeof(wolinfo));

    QByteArray nameBytes = logicalName.toLocal8Bit();
    strncpy(ifr.ifr_name, nameBytes.constData(), IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    wolinfo.cmd = ETHTOOL_SWOL;
    if (open)
        wolinfo.wolopts = 0 | WAKE_MAGIC;
    else
        wolinfo.wolopts = 0;
    ifr.ifr_data = reinterpret_cast<char *>(&wolinfo);

    if (0 == ioctl(fd, SIOCETHTOOL, &ifr))
        return true;
    return false;
}

bool WakeupUtils::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Parsing device info map";
    QStringList lines = item.split("\n");
    // 行数太少则为无用信息
    if (lines.size() <= LEAST_NUM) {
        return false;
    }

    foreach (const QString &line, lines) {
        QStringList words = line.split(": ");
        if (words.size() != 2)
            continue;
        mapInfo.insert(words[0].trimmed(), words[1].replace("\"", "").trimmed());
    }

    if (mapInfo["Hardware Class"] != "keyboard" && mapInfo["Hardware Class"] != "mouse")
        return false;

    return true;
}

QString WakeupUtils::getPS2Syspath(const QString &dfs)
{
    qCDebug(appLog) << "Getting PS/2 syspath for device:" << dfs;
    // 获取 dfs 中的 event
    QRegularExpression regdfs("(event[0-9]{1,2})");
    QRegularExpressionMatch matchDfs = regdfs.match(dfs);
    if (!matchDfs.hasMatch()) {
        return "";
    }
    QString eventdfs = matchDfs.captured(1);

    QFile file("/proc/bus/input/devices");
    if (!file.open(QIODevice::ReadOnly))
        return "";

    QString info = file.readAll();
    QStringList lstDevices = info.split("\n\n");
    foreach (const QString &item, lstDevices) {
        QStringList lines = item.split("\n");
        QString sysfs = "";
        QString event = "";
        foreach (const QString &line, lines) {
            if (line.startsWith("S:")) {
                sysfs = line;
                continue;
            }
            QRegularExpression reg("H: Handlers=.*(event[0-9]{1,2})");
            QRegularExpressionMatch match = reg.match(line);
            if (match.hasMatch()) {
                event = match.captured(1);
            }
        }

        if (!event.isEmpty() && !sysfs.isEmpty()) {
            if (event == eventdfs) {
                QRegularExpression regfs;
                if (sysfs.contains("i2c_designware"))
                    regfs = QRegularExpression("S: Sysfs=(.*)/input/input[0-9]{1,2}");
                else
                    regfs = QRegularExpression("S: Sysfs=(.*)/input[0-9]{1,2}");
                QRegularExpressionMatch matchFs = regfs.match(sysfs);
                if (matchFs.hasMatch()) {
                    return matchFs.captured(1);
                }
            }
        }
    }

    return "";
}
