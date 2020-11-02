#include "ThreadPool.h"
#include "ThreadPoolTask.h"

#include <QObjectCleanupHandler>
#include <QProcess>
#include <QDir>
#include <QDebug>

//#define PATH "/home/liujun/device-info/"
#define PATH "/tmp/device-info/"

ThreadPool::ThreadPool(QObject *parent)
    : QThreadPool(parent)
{
    initCmd();

    QDir dir;
    dir.mkdir("/tmp/device-info");
}

void ThreadPool::generateDeviceFile()
{
    QObjectCleanupHandler *cleaner = new QObjectCleanupHandler;
    cleaner->setParent(this);
    QList<Cmd>::iterator it = m_ListCmd.begin();
    for (; it != m_ListCmd.end(); ++it) {
        ThreadPoolTask *task = new ThreadPoolTask((*it).cmd, (*it).file, (*it).canNotReplace, (*it).waitingTime);
        cleaner->add(task);
        start(task);
        task->setAutoDelete(true);
    }
}

void ThreadPool::generateMonitor()
{
    QObjectCleanupHandler *cleaner = new QObjectCleanupHandler;
    cleaner->setParent(this);
    QList<Cmd>::iterator it = m_ListCmdMonitor.begin();
    for (; it != m_ListCmdMonitor.end(); ++it) {
        ThreadPoolTask *task = new ThreadPoolTask((*it).cmd, (*it).file, (*it).canNotReplace, (*it).waitingTime);
        cleaner->add(task);
        start(task);
        task->setAutoDelete(true);
    }
}

void ThreadPool::initCmd()
{
    // According to Huawei's requirements , Modify the way of judging klu and panguv
    QProcess process;
    process.start("gdbus introspect -y -d com.deepin.system.SystemInfo -o /com/deepin/system/SystemInfo -p");
    process.waitForFinished(-1);
    QString info = process.readAllStandardOutput();


    Cmd cmdLshw;
    cmdLshw.cmd = QString("%1 %2%3").arg("lshw > ").arg(PATH).arg("lshw.txt");
    cmdLshw.file = "lshw.txt";
    cmdLshw.canNotReplace = false;
    m_ListCmd.append(cmdLshw);

    Cmd cmdDmi0;
    cmdDmi0.cmd = QString("%1 %2%3").arg("dmidecode -t 0 > ").arg(PATH).arg("dmidecode_0.txt");
    cmdDmi0.file = "dmidecode_0.txt";
    cmdDmi0.canNotReplace = true;
    m_ListCmd.append(cmdDmi0);

    Cmd cmdDmi1;
    cmdDmi1.cmd = QString("%1 %2%3").arg("dmidecode -t 1 > ").arg(PATH).arg("dmidecode_1.txt");
    cmdDmi1.file = "dmidecode_1.txt";
    cmdDmi1.canNotReplace = true;
    m_ListCmd.append(cmdDmi1);

    Cmd cmdDmi2;
    cmdDmi2.cmd = QString("%1 %2%3").arg("dmidecode -t 2 > ").arg(PATH).arg("dmidecode_2.txt");
    cmdDmi2.file = "dmidecode_2.txt";
    cmdDmi2.canNotReplace = true;
    m_ListCmd.append(cmdDmi2);

    Cmd cmdDmi3;
    cmdDmi3.cmd = QString("%1 %2%3").arg("dmidecode -t 3 > ").arg(PATH).arg("dmidecode_3.txt");
    cmdDmi3.file = "dmidecode_3.txt";
    cmdDmi3.canNotReplace = true;
    m_ListCmd.append(cmdDmi3);

    Cmd cmdDmi4;
    cmdDmi4.cmd = QString("%1 %2%3").arg("dmidecode -t 4 > ").arg(PATH).arg("dmidecode_4.txt");
    cmdDmi4.file = "dmidecode_4.txt";
    cmdDmi4.canNotReplace = true;
    m_ListCmd.append(cmdDmi4);

    Cmd cmdDmi13;
    cmdDmi13.cmd = QString("%1 %2%3").arg("dmidecode -t 13 > ").arg(PATH).arg("dmidecode_13.txt");
    cmdDmi13.file = "dmidecode_13.txt";
    cmdDmi13.canNotReplace = true;
    m_ListCmd.append(cmdDmi13);

    Cmd cmdDmi16;
    cmdDmi16.cmd = QString("%1 %2%3").arg("dmidecode -t 16 > ").arg(PATH).arg("dmidecode_16.txt");
    cmdDmi16.file = "dmidecode_16.txt";
    cmdDmi16.canNotReplace = true;
    m_ListCmd.append(cmdDmi16);

    Cmd cmdDmi17;
    cmdDmi17.cmd = QString("%1 %2%3").arg("dmidecode -t 17 > ").arg(PATH).arg("dmidecode_17.txt");
    cmdDmi17.file = "dmidecode_17.txt";
    cmdDmi17.canNotReplace = true;
    m_ListCmd.append(cmdDmi17);

    Cmd cmdHwinfoMonitor;
    cmdHwinfoMonitor.cmd = QString("%1 %2%3").arg("hwinfo --monitor > ").arg(PATH).arg("hwinfo_monitor.txt");
    cmdHwinfoMonitor.file = "hwinfo_monitor.txt";
    cmdHwinfoMonitor.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoMonitor);
    m_ListCmdMonitor.append(cmdHwinfoMonitor);

    Cmd cmdHwinfoSound;
    cmdHwinfoSound.cmd = QString("%1 %2%3").arg("hwinfo --sound > ").arg(PATH).arg("hwinfo_sound.txt");
    cmdHwinfoSound.file = "hwinfo_sound.txt";
    cmdHwinfoSound.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoSound);

    Cmd cmdHwinfoUsb;
    cmdHwinfoUsb.cmd = QString("%1 %2%3").arg("hwinfo --usb > ").arg(PATH).arg("hwinfo_usb.txt");
    cmdHwinfoUsb.file = "hwinfo_usb.txt";
    cmdHwinfoUsb.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoUsb);

    Cmd cmdHwinfoNetwork;
    cmdHwinfoNetwork.cmd = QString("%1 %2%3").arg("hwinfo --network > ").arg(PATH).arg("hwinfo_network.txt");
    cmdHwinfoNetwork.file = "hwinfo_network.txt";
    cmdHwinfoNetwork.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoNetwork);

    Cmd cmdHwinfoKeyboard;
    cmdHwinfoKeyboard.cmd = QString("%1 %2%3").arg("hwinfo --keyboard > ").arg(PATH).arg("hwinfo_keyboard.txt");
    cmdHwinfoKeyboard.file = "hwinfo_keyboard.txt";
    cmdHwinfoKeyboard.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoKeyboard);

    Cmd cmdHwinfoCdrom;
    cmdHwinfoCdrom.cmd = QString("%1 %2%3").arg("hwinfo --cdrom > ").arg(PATH).arg("hwinfo_cdrom.txt");
    cmdHwinfoCdrom.file = "hwinfo_cdrom.txt";
    cmdHwinfoCdrom.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoCdrom);

    Cmd cmdHwinfoDisk;
    cmdHwinfoDisk.cmd = QString("%1 %2%3").arg("hwinfo --disk > ").arg(PATH).arg("hwinfo_disk.txt");
    cmdHwinfoDisk.file = "hwinfo_disk.txt";
    cmdHwinfoDisk.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoDisk);

    Cmd cmdHwinfoDisplay;
    cmdHwinfoDisplay.cmd = QString("%1 %2%3").arg("hwinfo --display > ").arg(PATH).arg("hwinfo_display.txt");
    cmdHwinfoDisplay.file = "hwinfo_display.txt";
    cmdHwinfoDisplay.canNotReplace = true;
    m_ListCmd.append(cmdHwinfoDisplay);

    Cmd cmdHwinfoMouse;
    cmdHwinfoMouse.cmd = QString("%1 %2%3").arg("hwinfo --mouse > ").arg(PATH).arg("hwinfo_mouse.txt");
    cmdHwinfoMouse.file = "hwinfo_mouse.txt";
    cmdHwinfoMouse.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoMouse);

    Cmd cmdUpower;
    cmdUpower.cmd = QString("%1 %2%3").arg("hwinfo --dump > ").arg(PATH).arg("upower_dump.txt");
    cmdUpower.file = "upower_dump.txt";
    cmdUpower.canNotReplace = true;
    m_ListCmd.append(cmdUpower);

    Cmd cmdLscpu;
    cmdLscpu.cmd = QString("%1 %2%3").arg("lscpu > ").arg(PATH).arg("lscpu.txt");
    cmdLscpu.file = "lscpu.txt";
    cmdLscpu.canNotReplace = true;
    m_ListCmd.append(cmdLscpu);

    Cmd cmdLsblk;
    cmdLsblk.cmd = QString("%1 %2%3").arg("lsblk -d -o name,rota > ").arg(PATH).arg("lsblk_d.txt");
    cmdLsblk.file = "lsblk_d.txt";
    cmdLsblk.canNotReplace = false;
    m_ListCmd.append(cmdLsblk);

    Cmd cmdLspci;
    cmdLspci.cmd = QString("%1 %2%3").arg("lspci > ").arg(PATH).arg("lspci.txt");
    cmdLspci.file = "lspci.txt";
    cmdLspci.canNotReplace = false;
    m_ListCmd.append(cmdLspci);

    Cmd cmdLpstate;
    cmdLpstate.cmd = QString("%1 %2%3").arg("lpstat -a > ").arg(PATH).arg("lpstat.txt");
    cmdLpstate.file = "lpstat.txt";
    cmdLpstate.canNotReplace = false;
    m_ListCmd.append(cmdLpstate);

    Cmd cmdXrandr;
    cmdXrandr.cmd = QString("%1 %2%3").arg("xrandr > ").arg(PATH).arg("xrandr.txt");
    cmdXrandr.file = "xrandr.txt";
    cmdXrandr.canNotReplace = false;
    m_ListCmd.append(cmdXrandr);
    m_ListCmdMonitor.append(cmdXrandr);

    Cmd cmdXrandrVerbose;
    cmdXrandrVerbose.cmd = QString("%1 %2%3").arg("xrandr --verbose > ").arg(PATH).arg("xrandr_verbose.txt");
    cmdXrandrVerbose.file = "xrandr_verbose.txt";
    cmdXrandrVerbose.canNotReplace = false;
    m_ListCmd.append(cmdXrandrVerbose);
    m_ListCmdMonitor.append(cmdXrandrVerbose);

    Cmd cmdDmesg;
    cmdDmesg.cmd = QString("%1 %2%3").arg("dmesg > ").arg(PATH).arg("dmesg.txt");
    cmdDmesg.file = "dmesg.txt";
    cmdDmesg.canNotReplace = true;
    m_ListCmd.append(cmdDmesg);

    Cmd cmdHciconfig;
    cmdHciconfig.cmd = QString("%1 %2%3").arg("hciconfig -a > ").arg(PATH).arg("hciconfig.txt");
    cmdHciconfig.file = "hciconfig.txt";
    cmdHciconfig.canNotReplace = false;
    m_ListCmd.append(cmdHciconfig);

    Cmd cmdBluetooth;
    cmdBluetooth.cmd = QString("%1 %2%3").arg("bluetoothctl paired-devices > ").arg(PATH).arg("bt_device.txt");
    cmdBluetooth.file = "bt_device.txt";
    cmdBluetooth.canNotReplace = false;
    cmdBluetooth.waitingTime = 500;
    m_ListCmd.append(cmdBluetooth);



//    if (info.contains("klu")) {
//        Cmd cmdGpuinfo;
//        cmdGpuinfo.cmd = "gpuinfo";
//        cmdGpuinfo.file = "gpuinfo.txt";
//        cmdGpuinfo.canNotReplace = true;
//        m_ListCmd.append(cmdGpuinfo);

//        Cmd cmdBootdevice;
//        cmdBootdevice.cmd = "cat /proc/bootdevice/product_name";
//        cmdBootdevice.file = "bootdevice.txt";
//        cmdBootdevice.canNotReplace = false;
//        m_ListCmd.append(cmdBootdevice);

//    } else if (info.contains("panguV")) {
//        Cmd cmdGpuinfo;
//        cmdGpuinfo.cmd = "gpuinfo";
//        cmdGpuinfo.file = "gpuinfo.txt";
//        cmdGpuinfo.canNotReplace = true;
//        m_ListCmd.append(cmdGpuinfo);

//        Cmd cmdBootdevice;
//        cmdBootdevice.cmd = "cat /proc/bootdevice/product_name";
//        cmdBootdevice.file = "bootdevice.txt";
//        cmdBootdevice.canNotReplace = false;
//        m_ListCmd.append(cmdBootdevice);

//        Cmd cmdEdidHdmi;
//        cmdEdidHdmi.cmd = "hexdump /sys/devices/platform/hisi-drm/drm/card0/card0-HDMI-A-1/edid";
//        cmdEdidHdmi.file = "EDID_HDMI.txt";
//        cmdEdidHdmi.canNotReplace = false;
//        m_ListCmd.append(cmdEdidHdmi);

//        Cmd cmdEdidVGA;
//        cmdEdidVGA.cmd = "hexdump /sys/devices/platform/hisi-drm/drm/card0/card0-VGA-1/edid";
//        cmdEdidVGA.file = "EDID_VGA.txt";
//        cmdEdidVGA.canNotReplace = false;
//        m_ListCmd.append(cmdEdidVGA);
//    }
}
