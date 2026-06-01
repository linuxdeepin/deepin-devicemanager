// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enableutils.h"
#include "enablesqlmanager.h"
#include "DDLog.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QProcess>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>

#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

using namespace DDLog;

#define LEAST_NUM 10
#define REG_ADDRESS "^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$"

EnableUtils::EnableUtils()
{

}

void EnableUtils::disableOutDevice(const QString &info)
{
    qCDebug(appLog) << "Disabling out device with info:" << info;
    QStringList items = info.split("\n\n");
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;

        // 获取设备的唯一标识
        // 网卡的唯一标识为网卡的物理地址
        // 有序列号id的设备使用序列号id作为唯一标识
        QString uniqueID;
        if (mapItem.find("Permanent HW Address") != mapItem.end()) {
            uniqueID = mapItem["Permanent HW Address"];
        } else if (mapItem.find("Serial ID") != mapItem.end()) {
            uniqueID = mapItem["Serial ID"];
        } else {
            uniqueID = mapItem["Module Alias"];
            uniqueID.replace(QRegularExpression("[0-9a-zA-Z]{10}$"), "");
        }

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
        if (uniqueID.isEmpty()) {
            continue;
        }

        // 获取设备信息路径
        // 有 SysFS Device Link 的使用 SysFS Device Link
        // 没有 SysFS Device Link 的使用 SysFS ID
        // 如果是网卡则使用逻辑名称作为path
        QString path;
        if (mapItem.find("SysFS Device Link") != mapItem.end()) {
            path = mapItem["SysFS Device Link"];
        } else {
            path = mapItem["SysFS ID"];
        }
        path.replace(QRegularExpression("[1-9]$"), "0");


        // 网卡采用ioctl的方式禁用
        QRegularExpression reg(REG_ADDRESS);
        if (reg.match(uniqueID).hasMatch()) {
            path = mapItem["Device File"];
            if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID) &&
                    EnableUtils::ioctlOperateNetworkLogicalName(path, false))
                EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID, path);
            continue;
        }


        // 先判断设备是否被记录在数据库，如果在则禁用
        if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID)) {
            QFile file("/sys" + path + QString("/authorized"));
            if (!file.open(QIODevice::ReadWrite)) {
                return;
            }
            file.write("0");
            file.close();
            // 更数据库信息，方式更换usb接口
            EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID, path);
        }
    }
}

void EnableUtils::disableInDevice()
{
    qCDebug(appLog) << "Disabling in devices";
    // 网卡通过ioctl禁用
    QList<QPair<QString, QString> > lstAuthPair;
    EnableSqlManager::getInstance()->authorizedPathUniqueIDList(lstAuthPair);
    for (QList<QPair<QString, QString>>::iterator it = lstAuthPair.begin() ; it != lstAuthPair.end(); ++it) {
        QRegularExpression reg(REG_ADDRESS);
        if (reg.match((*it).second).hasMatch()) {
            EnableUtils::ioctlOperateNetworkLogicalName((*it).first, false);
            continue;
        }
    }


    // 其它通过remove文件禁用
    QList<QPair<QString, QString> > lstRemovePair;
    EnableSqlManager::getInstance()->removePathUniqueIDList(lstRemovePair);
    for (QList<QPair<QString, QString>>::iterator it = lstRemovePair.begin() ; it != lstRemovePair.end(); ++it) {
        QString pathT = "/sys" + (*it).first + QString("/remove");
        if (!QFile::exists(pathT)) {
            pathT = "/sys" + (*it).first + QString("/reset");
        }

        QFile file(pathT);
        if (file.open(QIODevice::WriteOnly)) {
            file.write("1");
            file.close();
        }
    }
}

bool EnableUtils::ioctlOperateNetworkLogicalName(const QString &logicalName, bool enable)
{
    qCDebug(appLog) << "Performing ioctl operation on network interface:" << logicalName << "enable:" << enable;
    // 方案一：通过NetworkManager D-Bus设置DeviceEnabled属性
    if (enableNetworkByDBus(logicalName, enable))
        return true;

    // 方案二：方案一失败，回退到ioctl方式
    qWarning() << "Fallback to ioctl method for" << logicalName;
    return enableNetworkByIoctl(logicalName, enable);
}

// 方案一：通过NetworkManager D-Bus设置DeviceEnabled属性
bool EnableUtils::enableNetworkByDBus(const QString &logicalName, bool enable)
{
    // 1. 通过 system dbus 获取 devicePath
    QDBusInterface nmInterface("org.freedesktop.NetworkManager",
                               "/org/freedesktop/NetworkManager",
                               "org.freedesktop.NetworkManager",
                               QDBusConnection::systemBus());
    if (!nmInterface.isValid()) {
        qCritical() << "Failed to connect to NetworkManager:" << nmInterface.lastError().message();
        return false;
    }

    QDBusReply<QDBusObjectPath> reply = nmInterface.call("GetDeviceByIpIface", logicalName);
    if (!reply.isValid()) {
        qCritical() << "Failed to GetDeviceByIpIface for" << logicalName << ":" << reply.error().message();
        return false;
    }

    QString devicePath = reply.value().path();
    if (devicePath.isEmpty()) {
        qCritical() << "Got empty devicePath for interface:" << logicalName;
        return false;
    }

    // 2. 通过 system dbus 设置 DeviceEnabled 属性
    QDBusInterface deviceInterface("org.freedesktop.NetworkManager",
                                   devicePath,
                                   "org.freedesktop.DBus.Properties",
                                   QDBusConnection::systemBus());
    if (!deviceInterface.isValid()) {
        qCritical() << "Failed to connect to device properties:" << deviceInterface.lastError().message();
        return false;
    }

    QDBusReply<void> setReply = deviceInterface.call("Set",
                                                     "org.freedesktop.NetworkManager.Device",
                                                     "DeviceEnabled",
                                                     QVariant::fromValue(QDBusVariant(enable)));
    if (!setReply.isValid()) {
        qCritical() << "Failed to set DeviceEnabled for" << logicalName << ":" << setReply.error().message();
        return false;
    }

    return true;
}

// 方案二：通过ioctl控制网卡
bool EnableUtils::enableNetworkByIoctl(const QString &logicalName, bool enable)
{
    // 安全校验：网卡名长度合法性
    if (logicalName.isEmpty() || logicalName.length() >= IFNAMSIZ) {
        qCritical() << "Invalid logicalName length for ioctl";
        return false;
    }

    // 通过ioctl设置网卡启用/禁用
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return false;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    std::string nameStr = logicalName.toStdString();
    strncpy(ifr.ifr_name, nameStr.c_str(),IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // 先获取标识
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return false;
    }

    if (enable) {
        ifr.ifr_flags |= IFF_UP;
    } else {
        ifr.ifr_flags &= ~IFF_UP;
    }
    // 设置新标识
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool EnableUtils::validateAndNormalizeVidPid(const QString &vid, const QString &pid, QString &normalizedVid, QString &normalizedPid)
{
    qCDebug(appLog) << "Validating and normalizing VID:" << vid << "PID:" << pid;
    
    // Check if VID or PID is empty
    if (vid.isEmpty() || pid.isEmpty()) {
        qCWarning(appLog) << "VID or PID is empty for validation";
        return false;
    }
    
    // Normalize VID
    normalizedVid = vid.toLower();
    if (normalizedVid.startsWith("0x")) {
        normalizedVid = normalizedVid.mid(2);
    }
    
    // Normalize PID
    normalizedPid = pid.toLower();
    if (normalizedPid.startsWith("0x")) {
        normalizedPid = normalizedPid.mid(2);
    }
    
    // Validate VID and PID format (should be 4 characters each after normalization)
    if (normalizedVid.length() != 4 || normalizedPid.length() != 4) {
        qCWarning(appLog) << "Invalid VID or PID format after normalization. VID:" << normalizedVid << "PID:" << normalizedPid;
        return false;
    }
    
    // Validate that VID and PID contain only hex characters
    QRegularExpression hexPattern("^[0-9a-f]{4}$");
    if (!hexPattern.match(normalizedVid).hasMatch() || !hexPattern.match(normalizedPid).hasMatch()) {
        qCWarning(appLog) << "VID or PID contains non-hex characters. VID:" << normalizedVid << "PID:" << normalizedPid;
        return false;
    }
    
    qCDebug(appLog) << "VID and PID validation successful. Normalized VID:" << normalizedVid << "Normalized PID:" << normalizedPid;
    return true;
}

bool EnableUtils::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
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

    // hub为usb接口，可以直接过滤
    if (mapInfo["Hardware Class"] == "hub") {
        return false;
    }

    // 没有总线信息的设备可以过滤
    if (mapInfo.find("SysFS BusID") == mapInfo.end()) {
        return false;
    }

    return true;
}
