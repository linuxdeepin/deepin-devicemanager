// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusAnythingInterface.h"
#include "DDLog.h"

#include <QDir>

using namespace DDLog;

const QString SERVICE_NAME = "com.deepin.anything";
const QString SERVICE_PATH = "/com/deepin/anything";
const QString INTERFACE = "com.deepin.anything";

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusAnythingInterface *> DBusAnythingInterface::s_Instance;
std::mutex DBusAnythingInterface::m_mutex;

DBusAnythingInterface::DBusAnythingInterface()
    : mp_Iface(nullptr)
{
    qCDebug(appLog) << "DBusAnythingInterface constructor called";
    init();
}

void DBusAnythingInterface::init()
{
    qCDebug(appLog) << "Initializing DBusAnythingInterface";

    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        qCritical(appLog) << "Cannot connect to the D-Bus session bus.";
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, SERVICE_PATH, INTERFACE, QDBusConnection::systemBus());
}

bool DBusAnythingInterface::hasLFT(const QString& path)
{
    qCDebug(appLog) << "Checking LFT for path:" << path;

    QDBusReply<bool> reply = mp_Iface->call("hasLFT",path);
    if(reply.isValid()) {
        qCDebug(appLog) << "hasLFT reply is valid, value:" << reply.value();
        return reply.value();
    }
    qCWarning(appLog) << "hasLFT reply is invalid:" << reply.error().message();
    return false;
}

bool DBusAnythingInterface::searchDriver(const QString& sPath, QStringList& lstDriver)
{
    qCDebug(appLog) << "Searching driver in path:" << sPath;

    // anything是根据挂载来创建索引的，/data/home跟/home的挂载地址都是一样的，就只创建了/data的索引
    QString path = sPath;
    if(!hasLFT(path)){
        qCDebug(appLog) << "No LFT found for path:" << path;
        path.prepend("/data");
        if(QDir(path).exists()){
            qCDebug(appLog) << "Prepended /data, new path exists:" << path;
            if(!hasLFT(path)) {
                qCWarning(appLog) << "Still no LFT for path:" << path;
                return false;
            }
        }else{
            qCWarning(appLog) << "Prepended /data, but new path does not exist:" << path;
            return false;
        }
    }

    mp_Iface->setTimeout(1000 * 1000);
    QDBusReply<QStringList> reply = mp_Iface->call("search", path, "^.*(\\.deb|\\.ko)$",true);
    if (reply.isValid()){
        lstDriver = reply.value();
        qCDebug(appLog) << "Driver search successful, found" << lstDriver.size() << "drivers.";
        return true;
    }
    qCWarning(appLog) << "Driver search failed:" << reply.error().message();
    return false;
}
