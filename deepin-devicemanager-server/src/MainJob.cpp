#include "MainJob.h"
#include "ThreadPool.h"
#include "DetectThread.h"
#include "DebugTimeManager.h"
#include "DBusInterface.h"
#include "DriverDBusInterface.h"
#include "DBusEnableInterface.h"
#include "DeviceInfoManager.h"
#include "EnableSqlManager.h"

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
bool  MainJob::s_ServerIsUpdating = false;
bool  MainJob::s_ClientIsUpdating = false;
#define LEAST_NUM 10

MainJob::MainJob(QObject *parent)
    : QObject(parent)
    , mp_Pool(new ThreadPool)
    , mp_DetectThread(nullptr)
    , mp_IFace(new DBusInterface)
    , mp_DriverOperateIFace(new DriverDBusInterface(this))
    , mp_Enable(new DBusEnableInterface())
    , m_FirstUpdate(true)
{
    // 守护进程启动的时候加载所有信息
    if (!isZhaoXin()) {
        updateAllDevice();
    }

    // 后台加载后先禁用设备
    EnableSqlManager::getInstance()->clearEnableFromAuthorizedTable();
    disableDevice();
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

    static QThread t;
    mp_Enable->moveToThread(&t);
    t.start();

    return true;
}

void MainJob::disableDevice()
{
    // 先禁用外设
    const QString &info = DeviceInfoManager::getInstance()->getInfo("hwinfo");
    QStringList items = info.split("\n\n");
    foreach (const QString &item, items) {
        QMap<QString, QString> mapItem;
        if (!getMapInfo(item, mapItem))
            continue;
        // 防止禁用的设备被启用

        QString uniqueID = mapItem["Module Alias"];
        uniqueID.replace(QRegExp("[0-9a-zA-Z]{10}$"), "");
        if (uniqueID.isEmpty()) {
            return;
        }

        QString path = mapItem["SysFS ID"];
        path.replace(QRegExp("[1-9]$"), "0");
        if (EnableSqlManager::getInstance()->uniqueIDExisted(uniqueID)) {
            QFile file("/sys" + path + QString("/authorized"));
            if (!file.open(QIODevice::ReadWrite)) {
                return;
            }
            file.write("0");
            file.close();
            EnableSqlManager::getInstance()->updateDataToAuthorizedTable(uniqueID, path);
        }
    }

    // 禁用非外设
    // 由于rescan会将所有的remove数据都回复，因此需要重新禁用其它设备
    QStringList rpList;
    EnableSqlManager::getInstance()->removePathList(rpList);

    foreach (const QString &path, rpList) {
        QFile file(path + QString("/remove"));
        if (file.open(QIODevice::WriteOnly)) {
            file.write("1");
            file.close();
        }
    }
}

bool MainJob::getMapInfo(const QString &item, QMap<QString, QString> &mapInfo)
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

