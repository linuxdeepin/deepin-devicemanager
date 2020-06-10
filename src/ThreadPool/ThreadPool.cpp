#include "ThreadPool.h"
#include<QDebug>
#include<QMutex>
#include<QDateTime>
#include <unistd.h>
#include <QObjectCleanupHandler>
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"
#include "LoadInfo/CmdTool.h"
#include "LoadInfo/DeviceGenerator.h"
#include "LoadInfo/DeviceFactory.h"

CmdTask::CmdTask(QString key, QString cmd, QString debugFile, QString info, ThreadPool *parent)
    : QObject (parent), m_Key(key), m_Cmd(cmd), m_DebugFile(debugFile), m_Info(info), mp_Parent(parent)
{
    this->setAutoDelete(true);
}
CmdTask::~CmdTask()
{

}
void CmdTask::run()
{
    CmdTool tool;
    tool.loadCmdInfo(m_Key, m_Cmd, m_DebugFile);
    const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo = tool.cmdInfo();
    mp_Parent->finishedCmd(m_Info, cmdInfo);
}


GenerateTask::GenerateTask(DeviceType deviceType, ThreadPool *parent)
    : QObject(parent), m_Type(deviceType), mp_Parent(parent)
{

}
GenerateTask::~GenerateTask()
{

}
void GenerateTask::run()
{

    DeviceGenerator *generator = DeviceFactory::getDeviceGenerator();

    if (!generator) {
        return;
    }
    switch (m_Type) {
    case DT_Computer:
        generator->generatorComputerDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Cpu:
        generator->generatorCpuDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Bios:
        generator->generatorBiosDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Memory:
        generator->generatorMemoryDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Storage:
        generator->generatorDiskDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Gpu:
        generator->generatorGpuDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Monitor:
        generator->generatorMonitorDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Network:
        generator->generatorNetworkDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Audio:
        generator->generatorAudioDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Bluetoorh:
        generator->generatorBluetoothDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Keyboard:
        generator->generatorKeyboardDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Mouse:
        generator->generatorMouseDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Print:
        generator->generatorPrinterDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Image:
        generator->generatorCameraDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Cdrom:
        generator->generatorCdromDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Others:
        generator->generatorOthersDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    case DT_Power:
        generator->generatorPowerDevice();
        mp_Parent->finishedGenerateDevice(generator->getBusIDFromHwinfo());
        break;
    default:
        break;
    }

    delete generator;
    generator = nullptr;
}


ThreadPool::ThreadPool(QObject *parent) : QThreadPool(parent)
{

}

void ThreadPool::finishedCmd(const QString &info, const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo)
{
    m_lock.tryLock();
    m_FinishedCmd++;
    DeviceManager::instance()->addCmdInfo(cmdInfo);
    m_lock.unlock();
    if (m_FinishedCmd == m_AllCmdNum) {
        generateInfo();
    } else {
        if (!info.isEmpty())
            emit finished(info);
    }
}

void ThreadPool::finishedGenerateDevice(const QStringList &lst)
{
    m_lock.tryLock();
    m_FinishedGenerator++;
    if (lst.size() > 0) {
        DeviceManager::instance()->addBusId(lst);
    }
    if (m_FinishedGenerator == m_AllTypeNum) {
        emit finished("finish");
    }
    m_lock.unlock();
}

void ThreadPool::loadCmdInfo()
{
    // 初始化信息
    DeviceManager::instance()->clear();
    m_FinishedCmd = 0;
    m_FinishedGenerator = 0;

    // 开始线程池处理
    QList<QStringList> lstCmd;
    getCmdList(lstCmd);
    QList<QStringList>::iterator it = lstCmd.begin();
//    QObjectCleanupHandler *cleaner = new QObjectCleanupHandler;
    for (; it != lstCmd.end(); ++it) {
        CmdTask *task = new CmdTask((*it)[0], (*it)[1], (*it)[2], (*it)[3], this);
        start(task);

//        cleaner->add(task);
    }

//    cleaner->deleteLater();
}


void ThreadPool::generateInfo()
{
    // 开始生成设备信息
    QList<DeviceType> typeList;
    getTypeList(typeList);
    QList<DeviceType>::iterator it = typeList.begin();
    for (; it != typeList.end(); ++it) {
        if (*it == DT_Others) {  // 这里是为了确保所有设备执行完毕后，生成其它设备
            break;
        }
        start(new GenerateTask(*it, this));
    }

    // 这里是为了确保所有设备执行完毕后，生成其它设备
    qint64 beginMSecond = QDateTime::currentMSecsSinceEpoch();
    while (true) {
        qint64 curMSecond = QDateTime::currentMSecsSinceEpoch();
        if (m_FinishedGenerator == m_AllTypeNum - 1  || curMSecond - beginMSecond > 4000) {
            start(new GenerateTask(DT_Others, this));
            break;
        }
    }
}


void ThreadPool::getCmdList(QList<QStringList> &cmdList)
{
    cmdList.append({ "lshw",                 "sudo lshw",               "lshw.txt",               tr("Loading Audio Device Info...") });
    cmdList.append({ "printer",              "",                "printer.txt",            ""});

    cmdList.append({ "dmidecode0",           "sudo dmidecode -t 0",     "dmidecode_0.txt",        tr("Loading BIOS Info...")});
    cmdList.append({ "dmidecode1",           "sudo dmidecode -t 1",     "dmidecode_1.txt",        ""});
    cmdList.append({ "dmidecode2",           "sudo dmidecode -t 2",     "dmidecode_2.txt",        ""});
    cmdList.append({ "dmidecode3",           "sudo dmidecode -t 3",     "dmidecode_3.txt",        ""});
    cmdList.append({ "dmidecode4",           "sudo dmidecode -t 4",     "dmidecode_4.txt",        ""});
    cmdList.append({ "dmidecode13",          "sudo dmidecode -t 13",    "dmidecode_13.txt",       ""});
    cmdList.append({ "dmidecode16",          "sudo dmidecode -t 16",    "dmidecode_16.txt",       ""});
    cmdList.append({ "dmidecode17",          "sudo dmidecode -t 17",    "dmidecode_17.txt",       ""});


    cmdList.append({ "hwinfo_monitor",       "hwinfo --monitor",        "hwinfo_monitor.txt",     tr("Loading CD-ROM Info...")});
    cmdList.append({ "hwinfo_sound",         "hwinfo --sound",          "hwinfo_sound.txt",       ""});
    cmdList.append({ "hwinfo_usb",           "hwinfo --usb",            "hwinfo_usb.txt",         ""});
    cmdList.append({ "hwinfo_network",       "hwinfo --network",        "hwinfo_network.txt",     ""});
    cmdList.append({ "hwinfo_keyboard",      "hwinfo --keyboard",       "hwinfo_keyboard.txt",    tr("Loading Bluetooth Device Info...")});
    cmdList.append({ "hwinfo_cdrom",         "hwinfo --cdrom",          "hwinfo_cdrom.txt",       tr("Loading Image Devices Info...")});
    cmdList.append({ "hwinfo_disk",          "sudo hwinfo --disk",      "hwinfo_disk.txt",        tr("Loading Keyboard Info...")});
    cmdList.append({ "hwinfo_display",       "hwinfo --display",        "hwinfo_display.txt",     ""});
    cmdList.append({ "hwinfo_mouse",         "hwinfo --mouse",          "hwinfo_mouse.txt",       ""});


    cmdList.append({ "upower",               "upower --dump",           "upower_dump.txt",        ""});
    cmdList.append({ "lscpu",                "lscpu",                   "lscpu.txt",              tr("Loading Operating System Info...")});
    cmdList.append({ "lsblk_d",              "lsblk -d -o name,rota",   "lsblk_d.txt",            ""});
    cmdList.append({ "xrandr",               "xrandr",                  "xrandr.txt",             tr("Loading CPU Info...")});
    cmdList.append({ "xrandr_verbose",       "xrandr --verbose",        "xrandr_verbose.txt",     tr("Loading Other Devices Info...")});
    cmdList.append({ "dmesg",                "sudo dmesg",              "dmesg.txt",              tr("Loading Power Info...")});
    cmdList.append({ "hciconfig",            "hciconfig -a",            "hciconfig.txt",          tr("Loading Printer Info...")});
    cmdList.append({ "gpuinfo",              "gpuinfo",                 "gpuinfo.txt",            ""});

    cmdList.append({ "cat_cpuinfo",          "cat /proc/cpuinfo",       "cat_cpuinfo.txt",        tr("Loading Monitor Info...")});
    cmdList.append({ "cat_boardinfo",        "cat /proc/boardinfo",     "cat_boardinfo.txt",      tr("Loading Mouse Info...")});
    cmdList.append({ "cat_os_release",       "cat /etc/os-release",     "cat_os_release.txt",     tr("Loading Network Adapter Info...")});
    cmdList.append({ "cat_version",          "cat /proc/version",       "cat_version.txt",        ""});
    cmdList.append({ "cat_devices",          "cat /proc/bus/input/devices", "cat_devices.txt",     ""});
    cmdList.append({ "cat_audio",            "cat /proc/asound/card0/codec#0", "cat_audio.txt",     ""});
    cmdList.append({ "EDID_HDMI",            "hexdump /sys/devices/platform/hisi-drm/drm/card0/card0-HDMI-A-1/edid", "EDID_HDMI.txt",     ""});
    cmdList.append({ "EDID_VGA",             "hexdump /sys/devices/platform/hisi-drm/drm/card0/card0-VGA-1/edid", "EDID_VGA.txt",     ""});
    m_AllCmdNum = cmdList.size();
}

void ThreadPool::getTypeList(QList<DeviceType> &typeList)
{
    typeList.push_back(DT_Bluetoorh);
    typeList.push_back(DT_Keyboard);
    typeList.push_back(DT_Mouse);
    typeList.push_back(DT_Image);
    typeList.push_back(DT_Storage);
    typeList.push_back(DT_Computer);
    typeList.push_back(DT_Cpu);
    typeList.push_back(DT_Bios);
    typeList.push_back(DT_Memory);
    typeList.push_back(DT_Gpu);
    typeList.push_back(DT_Monitor);
    typeList.push_back(DT_Network);
    typeList.push_back(DT_Audio);
    typeList.push_back(DT_Print);
    typeList.push_back(DT_Cdrom);
    typeList.push_back(DT_Power);
    typeList.push_back(DT_Others);
    m_AllTypeNum = typeList.size();
}

