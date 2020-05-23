#include "ThreadPool.h"
#include<QDebug>
#include <unistd.h>
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"
#include "LoadInfo/CmdTool.h"
#include "LoadInfo/DeviceGenerator.h"
#include "LoadInfo/DeviceFactory.h"

CmdTask::CmdTask(QString key, QString cmd, QString debugFile, QString info, ThreadPool *parent)
    : m_Key(key), m_Cmd(cmd), m_DebugFile(debugFile), m_Info(info), mp_Parent(parent)
{

}
CmdTask::~CmdTask()
{

}
void CmdTask::run()
{
    CmdTool tool;
    tool.loadCmdInfo(m_Key, m_Cmd, m_DebugFile);
    mp_Parent->finishedCmd(m_Info);
}


GenerateTask::GenerateTask(DeviceType deviceType, ThreadPool *parent)
    : m_Type(deviceType), mp_Parent(parent)
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
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Cpu:
        generator->generatorCpuDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Bios:
        generator->generatorBiosDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Memory:
        generator->generatorMemoryDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Storage:
        generator->generatorDiskDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Gpu:
        generator->generatorGpuDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Monitor:
        generator->generatorMonitorDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Network:
        generator->generatorNetworkDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Audio:
        generator->generatorAudioDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Bluetoorh:
        generator->generatorBluetoothDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Keyboard:
        generator->generatorKeyboardDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Mouse:
        generator->generatorMouseDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Print:
        generator->generatorPrinterDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Image:
        generator->generatorCameraDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Cdrom:
        generator->generatorCdromDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Others:
        generator->generatorOthersDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Power:
        generator->generatorPowerDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    default:
        break;
    }
}


ThreadPool::ThreadPool(QObject *parent) : QThreadPool(parent)
{
    m_ListDeviceTypes.push_back(DT_Computer);
    m_ListDeviceTypes.push_back(DT_Cpu);
    m_ListDeviceTypes.push_back(DT_Bios);
    m_ListDeviceTypes.push_back(DT_Memory);
    m_ListDeviceTypes.push_back(DT_Storage);
    m_ListDeviceTypes.push_back(DT_Gpu);
    m_ListDeviceTypes.push_back(DT_Monitor);
    m_ListDeviceTypes.push_back(DT_Network);
    m_ListDeviceTypes.push_back(DT_Audio);
    m_ListDeviceTypes.push_back(DT_Bluetoorh);
    m_ListDeviceTypes.push_back(DT_Keyboard);
    m_ListDeviceTypes.push_back(DT_Mouse);
    m_ListDeviceTypes.push_back(DT_Print);
    m_ListDeviceTypes.push_back(DT_Image);
    m_ListDeviceTypes.push_back(DT_Cdrom);
    m_ListDeviceTypes.push_back(DT_Others);
    m_ListDeviceTypes.push_back(DT_Power);



    m_ListCmd.append({ "lshw",                 "sudo lshw",               "lshw.txt",               tr("Loading Audio Device Info...") });
    m_ListCmd.append({ "printer",              "********",                "printer.txt",            ""});

    m_ListCmd.append({ "dmidecode0",           "sudo dmidecode -t 0",     "dmidecode_0.txt",        tr("Loading BIOS Info...")});
    m_ListCmd.append({ "dmidecode1",           "sudo dmidecode -t 1",     "dmidecode_1.txt",        ""});
    m_ListCmd.append({ "dmidecode2",           "sudo dmidecode -t 2",     "dmidecode_2.txt",        ""});
    m_ListCmd.append({ "dmidecode3",           "sudo dmidecode -t 3",     "dmidecode_3.txt",        ""});
    m_ListCmd.append({ "dmidecode4",           "sudo dmidecode -t 4",     "dmidecode_4.txt",        ""});
    m_ListCmd.append({ "dmidecode16",          "sudo dmidecode -t 16",    "dmidecode_16.txt",       ""});
    m_ListCmd.append({ "dmidecode17",          "sudo dmidecode -t 17",    "dmidecode_17.txt",       ""});


    m_ListCmd.append({ "hwinfo_monitor",       "hwinfo --monitor",        "hwinfo_monitor.txt",     tr("Loading CD-ROM Info...")});
    m_ListCmd.append({ "hwinfo_sound",         "hwinfo --sound",          "hwinfo_sound.txt",       ""});
    m_ListCmd.append({ "hwinfo_usb",           "hwinfo --usb",            "hwinfo_usb.txt",         ""});
    m_ListCmd.append({ "hwinfo_network",       "hwinfo --network",        "hwinfo_network.txt",     ""});
    m_ListCmd.append({ "hwinfo_keyboard",      "hwinfo --keyboard",       "hwinfo_keyboard.txt",    tr("Loading Bluetooth Device Info...")});
    m_ListCmd.append({ "hwinfo_cdrom",         "hwinfo --cdrom",          "hwinfo_cdrom.txt",       tr("Loading Image Devices Info...")});
    m_ListCmd.append({ "hwinfo_disk",          "sudo hwinfo --disk",      "hwinfo_disk.txt",        tr("Loading Keyboard Info...")});
    m_ListCmd.append({ "hwinfo_display",       "hwinfo --display",        "hwinfo_display.txt",     ""});
    m_ListCmd.append({ "hwinfo_mouse",         "hwinfo --mouse",          "hwinfo_mouse.txt",       ""});


    m_ListCmd.append({ "upower",               "upower --dump",           "upower_dump.txt",        ""});
    m_ListCmd.append({ "lscpu",                "lscpu",                   "lscpu.txt",              tr("Loading Operating System Info...")});
    m_ListCmd.append({ "lsblk_d",              "lsblk -d -o name,rota",   "lsblk_d.txt",            ""});
    m_ListCmd.append({ "xrandr",               "xrandr",                  "xrandr.txt",             tr("Loading CPU Info...")});
    m_ListCmd.append({ "xrandr_verbose",       "xrandr --verbose",        "xrandr_verbose.txt",     tr("Loading Other Devices Info...")});
    m_ListCmd.append({ "dmesg",                "sudo dmesg",              "dmesg.txt",              tr("Loading Power Info...")});
    m_ListCmd.append({ "hciconfig",            "hciconfig -a",            "hciconfig.txt",          tr("Loading Printer Info...")});

    m_ListCmd.append({ "cat_cpuinfo",          "cat /proc/cpuinfo",       "cat_cpuinfo.txt",        tr("Loading Monitor Info...")});
    m_ListCmd.append({ "cat_boardinfo",        "cat /proc/boardinfo",     "cat_boardinfo.txt",      tr("Loading Mouse Info...")});
    m_ListCmd.append({ "cat_os_release",       "cat /etc/os-release",     "cat_os_release.txt",     tr("Loading Network Adapter Info...")});
    m_ListCmd.append({ "cat_version",          "cat /proc/version",       "cat_version.txt",        ""});
}

void ThreadPool::finishedCmd(const QString &info)
{
    m_FinishedNum++;
    if (m_FinishedNum == m_ListCmd.size()) {
        m_FinishedNum = 0;
        generateInfo();
    } else {
        if (!info.isEmpty())
            emit finished(info);
    }
}

void ThreadPool::finishedGenerateDevice()
{
    m_FinishedNum++;
    if (m_FinishedNum == m_ListDeviceTypes.size()) {
        emit finished("finish");
    }
}

void ThreadPool::loadCmdInfo()
{
    CmdTool::clear();
    DeviceManager::instance()->clear();
    m_FinishedNum = 0;
    foreach (QStringList item, m_ListCmd) {
        start(new CmdTask(item[0], item[1], item[2], item[3], this));
    }
}


void ThreadPool::generateInfo()
{
    DeviceManager::instance()->clear();
    foreach (DeviceType type, m_ListDeviceTypes) {
        start(new GenerateTask(type, this));
    }
}

