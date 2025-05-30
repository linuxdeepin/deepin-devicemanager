// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusEnableInterface.h"
#include "DDLog.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>

using namespace DDLog;

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusEnableInterface *> DBusEnableInterface::s_Instance;
std::mutex DBusEnableInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString ENABLE_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString ENABLE_SERVICE_INTER = "org.deepin.DeviceControl";

DBusEnableInterface::DBusEnableInterface()
    : mp_Iface(nullptr)
{
    qCDebug(appLog) << "DBusEnableInterface constructor";
    // 初始化dbus
    init();
}

bool DBusEnableInterface::getRemoveInfo(QString &info)
{
    qCDebug(appLog) << "Get remove info";
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getRemoveInfo");
    if (reply.isValid()) {
        info = reply.value();
        qCDebug(appLog) << "Remove info:" << info;
        return true;
    } else {
        qCWarning(appLog) << "Invalid DBus reply when getting remove info";
        return false;
    }
}

bool DBusEnableInterface::getAuthorizedInfo(QString &info)
{
    qCDebug(appLog) << "Get authorized info";
    QDBusReply<QString> reply = mp_Iface->call("getAuthorizedInfo");
    if (reply.isValid()) {
        info = reply.value();
        qCDebug(appLog) << "Authorized info:" << info;
        return true;
    } else {
        qCWarning(appLog) << "Invalid DBus reply when getting authorized info";
        info = "";
        return false;
    }
}

bool DBusEnableInterface::isDeviceEnabled(const QString &unique_id)
{
    qCDebug(appLog) << "Check device enabled state for:" << unique_id;
    QDBusReply<bool> reply = mp_Iface->call("isDeviceEnabled", unique_id);
    if (reply.isValid()) {
        bool enabled = reply.value();
        qCDebug(appLog) << "Device enabled state:" << enabled;
        return enabled;
    }
    qCWarning(appLog) << "Invalid DBus reply when checking device enabled state";
    return false;
}

bool DBusEnableInterface::enable(const QString &hclass, const QString &name, const QString &path, const QString &value, bool enable_device, const QString &strDriver)
{
    qCDebug(appLog) << "Enable device - class:" << hclass << "name:" << name << "path:" << path << "value:" << value << "enable:" << enable_device << "driver:" << strDriver;
    QDBusReply<bool> reply = mp_Iface->call("enable", hclass, name, path, value, enable_device, strDriver);
    if (reply.isValid()) {
        bool result = reply.value();
        qCInfo(appLog) << "Device enable operation result:" << result;
        return result;
    }
    qCWarning(appLog) << "Invalid DBus reply when enabling device";
    return false;
}

bool DBusEnableInterface::enablePrinter(const QString &hclass, const QString &name, const QString &path, bool enable_device)
{
    qCDebug(appLog) << "Enable printer - class:" << hclass << "name:" << name << "path:" << path << "enable:" << enable_device;
    QDBusReply<bool> reply = mp_Iface->call("enablePrinter", hclass, name, path, enable_device);
    if (reply.isValid()) {
        bool result = reply.value();
        qCInfo(appLog) << "Printer enable operation result:" << result;
        return result;
    }
    qCWarning(appLog) << "Invalid DBus reply when enabling printer";
    return false;
}

void DBusEnableInterface::init()
{
    qCDebug(appLog) << "Initialize DBus connection";
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        qCCritical(appLog) << "Cannot connect to the D-Bus system bus";
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, ENABLE_SERVICE_PATH, ENABLE_SERVICE_INTER, QDBusConnection::systemBus());
    qCDebug(appLog) << "DBus interface created for service:" << SERVICE_NAME;
}
