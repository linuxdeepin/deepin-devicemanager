#include "MainJob.h"
#include "ThreadPool.h"
#include "DetectThread.h"
#include "DebugTimeManager.h"
#include "DBusInterface.h"
#include "DriverDBusInterface.h"
#include "DBusEnableInterface.h"
#include "DBusWakeupInterface.h"
#include "DeviceInfoManager.h"
#include "EnableSqlManager.h"
#include "EnableUtils.h"
#include "WakeupUtils.h"
#include "DriverManager.h"

#include <QDateTime>
#include <QThread>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <QDBusConnection>
#include <QDebug>
#include <QFile>

#include <unistd.h>

static QMutex mutex;
const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString DRIVER_SERVICE_PATH = "/com/deepin/drivermanager";
const QString DEVICE_SERVICE_PATH = "/com/deepin/devicemanager";
const QString ENABLE_SERVICE_PATH = "/com/deepin/enablemanager";
const QString WAKEUP_SERVICE_PATH = "/com/deepin/wakeupmanager";
bool  MainJob::s_ServerIsUpdating = false;
bool  MainJob::s_ClientIsUpdating = false;

MainJob::MainJob(QObject *parent)
    : QObject(parent)
    , mp_Pool(new ThreadPool)
    , mp_DetectThread(nullptr)
    , mp_IFace(new DBusInterface(this))
    , mp_DriverOperateIFace(new DriverDBusInterface(this))
    , mp_Enable(new DBusEnableInterface(this))
    , mp_Wakeup(new DBusWakeupInterface(this))
    , m_FirstUpdate(true)
{
    // 守护进程启动的时候加载所有信息
    updateAllDevice();
    //启动时，检测驱动是否要更新，如果要更新则通知系统
    DriverManager *drivermanager = new DriverManager(this);
    if(drivermanager->checkDriverInfo()){
        QString strUsername("");
        QProcess process;
        process.start("sh", QStringList() << "-c" << "who");
        process.waitForFinished(-1);
        strUsername = process.readAll().split(' ')[0];
        qInfo() << strUsername;

        QString strCmd = "runuser -l " + strUsername + " -c \"XDG_RUNTIME_DIR=\"/run/user/$(id -u " + strUsername + " )\" /usr/bin/deepin-devicemanager notify\"";
        process.start("sh", QStringList() << "-c" << strCmd);
        process.waitForFinished(-1);
        qInfo() << process.readAll();
    }

    // 后台加载后先禁用设备
    const QString &info = DeviceInfoManager::getInstance()->getInfo("hwinfo");
    EnableUtils::disableOutDevice(info);
    EnableUtils::disableInDevice();
    WakeupUtils::updateWakeupDeviceInfo(info);
}

MainJob::~MainJob()
{
}

void MainJob::working()
{
    // 启动dbus
    if (!initDBus()) {
        exit(1);
    }

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->start();
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);
    connect(mp_Enable, &DBusEnableInterface::update, this, &MainJob::slotUsbChanged);
    connect(mp_IFace, &DBusInterface::update, this, &MainJob::slotUsbChanged);
    connect(mp_DriverOperateIFace, &DriverDBusInterface::sigFinished, this, &MainJob::slotDriverControl);
}

INSTRUCTION_RES MainJob::executeClientInstruction(const QString &instructions)
{
    QMutexLocker locker(&mutex);
    s_ServerIsUpdating = true;
    INSTRUCTION_RES res = IR_NULL;

    if (instructions.startsWith("DETECT")) {
        // 跟新缓存信息
        updateAllDevice();
    } else if (instructions.startsWith("START")) {
        if (m_FirstUpdate) {
            updateAllDevice();
        }
        res = IR_UPDATE;
    } else {
        res = IR_NULL;
    }

    s_ServerIsUpdating = false;
    return res;
}

bool MainJob::isZhaoXin()
{
    QString cmd = "sudo dmidecode -t 4";
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();
    if (info.contains("ZHAOXIN KaiXian KX-U")) {
        return true;
    } else {
        return false;
    }
}

bool MainJob::serverIsRunning()
{
    return s_ServerIsUpdating;
}

bool MainJob::clientIsRunning()
{
    return s_ClientIsUpdating;
}

void MainJob::slotUsbChanged()
{
    executeClientInstruction("DETECT");
}

void MainJob::slotDriverControl(bool success)
{
    if(success)
        executeClientInstruction("DETECT");
}

void MainJob::onFirstUpdate()
{
    if (m_FirstUpdate) {
        updateAllDevice();
    }
}

void MainJob::updateAllDevice()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateAllDevice()");
    if (m_FirstUpdate)
        mp_Pool->loadDeviceInfo();
    else
        mp_Pool->updateDeviceInfo();
    mp_Pool->waitForDone(-1);
    sleep(1);
    PERF_PRINT_END("POINT-01");
    m_FirstUpdate = false;
}

bool MainJob::initDBus()
{
    QDBusConnection systemBus = QDBusConnection::systemBus();
    //1. 申请一个总线连接
    if (!systemBus.isConnected()) {
        return false;
    }

    //2. 在总线连接上挂载服务，这样其他进程才能请求该服务
    if (!systemBus.registerService(SERVICE_NAME)) {
        return false;
    }

    //3. 在挂载的服务上注册一个执行服务的对象
    if (!systemBus.registerObject(DEVICE_SERVICE_PATH, mp_IFace, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }
    if (!systemBus.registerObject(DRIVER_SERVICE_PATH, mp_DriverOperateIFace, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals))  {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }
    if (!systemBus.registerObject(ENABLE_SERVICE_PATH, mp_Enable, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }
    if (!systemBus.registerObject(WAKEUP_SERVICE_PATH, mp_Wakeup, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }


    static QThread t;
    mp_Enable->moveToThread(&t);
    t.start();

    return true;
}

