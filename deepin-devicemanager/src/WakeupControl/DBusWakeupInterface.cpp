#include "DBusWakeupInterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusWakeupInterface *> DBusWakeupInterface::s_Instance;
std::mutex DBusWakeupInterface::m_mutex;

const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString WAKEUP_SERVICE_PATH = "/com/deepin/wakeupmanager";
const QString WAKEUP_INTERFACE = "com.deepin.wakeupmanager";

DBusWakeupInterface::DBusWakeupInterface()
    :mp_Iface(nullptr)
{
    init();
}

bool DBusWakeupInterface::setWakeupMachine(const QString& unique_id, const QString& path, bool wakeup)
{
    QDBusReply<bool> reply = mp_Iface->call("setWakeupMachine", unique_id, path, wakeup);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}

int DBusWakeupInterface::isNetworkWakeup(const QString& logical_name)
{
    QDBusReply<int> reply = mp_Iface->call("isNetworkWakeup", logical_name);
    if (reply.isValid()) {
        return reply.value();
    }
    return -1;
}

bool DBusWakeupInterface::setNetworkWakeup(const QString& logical_name, bool wake)
{
    QDBusReply<bool> reply = mp_Iface->call("setNetworkWake", logical_name, wake);
    if (reply.isValid()) {
        return reply.value();
    }
    return false;
}


void DBusWakeupInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, WAKEUP_SERVICE_PATH, WAKEUP_INTERFACE, QDBusConnection::systemBus());
}
