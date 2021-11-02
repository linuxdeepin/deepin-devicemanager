#include "DBusDriverInterface.h"

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusDriverInterface *> DBusDriverInterface::s_Instance;
std::mutex DBusDriverInterface::m_mutex;

const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString DRIVER_SERVICE_PATH = "/com/deepin/drivermanager";
const QString DRIVER_INTERFACE = "com.deepin.drivermanager";

void DBusDriverInterface::uninstallDriver(const QString& driver)
{
    QDBusPendingCall async = mp_Iface->asyncCall("unInstallDriver",driver);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher*)));
}

void DBusDriverInterface::installDriver(const QString& driver)
{
    QDBusPendingCall async = mp_Iface->asyncCall("installDriver",driver);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher*)));
}

DBusDriverInterface::DBusDriverInterface(QObject* parent)
    : QObject (parent)
    , mp_Iface(nullptr)
{
    init();
}

DBusDriverInterface::~DBusDriverInterface()
{

}

void DBusDriverInterface::slotProcessChange(qint32 value,QString detail)
{
    emit processChange(value,detail);
}

void DBusDriverInterface::slotCallFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<bool> reply = *watcher;
    if (reply.isError()) {
        emit processEnd(false);
    }else{
        emit processEnd(reply.value());
    }
    watcher->deleteLater();
}

void DBusDriverInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, DRIVER_SERVICE_PATH, DRIVER_INTERFACE, QDBusConnection::systemBus());

    if(mp_Iface->isValid()){
        connect(mp_Iface, SIGNAL(sigProgressDetail(qint32, QString)), this, SLOT(slotProcessChange(qint32, QString)));
    }

}
