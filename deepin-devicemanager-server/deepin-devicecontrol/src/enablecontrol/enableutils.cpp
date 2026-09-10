// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "enableutils.h"
#include "enablesqlmanager.h"

#include <QStringList>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QDebug>

#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define LEAST_NUM 10
#define REG_ADDRESS "^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$"

EnableUtils::EnableUtils()
{

}

void EnableUtils::disableOutDevice(const QString &info)
{
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
            uniqueID.replace(QRegExp("[0-9a-zA-Z]{10}$"), "");
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
                buf.append(valueStr);
                if (mapItem.contains("SysFS Device Link") && !mapItem["SysFS Device Link"].isEmpty()) {
                    buf.append(mapItem["SysFS Device Link"].trimmed());
                } else {
                    buf.append(mapItem["SysFS ID"].trimmed());
                }
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
        path.replace(QRegExp("[1-9]$"), "0");


        // 网卡采用ioctl的方式禁用
        QRegExp reg(REG_ADDRESS);
        if (reg.exactMatch(uniqueID)) {
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
    // 网卡通过ioctl禁用
    QList<QPair<QString, QString> > lstAuthPair;
    EnableSqlManager::getInstance()->authorizedPathUniqueIDList(lstAuthPair);
    for (QList<QPair<QString, QString>>::iterator it = lstAuthPair.begin() ; it != lstAuthPair.end(); ++it) {
        QRegExp reg(REG_ADDRESS);
        if (reg.exactMatch((*it).second)) {
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
    qDebug() << "[ioctlOperateNetworkLogicalName] enter, logicalName:" << logicalName << "enable:" << enable;

    // 首先判断当前网卡是有线网卡还是无线网卡
    if (isWirelessNetwork(logicalName)) {
        // 无线网卡：需要判断当前系统上无线网卡的个数
        int wirelessCount = wirelessNetworkCount();
        qDebug() << "[ioctlOperateNetworkLogicalName] wireless card detected, wireless count:" << wirelessCount;
        if (wirelessCount <= 1) {
            // 无线网卡个数为1时，通过NetworkManager的WirelessEnabled属性统一禁用/启用无线网卡
            qDebug() << "[ioctlOperateNetworkLogicalName] wireless count <= 1, using enableWirelessByDBus";
            return enableWirelessByDBus(enable);
        }

        // 无线网卡个数大于1时，通过ioctl方式禁用/启用指定无线网卡
        qDebug() << "[ioctlOperateNetworkLogicalName] wireless count > 1, using enableNetworkByIoctl";
        return enableNetworkByIoctl(logicalName, enable);
    }

    // 有线网卡：通过ioctl方式禁用/启用指定网卡
    qDebug() << "[ioctlOperateNetworkLogicalName] wired card detected, using enableNetworkByIoctl";
    return enableNetworkByIoctl(logicalName, enable);
}

// 通过ioctl禁用/启用指定网卡
bool EnableUtils::enableNetworkByIoctl(const QString &logicalName, bool enable)
{
    qDebug() << "[enableNetworkByIoctl] enter, logicalName:" << logicalName << "enable:" << enable;

    // 安全校验：网卡名长度合法性
    if (logicalName.isEmpty() || logicalName.length() >= IFNAMSIZ) {
        qCritical() << "[enableNetworkByIoctl] Invalid logicalName length for ioctl, length:" << logicalName.length();
        return false;
    }

    // 通过ioctl设置网卡启用/禁用
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        qCritical() << "[enableNetworkByIoctl] Failed to create socket";
        return false;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));

    std::string nameStr = logicalName.toStdString();
    strncpy(ifr.ifr_name, nameStr.c_str(),IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    // 先获取标识
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        qCritical() << "[enableNetworkByIoctl] SIOCGIFFLAGS failed for" << logicalName << "errno:" << errno;
        close(fd);
        return false;
    }

    qDebug() << "[enableNetworkByIoctl] current flags for" << logicalName << ":" << QString::number(ifr.ifr_flags, 2);

    if (enable) {
        ifr.ifr_flags |= IFF_UP;
    } else {
        ifr.ifr_flags &= ~IFF_UP;
    }
    // 设置新标识
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        qCritical() << "[enableNetworkByIoctl] SIOCSIFFLAGS failed for" << logicalName << "errno:" << errno;
        close(fd);
        return false;
    }

    qDebug() << "[enableNetworkByIoctl] success, logicalName:" << logicalName << "enable:" << enable;
    close(fd);
    return true;
}

// 判断网卡是否是无线网卡
bool EnableUtils::isWirelessNetwork(const QString &logicalName)
{
    qDebug() << "[isWirelessNetwork] enter, logicalName:" << logicalName;

    // 1. 通过 system dbus 获取网卡的 dbus path
    QDBusInterface nmInterface("org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.NetworkManager",
                              QDBusConnection::systemBus());
    if (!nmInterface.isValid()) {
        qCritical() << "[isWirelessNetwork] Failed to connect to NetworkManager:" << nmInterface.lastError().message();
        return false;
    }

    QDBusReply<QDBusObjectPath> reply = nmInterface.call("GetDeviceByIpIface", logicalName);
    if (!reply.isValid()) {
        qCritical() << "[isWirelessNetwork] Failed to GetDeviceByIpIface for" << logicalName << ":" << reply.error().message();
        return false;
    }

    QString devicePath = reply.value().path();
    qDebug() << "[isWirelessNetwork] Got devicePath for" << logicalName << ":" << devicePath;
    if (devicePath.isEmpty()) {
        qCritical() << "[isWirelessNetwork] Got empty devicePath for interface:" << logicalName;
        return false;
    }

    // 2. 通过 dbus path 构建 Properties 接口，获取 DeviceType 属性
    QDBusInterface deviceInterface("org.freedesktop.NetworkManager",
                                  devicePath,
                                  "org.freedesktop.DBus.Properties",
                                  QDBusConnection::systemBus());
    if (!deviceInterface.isValid()) {
        qCritical() << "[isWirelessNetwork] Failed to connect to device properties:" << deviceInterface.lastError().message();
        return false;
    }

    QDBusReply<QVariant> typeReply = deviceInterface.call("Get",
                                                          "org.freedesktop.NetworkManager.Device",
                                                          "DeviceType");
    if (!typeReply.isValid()) {
        qCritical() << "[isWirelessNetwork] Failed to get DeviceType for" << logicalName << ":" << typeReply.error().message();
        return false;
    }

    unsigned int deviceType = typeReply.value().toUInt();
    qDebug() << "[isWirelessNetwork] DeviceType for" << logicalName << "=" << deviceType
             << (deviceType == 2 ? "(wireless)" : "(not wireless)");

    // DeviceType 属性值为 2 表示无线网卡
    return deviceType == 2;
}

// 统计当前系统上无线网卡的个数
int EnableUtils::wirelessNetworkCount()
{
    // 1. 通过 system dbus 获取所有网络设备的 dbus path
    QDBusInterface nmInterface("org.freedesktop.NetworkManager",
                              "/org/freedesktop/NetworkManager",
                              "org.freedesktop.NetworkManager",
                              QDBusConnection::systemBus());
    if (!nmInterface.isValid()) {
        qCritical() << "Failed to connect to NetworkManager:" << nmInterface.lastError().message();
        return 0;
    }

    QDBusReply<QList<QDBusObjectPath> > reply = nmInterface.call("GetDevices");
    if (!reply.isValid()) {
        qCritical() << "Failed to GetDevices:" << reply.error().message();
        return 0;
    }

    // 2. 遍历每个设备 dbus path，检查 Managed 和 DeviceType 属性
    int count = 0;
    foreach (const QDBusObjectPath &devicePath, reply.value()) {
        QDBusInterface deviceInterface("org.freedesktop.NetworkManager",
                                      devicePath.path(),
                                      "org.freedesktop.DBus.Properties",
                                      QDBusConnection::systemBus());
        if (!deviceInterface.isValid())
            continue;

        // 先检查 Managed 属性，值为 false 的设备直接跳过
        QDBusReply<QVariant> managedReply = deviceInterface.call("Get",
                                                                  "org.freedesktop.NetworkManager.Device",
                                                                  "Managed");
        if (!managedReply.isValid() || !managedReply.value().toBool())
            continue;

        // Managed 为 true 时，检查 DeviceType 属性，值为 2 表示无线网卡
        QDBusReply<QVariant> typeReply = deviceInterface.call("Get",
                                                              "org.freedesktop.NetworkManager.Device",
                                                              "DeviceType");
        if (typeReply.isValid() && typeReply.value().toUInt() == 2)
            ++count;
    }
    return count;
}

// 系统上仅有1个无线网卡时，通过NetworkManager的WirelessEnabled属性统一禁用/启用无线网卡
bool EnableUtils::enableWirelessByDBus(bool enable)
{
    qDebug() << "[enableWirelessByDBus] enter, enable:" << enable;

    // 通过 system dbus 设置 WirelessEnabled 属性
    QDBusInterface nmInterface("org.freedesktop.NetworkManager",
                               "/org/freedesktop/NetworkManager",
                               "org.freedesktop.NetworkManager",
                               QDBusConnection::systemBus());
    if (!nmInterface.isValid()) {
        qCritical() << "[enableWirelessByDBus] Failed to connect to NetworkManager:" << nmInterface.lastError().message();
        return false;
    }

    // WirelessEnabled属性用于整体禁用/启用系统上的无线网卡
    if (!nmInterface.setProperty("WirelessEnabled", QVariant(enable))) {
        qCritical() << "[enableWirelessByDBus] Failed to set WirelessEnabled:" << nmInterface.lastError().message();
        return false;
    }

    qDebug() << "[enableWirelessByDBus] success, WirelessEnabled set to" << enable;
    return true;
}

bool EnableUtils::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
{
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
