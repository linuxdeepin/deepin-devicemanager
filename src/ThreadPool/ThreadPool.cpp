#include "ThreadPool.h"
#include<QDebug>
#include <unistd.h>
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"
#include "LoadInfo/CmdTool.h"
#include "LoadInfo/GenerateDevice.h"

DeviceTask::DeviceTask(DeviceType deviceType, ThreadPool *parent)
    : m_Type(deviceType), mp_Parent(parent)
{
}
DeviceTask::~DeviceTask()
{

}
void DeviceTask::run()
{
    DeviceInfoParser infoParser;
    switch (m_Type) {
    case DT_Audio:
        infoParser.loadAudioInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Audio Device Info..."));
        }
        break;
    case DT_Bios:
        infoParser.loadBiosInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading BIOS Info..."));
        }
        break;
    case DT_Bluetoorh:
        infoParser.loadBluetoothInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Bluetooth Device Info..."));
        }
        break;
    case DT_Cdrom:
        infoParser.loadCdromInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading CD-ROM Info..."));
        }
        break;
    case DT_Computer:
        infoParser.loadComputerInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Operating System Info..."));
        }
        break;
    case DT_Cpu:
        infoParser.loadCpuInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading CPU Info..."));
        }
        break;
    case DT_Gpu:
        infoParser.loadGpuInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading GPU Info..."));
        }
        break;
    case DT_Image:
        infoParser.loadImageInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Image Devices Info..."));
        }
        break;
    case DT_Keyboard:
        infoParser.loadKeyboardInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Keyboard Info..."));
        }
        break;
    case DT_Memory:
        infoParser.loadMemoryInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Memory Info..."));
        }
        break;
    case DT_Monitor:
        infoParser.loadMonitorInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Monitor Info..."));
        }
        break;
    case DT_Mouse:
        infoParser.loadMouseInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Mouse Info..."));
        }
        break;
    case DT_Network:
        infoParser.loadNetworkInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Network Adapter Info..."));
        }
        break;
    case DT_OtherPCI:

        break;
    case DT_Others:
        infoParser.loadOtherDevices();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Other Devices Info..."));
        }
        break;
    case DT_Power:
        infoParser.loadPowerInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Power Info..."));
        }
        break;
    case DT_Print:
        infoParser.loadPrinterInfo();
        if (mp_Parent) {
            mp_Parent->finishedTask(tr("Loading Printer Info..."));
        }
        break;
    default: {

    }
    }
}


CmdTask::CmdTask(QString cmd, QString debugFile,  QString key, QString paragraphSplit, QString st,  QString keyValueSplit, QString info, ThreadPool *parent)
    : m_Cmd(cmd), m_DebugFile(debugFile), m_Key(key), m_ST(st), m_Info(info), m_paragraphSplit(paragraphSplit), m_KeyValueSplit(keyValueSplit), mp_Parent(parent)
{

}
CmdTask::~CmdTask()
{

}
void CmdTask::run()
{
    CmdTool tool;
    int st = m_ST.toInt();
    tool.loadCmdInfo(m_Cmd, m_Key, m_paragraphSplit, KeyValueSplit(st), m_KeyValueSplit, m_DebugFile);
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
    GenerateDevice task;
    switch (m_Type) {
    case DT_Computer:
        task.generatorComputerDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Cpu:
        task.generatorCpuDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Bios:
        task.generatorBiosDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Memory:
        task.generatorMemoryDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Storage:
        task.generatorDiskDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Gpu:
        task.generatorGpuDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Monitor:
        task.generatorMonitorDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Network:
        task.generatorNetworkDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Audio:
        task.generatorAudioDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Bluetoorh:
        task.generatorBluetoothDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Keyboard:
        task.generatorKeyboardDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Mouse:
        task.generatorMouseDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Print:
        task.generatorPrinterDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Image:
        task.generatorCameraDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Cdrom:
        task.generatorCdromDevice();
        mp_Parent->finishedGenerateDevice();
        break;
    case DT_Others:
        task.generatorOthersDevice();
        mp_Parent->finishedGenerateDevice();
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


    m_ListDeviceType.push_back(DT_Audio);
    m_ListDeviceType.push_back(DT_Bios);
    m_ListDeviceType.push_back(DT_Bluetoorh);
    m_ListDeviceType.push_back(DT_Computer);
    m_ListDeviceType.push_back(DT_Cpu);
    m_ListDeviceType.push_back(DT_Gpu);
    m_ListDeviceType.push_back(DT_Image);
    m_ListDeviceType.push_back(DT_Keyboard);
    m_ListDeviceType.push_back(DT_Memory);
    m_ListDeviceType.push_back(DT_Monitor);
    m_ListDeviceType.push_back(DT_Mouse);
    m_ListDeviceType.push_back(DT_Others);
    m_ListDeviceType.push_back(DT_Network);
    m_ListDeviceType.push_back(DT_Power);
    m_ListDeviceType.push_back(DT_Print);
    m_ListDeviceType.push_back(DT_Cdrom);



    m_ListCmd.append({ "sudo lshw", "lshw.txt", "lshw", "*-",    "2", ": ",   tr("Loading Audio Device Info...") });
    m_ListCmd.append({ "printer",                "printer.txt",        "printer",      "\n\n",    "1", ": ",   ""});

    m_ListCmd.append({ "sudo dmidecode -t 0",  "dmidecode_0.txt",  "dmidecode0", "\n\n",    "3", ": ",   tr("Loading BIOS Info...")});
    m_ListCmd.append({ "sudo dmidecode -t 1",  "dmidecode_1.txt",  "dmidecode1", "\n\n",    "3", ": ",   ""});
    m_ListCmd.append({ "sudo dmidecode -t 2",  "dmidecode_2.txt",  "dmidecode2", "\n\n",    "3", ": ",   ""});
    m_ListCmd.append({ "sudo dmidecode -t 3",  "dmidecode_3.txt",  "dmidecode3", "\n\n",    "3", ": ",   ""});
    m_ListCmd.append({ "sudo dmidecode -t 4",  "dmidecode_4.txt",  "dmidecode4", "\n\n",    "3", ": ",   ""});
    m_ListCmd.append({ "sudo dmidecode -t 16", "dmidecode_16.txt", "dmidecode16", "\n\n",    "3", ": ",   ""});
    m_ListCmd.append({ "sudo dmidecode -t 17", "dmidecode_17.txt", "dmidecode17", "\n\n",    "3", ": ",   ""});


    m_ListCmd.append({ "hwinfo --monitor",     "hwinfo_monitor.txt",      "hwinfo_monitor",  "\n\n",    "4", ": ",   tr("Loading CD-ROM Info...")});
    m_ListCmd.append({ "hwinfo --sound",       "hwinfo_sound.txt",        "hwinfo_sound",    "\n\n",    "4", ": ",   ""});
    m_ListCmd.append({ "hwinfo --usb",         "hwinfo_usb.txt",          "hwinfo_usb",      "\n\n",    "4", ": ",   ""});
    m_ListCmd.append({ "hwinfo --network",     "hwinfo_network.txt",      "hwinfo_network",  "\n\n",    "4", ": ",   ""});
    m_ListCmd.append({ "hwinfo --keyboard",    "hwinfo_keyboard.txt",     "hwinfo_keyboard", "\n\n",    "4", ": ",   tr("Loading Bluetooth Device Info...")});
    m_ListCmd.append({ "hwinfo --cdrom",       "hwinfo_cdrom.txt",        "hwinfo_cdrom",    "\n\n",    "4", ": ",   tr("Loading Image Devices Info...")});
    m_ListCmd.append({ "sudo hwinfo --disk",        "hwinfo_disk.txt",         "hwinfo_disk",     "\n\n",    "4", ": ",   tr("Loading Keyboard Info...")});
    m_ListCmd.append({ "hwinfo --display",     "hwinfo_display.txt",      "hwinfo_display",  "\n\n",    "4", ": ",   ""});
    m_ListCmd.append({ "hwinfo --mouse",       "hwinfo_mouse.txt",        "hwinfo_mouse",    "\n\n",    "4", ": ",   ""});


    m_ListCmd.append({ "lscpu",                 "lscpu.txt",               "lscpu",          "\n\n",    "1", ": ",   tr("Loading Operating System Info...")});
    m_ListCmd.append({ "lsblk -d -o name,rota", "lsblk_d.txt",             "lsblk_d",        "\n\n",    "0", ": ",   ""});
    m_ListCmd.append({ "xrandr",                "xrandr.txt",              "xrandr",         "\n\n",    "0", ": ",   tr("Loading CPU Info...")});
    m_ListCmd.append({ "xrandr --verbose",      "xrandr_verbose.txt",      "xrandr_verbose", "\n\n",    "0", ": ",   tr("Loading Other Devices Info...")});
    m_ListCmd.append({ "sudo dmesg",            "dmesg.txt",               "dmesg",          "\n\n",    "0", ": ",   tr("Loading Power Info...")});
    m_ListCmd.append({ "lspci",                 "lspci.txt",               "lspci",          "\n\n",    "0", ": ",   tr("Loading GPU Info...")});
    m_ListCmd.append({ "hciconfig -a",          "hciconfig.txt",           "hciconfig",      "\n\n",    "0", ": ",   tr("Loading Printer Info...")});


    m_ListCmd.append({ "cat /proc/bus/input/devices",      "cat_devices.txt",        "cat_devices",      "\n\n",    "1", ": ",   tr("Loading Memory Info...")});
    m_ListCmd.append({ "cat /proc/cpuinfo",                "cat_cpuinfo.txt",        "cat_cpuinfo",      "\n\n",    "1", ": ",   tr("Loading Monitor Info...")});
    m_ListCmd.append({ "cat /proc/boardinfo",              "cat_boardinfo.txt",      "cat_boardinfo",    "\n\n",    "1", ": ",   tr("Loading Mouse Info...")});
    m_ListCmd.append({ "cat /etc/os-release",              "cat_os_release.txt",     "cat_os_release",   "\n\n",        "1", "=",   tr("Loading Network Adapter Info...")});
    m_ListCmd.append({ "cat /proc/version",                "cat_version.txt",        "cat_version",      "\n\n",    "1", ": ",   ""});
}

void ThreadPool::finishedTask(const QString &info)
{
    m_FinishedNum++;
    if (m_FinishedNum == m_ListDeviceType.size()) {
        emit finished("finish");
    } else {
        emit finished(info);
    }
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

void ThreadPool::loadDeviceInfo()
{
    DeviceManager::instance()->clear();
    DeviceInfoParser::clear();
    m_FinishedNum = 0;
    foreach (DeviceType type, m_ListDeviceType) {
        start(new DeviceTask(type, this));
    }
}


void ThreadPool::loadCmdInfo()
{
    CmdTool::clear();
    DeviceManager::instance()->clear();
    m_FinishedNum = 0;
    foreach (QStringList item, m_ListCmd) {
        start(new CmdTask(item[0], item[1], item[2], item[3], item[4], item[5], item[6], this));
    }
}


void ThreadPool::generateInfo()
{
    DeviceManager::instance()->clear();
    foreach (DeviceType type, m_ListDeviceTypes) {
        start(new GenerateTask(type, this));
    }
}

