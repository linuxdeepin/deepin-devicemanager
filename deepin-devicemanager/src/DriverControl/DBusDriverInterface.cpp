#include "DBusDriverInterface.h"

#include <unistd.h>

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

void DBusDriverInterface::uninstallPrinter(const QString &vendor, const QString &model)
{
    //Sets the timeout in milliseconds for all future DBus calls to timeout. -1 means the default DBus timeout (usually 25 seconds).
    //超时默认是25s，打印卸载超时大概30s
    mp_Iface->setTimeout(1000 * 1000);
    QDBusPendingCall async = mp_Iface->asyncCall("unInstallPrinter", vendor, model);
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

bool DBusDriverInterface::isDriverPackage(const QString &path)
{
    mp_Iface->setTimeout(1000 * 1000);
    QDBusReply<bool> reply = mp_Iface->call("isDriverPackage", path);
    if (reply.isValid())
        return reply.value();
    return false;
}

bool DBusDriverInterface::isArchMatched(const QString &path)
{
    QDBusReply<bool> reply = mp_Iface->call("isArchMatched", path);
    if (reply.isValid())
        return reply.value();
    return false;
}

bool DBusDriverInterface::isDebValid(const QString &path)
{
    QDBusReply<bool> reply = mp_Iface->call("isDebValid", path);
    if (reply.isValid())
        return reply.value();
    return false;
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

void DBusDriverInterface::slotProcessEnd(bool success, QString msg)
{
    if(success){
        emit processChange(100,"");
        usleep(300000);
    }
    qInfo() << "Error msg ******************** " << msg;
    // 将错误码转换为错误信息
    if(m_MapErrMsg.find(msg) != m_MapErrMsg.end()){
        msg = m_MapErrMsg[msg];
    }else{
        msg = tr("Unknown error");
    }
    emit processEnd(success, msg);
}

void DBusDriverInterface::slotCallFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();
}

void DBusDriverInterface::init()
{
    // 初始化错误消息
    m_MapErrMsg.insert("2",tr("The driver module was not found"));           // ENOENT		2	未发现该驱动模块 /* No such file or directory */
    m_MapErrMsg.insert("11",tr("The driver module has dependencies"));       // EAGAIN 	    11	驱动模块被依赖
//    m_MapErrMsg.insert("9",tr(""));                // EBADF		9	/* Bad file number */
//    m_MapErrMsg.insert("17",tr(""));               // EEXIST		17	/* File exists */
//    m_MapErrMsg.insert("19",tr(""));               // ENODEV		19	/* No such device */
//    m_MapErrMsg.insert("30",tr(""));               // EROFS		30	/* Read-only file system */

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
        connect(mp_Iface, SIGNAL(sigFinished(bool, QString)), this, SLOT(slotProcessEnd(bool, QString)));
    }
}
