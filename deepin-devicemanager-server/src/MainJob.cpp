#include "MainJob.h"
#include "ThreadPool.h"
#include "DetectThread.h"
#include "DebugTimeManager.h"
#include "DBusInterface.h"
#include "DeviceInfoManager.h"

#include <unistd.h>
#include <QDateTime>
#include <QThread>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <QDBusConnection>
#include <QDebug>

static QMutex mutex;
const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString SERVICE_PATH = "/com/deepin/devicemanager";

MainJob::MainJob(QObject *parent)
    : QObject(parent)
    , mp_Pool(new ThreadPool)
    , mp_DetectThread(nullptr)
    , mp_IFace(new DBusInterface)
    , m_ClientIsUpdating(false)
    , m_ServerIsUpdating(false)
    , m_FirstUpdate(true)
{
    // 守护进程启动的时候加载所有信息
    if (!isZhaoXin()) {
        updateAllDevice();
    }
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
    mp_IFace->setMainJob(this);

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->start();
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);
}

INSTRUCTION_RES MainJob::executeClientInstruction(const QString &instructions)
{
    QMutexLocker locker(&mutex);
    m_ServerIsUpdating = true;
    INSTRUCTION_RES res = IR_NULL;

    if (instructions.startsWith("DETECT")) {
        // 跟新缓存信息
        updateAllDevice();
    } else if (instructions.startsWith("START")) {
        if (m_FirstUpdate) {
            updateAllDevice();
        }
        res = IR_UPDATE;
    } else if (instructions.startsWith("DRIVER")) {
        // 执行启用禁用的驱动指令
        res = driverInstruction(instructions);
    } else if (instructions.startsWith("IFCONFIG")) {
        // 执行ifconfig指令
        res = ifconfigInstruction(instructions);
    } else {
        res = IR_NULL;
    }

    m_ServerIsUpdating = false;
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

bool MainJob::isServerRunning()
{
    return m_ServerIsUpdating;
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

INSTRUCTION_RES MainJob::driverInstruction(const QString &instruction)
{
    qInfo() << instruction;
    QStringList lst = instruction.split("#");
    if (lst.size() != 2) {
        return IR_NULL;
    }
    const QString &cmd = lst[1];

    QStringList lstCmd = cmd.split(" ");
    if (lstCmd.size() != 2) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }

    qInfo() << lstCmd[0] << " " <<lstCmd[1];
    //检查输入，防止代码注入
    if (("insmod" != lstCmd[0]) && ("rmmod" != lstCmd[0])) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }

    if (("insmod" == lstCmd[0]) && (!lstCmd[1].contains("ko"))) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }

    if ((lstCmd[1].contains("|")) || (lstCmd[1].contains("||"))
            || (lstCmd[1].contains("&")) || (lstCmd[1].contains("&&"))) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }

    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        return IR_NULL;
    } else {
        QString output = process.readAllStandardOutput();
        if (output == "") {
            return IR_SUCCESS;
        } else {
            return IR_FAILED;
        }
    }
}

INSTRUCTION_RES MainJob::ifconfigInstruction(const QString &instruction)
{
    QStringList lst = instruction.split("#");
    if (lst.size() != 2) {
        return IR_NULL;
    }
    const QString &cmd = lst[1];//ifconfig %1 up ifconfig %1 down
    QStringList lstCmd = cmd.split(" ");
    if (lstCmd.size() != 3) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }
    if (("ifconfig" != lstCmd[0]) || ("up" != lstCmd[2] && "down" != lstCmd[2])) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }

    QRegExp reg("\\w{1,15}");//接口名称限制，长度1-15.
    if (!reg.exactMatch(lstCmd[1])) {
        qInfo() << "Cmd format is wrong.";
        return IR_NULL;
    }
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        return IR_NULL;
    } else {
        QString output = process.readAllStandardOutput();
        if (output == "") {
            return IR_SUCCESS;
        } else {
            return IR_FAILED;
        }
    }
}

bool MainJob::initDBus()
{
    //1. 申请一个总线连接
    if (!QDBusConnection::systemBus().isConnected()) {
        return false;
    }

    //2. 在总线连接上挂载服务，这样其他进程才能请求该服务
    if (!QDBusConnection::systemBus().registerService(SERVICE_NAME)) {
        return false;
    }

    //3. 在挂载的服务上注册一个执行服务的对象
    if (!QDBusConnection::systemBus().registerObject(SERVICE_PATH, mp_IFace, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qInfo() << QDBusConnection::systemBus().lastError();
        return false;
    }

    return true;
}
