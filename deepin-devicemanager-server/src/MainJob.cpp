#include "MainJob.h"
#include <QDebug>
#include "DetectJob.h"
#include "ThreadPool.h"
#include "RRServer.h"
#include "DetectThread.h"
#include "DebugTimeManager.h"

#include <unistd.h>
#include <QDateTime>
#include <QThread>
#include <QProcess>

MainJob::MainJob(QObject *parent)
    : QObject(parent)
    , mp_Pool(new ThreadPool)
    , mp_ZmqServer(nullptr)
    , mp_DetectThread(nullptr)
{
}

MainJob::~MainJob()
{
    if (mp_ZmqServer) {
        delete mp_ZmqServer;
        mp_ZmqServer = nullptr;
    }
}

void MainJob::working()
{
    // 启动线程监听客户端询问
    mp_ZmqServer = new RRServer(this);
    char ch[] = "tcp://127.0.0.1:8700";
    bool suc = mp_ZmqServer->initTo(ch);
    qDebug() << "Bind to tcp://127.0.0.1:8700 ************ " << suc;
    mp_ZmqServer->start();
    connect(mp_ZmqServer, &RRServer::instruction, this, &MainJob::slotExecuteClientInstructions);

    // 启动线程监听USB是否有新的设备
    mp_DetectThread = new DetectThread(this);
    mp_DetectThread->start();
    connect(mp_DetectThread, &DetectThread::usbChanged, this, &MainJob::slotUsbChanged, Qt::ConnectionType::QueuedConnection);
}

void MainJob::slotUsbChanged()
{
    handleInstruction("DETECT");
}

void MainJob::slotExecuteClientInstructions(const QString &instructions)
{
    handleInstruction("ZMQ#" + instructions);
}

void MainJob::handleInstruction(const QString &instruction)
{
    if (instruction.startsWith("DETECT")) {
        updateAllDevice();
    } else if (instruction.startsWith("ZMQ")) {
        if (instruction.startsWith("ZMQ#UPDATE")) {
            updateMonitor();
        } else if (instruction.startsWith("ZMQ#DRIVER")) {
            driverInstruction(instruction);
        } else if (instruction.startsWith("ZMQ#IFCONFIG")) {
            ifconfigInstruction(instruction);
        } else if (instruction.startsWith("ZMQ#UNINSTALL")) {

        } else {
            nullInstruction();
        }
    }
}

void MainJob::updateAllDevice()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateAllDevice()");
    mp_Pool->generateDeviceFile();
    mp_Pool->waitForDone(-1);
    PERF_PRINT_END("POINT-01");
}

void MainJob::updateMonitor()
{
    PERF_PRINT_BEGIN("POINT-01", "MainJob::updateMonitor()");
    mp_Pool->generateMonitor();
    mp_Pool->waitForDone(-1);
    mp_ZmqServer->setReturnStr("1");
    PERF_PRINT_END("POINT-01");

    // 停止3秒钟的时间
//    sleep(3);
}

void MainJob::nullInstruction()
{
    PERF_PRINT_BEGIN("POINT-02", "MainJob::nullInstruction()");
    mp_ZmqServer->setReturnStr("0");
    PERF_PRINT_END("POINT-02");
}

void MainJob::driverInstruction(const QString &instruction)
{
    PERF_PRINT_BEGIN("POINT-03", "MainJob::driverInstruction()");
    QStringList lst = instruction.split("#");
    if (lst.size() != 3) {
        mp_ZmqServer->setReturnStr("0");
    }
    const QString &cmd = lst[2];
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        mp_ZmqServer->setReturnStr("0");
    } else {
        QString output = process.readAllStandardOutput();
        if (output == "") {
            mp_ZmqServer->setReturnStr("2");
        } else {
            mp_ZmqServer->setReturnStr("1");
        }
    }

    PERF_PRINT_END("POINT-03");
}

void MainJob::ifconfigInstruction(const QString &instruction)
{
    PERF_PRINT_BEGIN("POINT-04", "MainJob::ifconfigInstruction()");
    QStringList lst = instruction.split("#");
    if (lst.size() != 3) {
        mp_ZmqServer->setReturnStr("0");
    }
    const QString &cmd = lst[2];
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    int exitcode = process.exitCode();
    if (exitcode == 127 || exitcode == 126) {
        mp_ZmqServer->setReturnStr("0");
    } else {
        QString output = process.readAllStandardOutput();
        if (output == "") {
            mp_ZmqServer->setReturnStr("2");
        } else {
            mp_ZmqServer->setReturnStr("1");
        }
    }

    PERF_PRINT_END("POINT-04");
}
