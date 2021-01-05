#include "ThreadPool.h"
#include "ThreadPoolTask.h"

#include <QObjectCleanupHandler>
#include <QProcess>
#include <QDir>
#include <QDebug>

ThreadPool::ThreadPool(QObject *parent)
    : QThreadPool(parent)
{
    initCmd();

    QDir dir;
    dir.mkdir(PATH);
}

void ThreadPool::generateDeviceFile()
{
    // 根据m_ListCmd生成所有设备信息
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

void ThreadPool::initCmd()
{
    // According to Huawei's requirements , Modify the way of judging klu and panguv
    // 获取华为KLU、PanguV信息
    Cmd cmdDbus;
    cmdDbus.cmd = QString("%1 %2%3").arg("gdbus introspect -y -d com.deepin.system.SystemInfo -o /com/deepin/system/SystemInfo -p >").arg(PATH).arg("gdbus.txt");
    cmdDbus.file = "gdbus.txt";
    cmdDbus.canNotReplace = false;
    m_ListCmd.append(cmdDbus);

    // 获取架构信息
    Cmd cmdUname;
    cmdUname.cmd = QString("%1 %2%3").arg("uname -m > ").arg(PATH).arg("uname_m.txt");
    cmdUname.file = "uname_m.txt";
    cmdUname.canNotReplace = false;
    m_ListCmd.append(cmdUname);

    // 添加lshw命令
    Cmd cmdLshw;
    cmdLshw.cmd = QString("%1 %2%3").arg("lshw > ").arg(PATH).arg("lshw.txt");
    cmdLshw.file = "lshw.txt";
    cmdLshw.canNotReplace = false;
    m_ListCmd.append(cmdLshw);

    // 添加dmidecode -t 0命令
    Cmd cmdDmi0;
    cmdDmi0.cmd = QString("%1 %2%3").arg("dmidecode -t 0 > ").arg(PATH).arg("dmidecode_0.txt");
    cmdDmi0.file = "dmidecode_0.txt";
    cmdDmi0.canNotReplace = true;
    m_ListCmd.append(cmdDmi0);

    // 添加dmidecode -t 1命令
    Cmd cmdDmi1;
    cmdDmi1.cmd = QString("%1 %2%3").arg("dmidecode -t 1 > ").arg(PATH).arg("dmidecode_1.txt");
    cmdDmi1.file = "dmidecode_1.txt";
    cmdDmi1.canNotReplace = true;
    m_ListCmd.append(cmdDmi1);

    // 添加dmidecode -t 2命令
    Cmd cmdDmi2;
    cmdDmi2.cmd = QString("%1 %2%3").arg("dmidecode -t 2 > ").arg(PATH).arg("dmidecode_2.txt");
    cmdDmi2.file = "dmidecode_2.txt";
    cmdDmi2.canNotReplace = true;
    m_ListCmd.append(cmdDmi2);

    // 添加dmidecode -t 3命令
    Cmd cmdDmi3;
    cmdDmi3.cmd = QString("%1 %2%3").arg("dmidecode -t 3 > ").arg(PATH).arg("dmidecode_3.txt");
    cmdDmi3.file = "dmidecode_3.txt";
    cmdDmi3.canNotReplace = true;
    m_ListCmd.append(cmdDmi3);

    // 添加dmidecode -t 4命令
    Cmd cmdDmi4;
    cmdDmi4.cmd = QString("%1 %2%3").arg("dmidecode -t 4 > ").arg(PATH).arg("dmidecode_4.txt");
    cmdDmi4.file = "dmidecode_4.txt";
    cmdDmi4.canNotReplace = true;
    m_ListCmd.append(cmdDmi4);

    // 添加dmidecode -t 13命令
    Cmd cmdDmi13;
    cmdDmi13.cmd = QString("%1 %2%3").arg("dmidecode -t 13 > ").arg(PATH).arg("dmidecode_13.txt");
    cmdDmi13.file = "dmidecode_13.txt";
    cmdDmi13.canNotReplace = true;
    m_ListCmd.append(cmdDmi13);

    // 添加dmidecode -t 16命令
    Cmd cmdDmi16;
    cmdDmi16.cmd = QString("%1 %2%3").arg("dmidecode -t 16 > ").arg(PATH).arg("dmidecode_16.txt");
    cmdDmi16.file = "dmidecode_16.txt";
    cmdDmi16.canNotReplace = true;
    m_ListCmd.append(cmdDmi16);

    // 添加dmidecode -t 17命令
    Cmd cmdDmi17;
    cmdDmi17.cmd = QString("%1 %2%3").arg("dmidecode -t 17 > ").arg(PATH).arg("dmidecode_17.txt");
    cmdDmi17.file = "dmidecode_17.txt";
    cmdDmi17.canNotReplace = true;
    m_ListCmd.append(cmdDmi17);

    // 添加hwinfo --sound命令
    Cmd cmdHwinfoSound;
    cmdHwinfoSound.cmd = QString("%1 %2%3").arg("hwinfo --sound > ").arg(PATH).arg("hwinfo_sound.txt");
    cmdHwinfoSound.file = "hwinfo_sound.txt";
    cmdHwinfoSound.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoSound);

    // 添加hwinfo --usb命令
    Cmd cmdHwinfoUsb;
    cmdHwinfoUsb.cmd = QString("%1 %2%3").arg("hwinfo --usb > ").arg(PATH).arg("hwinfo_usb.txt");
    cmdHwinfoUsb.file = "hwinfo_usb.txt";
    cmdHwinfoUsb.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoUsb);

    // 添加hwinfo --network命令
    Cmd cmdHwinfoNetwork;
    cmdHwinfoNetwork.cmd = QString("%1 %2%3").arg("hwinfo --network > ").arg(PATH).arg("hwinfo_network.txt");
    cmdHwinfoNetwork.file = "hwinfo_network.txt";
    cmdHwinfoNetwork.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoNetwork);

    // 添加hwinfo --keyboard命令
    Cmd cmdHwinfoKeyboard;
    cmdHwinfoKeyboard.cmd = QString("%1 %2%3").arg("hwinfo --keyboard > ").arg(PATH).arg("hwinfo_keyboard.txt");
    cmdHwinfoKeyboard.file = "hwinfo_keyboard.txt";
    cmdHwinfoKeyboard.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoKeyboard);

    // 添加hwinfo --network命令
    Cmd cmdHwinfoCdrom;
    cmdHwinfoCdrom.cmd = QString("%1 %2%3").arg("hwinfo --cdrom > ").arg(PATH).arg("hwinfo_cdrom.txt");
    cmdHwinfoCdrom.file = "hwinfo_cdrom.txt";
    cmdHwinfoCdrom.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoCdrom);

    // 添加hwinfo --disk命令
    Cmd cmdHwinfoDisk;
    cmdHwinfoDisk.cmd = QString("%1 %2%3").arg("hwinfo --disk > ").arg(PATH).arg("hwinfo_disk.txt");
    cmdHwinfoDisk.file = "hwinfo_disk.txt";
    cmdHwinfoDisk.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoDisk);

    // 添加hwinfo --display命令
    Cmd cmdHwinfoDisplay;
    cmdHwinfoDisplay.cmd = QString("%1 %2%3").arg("hwinfo --display > ").arg(PATH).arg("hwinfo_display.txt");
    cmdHwinfoDisplay.file = "hwinfo_display.txt";
    cmdHwinfoDisplay.canNotReplace = true;
    m_ListCmd.append(cmdHwinfoDisplay);

    // 添加hwinfo --mouse命令
    Cmd cmdHwinfoMouse;
    cmdHwinfoMouse.cmd = QString("%1 %2%3").arg("hwinfo --mouse > ").arg(PATH).arg("hwinfo_mouse.txt");
    cmdHwinfoMouse.file = "hwinfo_mouse.txt";
    cmdHwinfoMouse.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoMouse);

    // 添加hwinfo --power命令
    Cmd cmdUpower;
    cmdUpower.cmd = QString("%1 %2%3").arg("upower --dump > ").arg(PATH).arg("upower_dump.txt");
    cmdUpower.file = "upower_dump.txt";
    cmdUpower.canNotReplace = true;
    m_ListCmd.append(cmdUpower);

    // 添加lscpu命令
    Cmd cmdLscpu;
    cmdLscpu.cmd = QString("%1 %2%3").arg("lscpu > ").arg(PATH).arg("lscpu.txt");
    cmdLscpu.file = "lscpu.txt";
    cmdLscpu.canNotReplace = true;
    m_ListCmd.append(cmdLscpu);

    // 添加lsblk -d -o name,rota命令
    Cmd cmdLsblk;
    cmdLsblk.cmd = QString("%1 %2%3").arg("lsblk -d -o name,rota > ").arg(PATH).arg("lsblk_d.txt");
    cmdLsblk.file = "lsblk_d.txt";
    cmdLsblk.canNotReplace = false;
    m_ListCmd.append(cmdLsblk);

    // 添加lsblk -d -o name,rota命令
    Cmd cmdLssg;
    cmdLssg.cmd = QString("%1 %2%3").arg("ls /dev/sg* > ").arg(PATH).arg("ls_sg.txt");
    cmdLssg.file = "ls_sg.txt";
    cmdLssg.canNotReplace = false;
    m_ListCmd.append(cmdLssg);

    // 添加lspci命令
    Cmd cmdLspci;
    cmdLspci.cmd = QString("%1 %2%3").arg("lspci > ").arg(PATH).arg("lspci.txt");
    cmdLspci.file = "lspci.txt";
    cmdLspci.canNotReplace = false;
    m_ListCmd.append(cmdLspci);

    // 添加lpstat -a命令
    Cmd cmdLpstate;
    cmdLpstate.cmd = QString("%1 %2%3").arg("lpstat -a > ").arg(PATH).arg("lpstat.txt");
    cmdLpstate.file = "lpstat.txt";
    cmdLpstate.canNotReplace = false;
    m_ListCmd.append(cmdLpstate);

    // 添加dmesg命令
    Cmd cmdDmesg;
    cmdDmesg.cmd = QString("%1 %2%3").arg("dmesg > ").arg(PATH).arg("dmesg.txt");
    cmdDmesg.file = "dmesg.txt";
    cmdDmesg.canNotReplace = true;
    m_ListCmd.append(cmdDmesg);

    // 添加hciconfig -a命令
    Cmd cmdHciconfig;
    cmdHciconfig.cmd = QString("%1 %2%3").arg("hciconfig -a > ").arg(PATH).arg("hciconfig.txt");
    cmdHciconfig.file = "hciconfig.txt";
    cmdHciconfig.canNotReplace = false;
    m_ListCmd.append(cmdHciconfig);

    // 添加bluetoothctl paired-devices命令
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
