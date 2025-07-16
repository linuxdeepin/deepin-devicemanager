// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusWakeupInterface.h"
#include "DDLog.h"
#include <DSysInfo>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QFile>
#include <QMetaMethod>

using namespace DDLog;

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusWakeupInterface *> DBusWakeupInterface::s_Instance;
std::mutex DBusWakeupInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString WAKEUP_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString WAKEUP_INTERFACE = "org.deepin.DeviceControl";

const QString INPUT_SERVICE_NAME_V23 = "org.deepin.dde.InputDevices1";
const QString INPUT_WAKEUP_SERVICE_PATH_V23 = "/org/deepin/dde/InputDevices1";
const QString INPUT_WAKEUP_INTERFACE_V23 = "org.deepin.dde.InputDevices1";

const QString INPUT_SERVICE_NAME_V20 = "com.deepin.system.InputDevices";
const QString INPUT_WAKEUP_SERVICE_PATH_V20 = "/com/deepin/system/InputDevices";
const QString INPUT_WAKEUP_INTERFACE_V20 = "com.deepin.system.InputDevices";

inline bool isV20() { return Dtk::Core::DSysInfo::majorVersion() == "20"; }
const QString INPUT_SERVICE_NAME = isV20() ? INPUT_SERVICE_NAME_V20 : INPUT_SERVICE_NAME_V23;
const QString INPUT_WAKEUP_SERVICE_PATH = isV20() ? INPUT_WAKEUP_SERVICE_PATH_V20 : INPUT_WAKEUP_SERVICE_PATH_V23;
const QString INPUT_WAKEUP_INTERFACE = isV20() ? INPUT_WAKEUP_INTERFACE_V20 : INPUT_WAKEUP_INTERFACE_V23;

const QString INPUT_WAKEUP_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";

DBusWakeupInterface::DBusWakeupInterface()
    : mp_Iface(nullptr), mp_InputIface(nullptr)
{
    qCDebug(appLog) << "DBusWakeupInterface constructor called";
    init();
}

bool DBusWakeupInterface::setWakeupMachine(const QString &unique_id, const QString &path, bool wakeup, const QString &name)
{
    qCDebug(appLog) << "Setting wakeup for device. ID:" << unique_id << "Path:" << path << "Wakeup:" << wakeup << "Name:" << name;

    if (nullptr != mp_InputIface && mp_InputIface->isValid()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStringList pathList = path.split("/", QString::SkipEmptyParts);
#else
        QStringList pathList = path.split("/", Qt::SkipEmptyParts);
#endif
        if (pathList.size() < 3)
            return false;

        qCDebug(appLog) << "Processing PS/2 device";
        if (name.contains("PS/2")) {
            // ps2设备无法通过/sys/devices/platform/i8042/serio1/power/wakeup控制，只能通过acpi的接口进行控制
            QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
            if (interface.isValid()) {
                QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
                QVariant v =  replay.arguments().first();
                if (v.isValid()) {
                    QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                    QMap<QString, QString> allSupportWakeupDevices;
                    arg >> allSupportWakeupDevices;
                    QStringList pathList = allSupportWakeupDevices.keys();

                    for (QString path : pathList) {
                        if (path.contains("PS2")) {
                            mp_InputIface->call("SetWakeupDevices", path, wakeup ? "enabled" : "disabled");
                            return true;
                        }
                    }
                }
            }
        } else {
            qCDebug(appLog) << "Processing non-PS/2 device";
            auto metaObject = mp_InputIface->metaObject();
            for (int i = 0 ; i < metaObject->methodCount(); ++i) {
                if (metaObject->method(i).name() == "SetWakeupDevices") {
                    QString curPath = pathList[pathList.size() - 2];
                    QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                    mp_InputIface->call("SetWakeupDevices", busPath, wakeup ? "enabled" : "disabled");
                    return true;
                }
            }
        }
    }

    qCDebug(appLog) << "Calling DBus setWakeupMachine";
    QDBusReply<bool> reply = mp_Iface->call("setWakeupMachine", unique_id, path, wakeup);
    if (reply.isValid()) {
        qCDebug(appLog) << "setWakeupMachine DBus call successful, returned:" << reply.value();
        return reply.value();
    }
    qCWarning(appLog) << "setWakeupMachine DBus call failed:" << reply.error().message();
    return false;
}

bool DBusWakeupInterface::canInputWakeupMachine(const QString &path)
{
    qCDebug(appLog) << "Checking if input device can wakeup machine. Path:" << path;

    bool inputIfaceValid = (nullptr != mp_InputIface && mp_InputIface->isValid());
    qCDebug(appLog) << "Input interface valid:" << inputIfaceValid;
    if (inputIfaceValid) {
        QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        if (interface.isValid()) {
            QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
            QVariant v =  replay.arguments().first();
            if (v.isValid()) {
                QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                QMap<QString, QString> allSupportWakeupDevices;
                arg >> allSupportWakeupDevices;

                QString curPath = path.left(path.size() - 13);
                int index = curPath.lastIndexOf('/');
                if (index < 1) {
                    return false;
                }
                curPath = curPath.right(curPath.size() - index - 1);
                QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                qCDebug(appLog) << "Checking if wakeup is supported for bus path:" << busPath;
                return allSupportWakeupDevices.contains(busPath);
            }
            qCWarning(appLog) << "Could not get SupportWakeupDevices property from DBus";
        }
    }

    qCDebug(appLog) << "Falling back to file check for wakeup capability";
    QFile file(path);
    bool canOpen = file.open(QIODevice::ReadOnly);
    if (!canOpen) {
        qCWarning(appLog) << "Failed to open wakeup file:" << path;
    }
    return canOpen;
}

bool DBusWakeupInterface::isInputWakeupMachine(const QString &path, const QString &name)
{
    qCDebug(appLog) << "Checking input wakeup state. Path:" << path << "Name:" << name;

    bool inputIfaceValid = (nullptr != mp_InputIface && mp_InputIface->isValid());
    qCDebug(appLog) << "Input interface valid:" << inputIfaceValid;
    if (inputIfaceValid) {
        QDBusInterface interface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_PROPERTIES_INTERFACE, QDBusConnection::systemBus());
        if (interface.isValid()) {
            QDBusMessage replay = interface.call("Get", INPUT_WAKEUP_INTERFACE, "SupportWakeupDevices");
            QVariant v =  replay.arguments().first();
            if (v.isValid()) {
                QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
                QMap<QString, QString> allSupportWakeupDevices;
                arg >> allSupportWakeupDevices;

                if (name.contains("PS/2")) {
                    for(QString path : allSupportWakeupDevices.keys()) {
                        if (path.contains("PS2")) {
                            return allSupportWakeupDevices[path] == "enabled";
                        }
                    }
                } else {
                    QString curPath = path.left(path.size() - 13);
                    int index = curPath.lastIndexOf('/');
                    if (index < 1)
                        return false;
                    curPath = curPath.right(curPath.size() - index - 1);
                    QString busPath = QString("/sys/bus/usb/devices/%1/power/wakeup").arg(curPath);
                    bool isWakeupEnabled = (allSupportWakeupDevices.contains(busPath) && allSupportWakeupDevices[busPath] == "enabled");
                    qCDebug(appLog) << "Wakeup state for bus path" << busPath << "is" << isWakeupEnabled;
                    return isWakeupEnabled;
                }
            }
            qCWarning(appLog) << "Could not get SupportWakeupDevices property from DBus";
        }
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(appLog) << "Failed to open wakeup file:" << path;
        return false;
    }
    QString info = file.readAll();
    return !(info.contains("disabled"));
}

int DBusWakeupInterface::isNetworkWakeup(const QString &logical_name)
{
    QDBusReply<int> reply = mp_Iface->call("isNetworkWakeup", logical_name);
    if (reply.isValid()) {
        qCDebug(appLog) << "isNetworkWakeup returned:" << reply.value();
        return reply.value();
    }
    qCWarning(appLog) << "isNetworkWakeup DBus call failed:" << reply.error().message();
    return -1;
}

bool DBusWakeupInterface::setNetworkWakeup(const QString &logical_name, bool wake)
{
    qCDebug(appLog) << "Calling DBus setNetworkWake";
    QDBusReply<bool> reply = mp_Iface->call("setNetworkWake", logical_name, wake);
    if (reply.isValid()) {
        qCDebug(appLog) << "setNetworkWakeup returned:" << reply.value();
        return reply.value();
    }
    qCWarning(appLog) << "setNetworkWakeup DBus call failed:" << reply.error().message();
    return false;
}


void DBusWakeupInterface::init()
{
    qCDebug(appLog) << "Initializing DBus interfaces";

    // 1. 连接到dbus
    bool isConnected = QDBusConnection::systemBus().isConnected();
    qCDebug(appLog) << "DBus system bus connected:" << isConnected;
    if (!isConnected) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, WAKEUP_SERVICE_PATH, WAKEUP_INTERFACE, QDBusConnection::systemBus());

    // 3. create interface
    mp_InputIface = new QDBusInterface(INPUT_SERVICE_NAME, INPUT_WAKEUP_SERVICE_PATH, INPUT_WAKEUP_INTERFACE, QDBusConnection::systemBus());
}
