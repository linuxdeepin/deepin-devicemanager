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
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QDBusObjectPath>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QDebug>
#include <QTimer>

#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define LEAST_NUM 10
#define REG_ADDRESS "^[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}:[0-9a-z]{2}$"

// 读取sysfs文件内容，读取失败返回空字符串
static QString readSysfsContent(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromLatin1(file.readAll()).trimmed();
}

// 写入sysfs文件内容
static bool writeSysfsContent(const QString &path, const QString &content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    const QByteArray data = content.toLatin1();
    return file.write(data) == data.size();
}

// 校验网卡名格式：仅允许字母、数字、下划线、短横线、点号，长度不超过IFNAMSIZ-1，
// 禁止路径分隔符和".."等，防止将外部可控参数拼入sysfs路径时发生路径穿越
static bool isValidInterfaceName(const QString &name)
{
    if (name.isEmpty() || name.length() >= IFNAMSIZ)
        return false;
    static const QRegularExpression re("^[a-zA-Z0-9._-]+$");
    return re.match(name).hasMatch() && !name.contains("..");
}

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


        // 网卡采用ioctl的方式禁用（链路二：开机/服务唤起时恢复禁用状态）
        QRegExp reg(REG_ADDRESS);
        if (reg.exactMatch(uniqueID)) {
            path = mapItem["Device File"];
            if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID) &&
                    EnableUtils::ioctlOperateNetworkLogicalName(path, false, NOS_BOOT_RESTORE))
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
            // 链路二：开机/服务唤起时恢复禁用状态
            EnableUtils::ioctlOperateNetworkLogicalName((*it).first, false, NOS_BOOT_RESTORE);
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

bool EnableUtils::ioctlOperateNetworkLogicalName(const QString &logicalName, bool enable, NetworkOperateSource source)
{
    const QString nmServiceName = "org.freedesktop.NetworkManager";

    // 1. 首先判断NetworkManager服务是否已经启动
    QDBusConnectionInterface *busInterface = QDBusConnection::systemBus().interface();
    if (busInterface == nullptr) {
        // 无法获取系统总线接口时既无法确认服务状态也无法监控，直接执行当前逻辑
        // （其内部对NetworkManager的调用失败时会按有线网卡走ioctl方式处理）
        qCritical() << "[ioctlOperateNetworkLogicalName] system bus interface is invalid, executing directly";
        return ioctlOperateNetworkLogicalNameImpl(logicalName, enable, source);
    }

    // 2. 服务已启动，直接调用当前的启用/禁用逻辑
    if (busInterface->isServiceRegistered(nmServiceName)) {
        qDebug() << "[ioctlOperateNetworkLogicalName] NetworkManager is running, executing directly";
        return ioctlOperateNetworkLogicalNameImpl(logicalName, enable, source);
    }

    // 3. 服务未启动：监控该服务，待其启动后再等待3秒执行当前逻辑
    qDebug() << "[ioctlOperateNetworkLogicalName] NetworkManager is not running,"
             << "watching for service start, will execute 3s after it starts";

    // 先创建服务监控，避免"检查未启动"与"监控生效"之间服务恰好启动而错过启动信号
    QDBusServiceWatcher *watcher = new QDBusServiceWatcher(nmServiceName,
                                                           QDBusConnection::systemBus(),
                                                           QDBusServiceWatcher::WatchForRegistration);
    QObject::connect(watcher, &QDBusServiceWatcher::serviceRegistered, watcher,
                     [watcher, logicalName, enable, source](const QString &serviceName) {
        qDebug() << "[ioctlOperateNetworkLogicalName] NetworkManager started:" << serviceName
                 << ", waiting 3 seconds before executing";

        // 服务启动后等待3秒再执行，给NetworkManager留出枚举并初始化网卡设备信息的时间
        QTimer::singleShot(3000, watcher, [watcher, logicalName, enable, source]() {
            watcher->deleteLater();
            EnableUtils::ioctlOperateNetworkLogicalNameImpl(logicalName, enable, source);
        });
    });

    // 再次检查，防止服务在上述窗口内恰好启动：
    // 若监控未收到启动信号，此处改为直接同步执行；
    // 若监控的信号已在事件队列中，watcher销毁后其延迟执行会随之取消，不会重复执行
    if (busInterface->isServiceRegistered(nmServiceName)) {
        qDebug() << "[ioctlOperateNetworkLogicalName] second check: NetworkManager is now registered,"
                 << "cancelling watcher and executing directly";
        watcher->deleteLater();
        return ioctlOperateNetworkLogicalNameImpl(logicalName, enable, source);
    }

    // 操作已转为异步挂起，无法同步返回执行结果：
    // 返回true表示操作已被接受，将在NetworkManager服务启动后延迟3秒执行
    qDebug() << "[ioctlOperateNetworkLogicalName] operation is pending asynchronously,"
             << "will execute 3s after NetworkManager starts";
    return true;
}

// 实际执行网卡启用/禁用逻辑，由ioctlOperateNetworkLogicalName在NetworkManager服务就绪后调用
bool EnableUtils::ioctlOperateNetworkLogicalNameImpl(const QString &logicalName, bool enable, NetworkOperateSource source)
{
    qDebug() << "[ioctlOperateNetworkLogicalName] enter, logicalName:" << logicalName
             << "enable:" << enable
             << "source:" << (source == NOS_UI_OPERATE ? "UI" : "BootRestore");

    // 安全校验：网卡名格式校验，防止路径穿越（logicalName来自DBus调用者，服务以root运行）
    if (!isValidInterfaceName(logicalName)) {
        qCritical() << "[ioctlOperateNetworkLogicalName] invalid logical name, rejecting:" << logicalName;
        return false;
    }

    // 首先判断当前网卡是有线网卡还是无线网卡
    if (isWirelessNetwork(logicalName)) {
        // 无线网卡：需要判断当前系统上无线网卡的个数
        int wirelessCount = wirelessNetworkCount();
        qDebug() << "[ioctlOperateNetworkLogicalName] wireless card detected, wireless count:" << wirelessCount;
        if (wirelessCount <= 1) {
            // 无线网卡个数为1时，通过rfkill直接block/unblock当前无线网卡
            qDebug() << "[ioctlOperateNetworkLogicalName] wireless count <= 1, using enableWirelessByRfkill";
            return enableWirelessByRfkill(logicalName, enable);
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

    // 安全校验：网卡名格式合法性（长度+字符集，防止ioctl注入和路径穿越）
    if (!isValidInterfaceName(logicalName)) {
        qCritical() << "[enableNetworkByIoctl] Invalid logicalName, rejected:" << logicalName;
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

// 无线网卡个数为1时，通过rfkill直接block/unblock当前无线网卡
bool EnableUtils::enableWirelessByRfkill(const QString &logicalName, bool enable)
{
    qDebug() << "[enableWirelessByRfkill] enter, logicalName:" << logicalName << "enable:" << enable;

    // 禁用时block当前无线网卡（射频下电），启用时unblock
    if (!setRfkillBlocked(logicalName, !enable)) {
        qCritical() << "[enableWirelessByRfkill] failed to" << (enable ? "unblock" : "block") << "wireless card:" << logicalName;
        return false;
    }

    qDebug() << "[enableWirelessByRfkill] success, logicalName:" << logicalName << "enable:" << enable;
    return true;
}

// 通过rfkill block/unblock指定无线网卡
bool EnableUtils::setRfkillBlocked(const QString &interfaceName, bool blocked)
{
    qDebug() << "[setRfkillBlocked] enter, interfaceName:" << interfaceName << "blocked:" << blocked;

    // 安全校验：网卡名格式校验，防止路径穿越（interfaceName来自DBus调用者，服务以root运行，
    // 若包含"../"等可逃逸/sys/class/net/读取任意文件）
    if (!isValidInterfaceName(interfaceName)) {
        qCritical() << "[setRfkillBlocked] invalid interface name, rejecting to prevent sysfs path traversal:" << interfaceName;
        return false;
    }

    // 仅无线网卡支持rfkill：读取 /sys/class/net/<iface>/phy80211/name 得到phy名称
    const QString phyName = readSysfsContent(QString("/sys/class/net/%1/phy80211/name").arg(interfaceName));
    if (phyName.isEmpty()) {
        // 非无线网卡或sysfs读取失败，操作失败
        qCritical() << "[setRfkillBlocked] no phy80211 info for" << interfaceName << ", not a wireless card or sysfs read failed";
        return false;
    }
    qDebug() << "[setRfkillBlocked] phy name for" << interfaceName << ":" << phyName;

    // 在 /sys/class/rfkill/ 下查找 type=wlan 且 name=phy 的rfkill设备
    const QDir rfkillDir("/sys/class/rfkill");
    const QStringList entries = rfkillDir.entryList(QStringList() << "rfkill*", QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        const QString base = rfkillDir.absoluteFilePath(entry);
        if (readSysfsContent(base + "/type") != "wlan")
            continue;
        if (readSysfsContent(base + "/name") != phyName)
            continue;

        // 找到当前网卡对应的rfkill设备，写soft文件：1表示block，0表示unblock
        const QString softPath = base + "/soft";
        if (!writeSysfsContent(softPath, blocked ? "1" : "0")) {
            qCritical() << "[setRfkillBlocked] failed to write" << softPath;
            return false;
        }

        qDebug() << "[setRfkillBlocked] success, wrote" << softPath << "with" << (blocked ? "1" : "0");
        return true;
    }

    // 未找到对应的rfkill设备
    qWarning() << "[setRfkillBlocked] rfkill device not found for" << interfaceName << "phy" << phyName;
    return false;
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
