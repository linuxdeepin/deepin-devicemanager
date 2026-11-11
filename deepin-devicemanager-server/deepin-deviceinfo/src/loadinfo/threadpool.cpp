// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "threadpool.h"
#include "threadpooltask.h"
#include "deviceinfomanager.h"

#include <QObjectCleanupHandler>
#include <QDir>
#include <QLoggingCategory>

ThreadPool::ThreadPool(QObject *parent)
    : QThreadPool(parent)
{
    initCmd();

    QDir dir;
    dir.mkdir(PATH);
}

void ThreadPool::loadDeviceInfo()
{
    // 根据m_ListCmd生成所有设备信息
    updateInfo(m_ListCmd);
}

void ThreadPool::updateDeviceInfo()
{
    // 根据m_ListUpdate生成所有设备信息
    updateInfo(m_ListUpdate);
}

void ThreadPool::updateHotplugInfo()
{
    // 只刷新与 USB 插拔相关、开销可控的信息
    updateInfo(m_ListHotplug);
}

void ThreadPool::updateInfo(const QList<Cmd> &cmds)
{
    for (const Cmd &cmd : cmds) {
        ThreadPoolTask *task = new ThreadPoolTask(cmd.cmd, cmd.file, cmd.canNotReplace, cmd.waitingTime);
        task->setAutoDelete(true);
        start(task);
    }

    // 等待所有任务执行完毕，最多 10 秒。
    // 原实现是 while (true) + activeThreadCount() 的忙等自旋，不 sleep，
    // 会白占一个 CPU 核心最多 10 秒；音频等实时线程正好被它挤掉时间片。
    // QThreadPool::waitForDone() 内部用条件变量等待，不消耗 CPU。
    waitForDone(10000);
}

void ThreadPool::initCmd()
{
    // 添加lshw命令
    Cmd cmdLshw;
    cmdLshw.cmd = QString("%1 %2%3").arg("lshw > ").arg(PATH).arg("lshw.txt");
    cmdLshw.file = "lshw.txt";
    cmdLshw.canNotReplace = false;
    m_ListCmd.append(cmdLshw);
    m_ListUpdate.append(cmdLshw);

    // 添加dmidecode -s system-product-name命令
    Cmd cmdDmiSPN;
    cmdDmiSPN.cmd = QString("%1 %2%3").arg("dmidecode -s system-product-name > ").arg(PATH).arg("dmidecode_spn.txt");
    cmdDmiSPN.file = "dmidecode_spn.txt";
    cmdDmiSPN.canNotReplace = true;
    m_ListCmd.append(cmdDmiSPN);

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

    // 添加hwinfo --power命令
    Cmd cmdUpower;
    cmdUpower.cmd = QString("%1 %2%3").arg("upower --dump > ").arg(PATH).arg("upower_dump.txt");
    cmdUpower.file = "upower_dump.txt";
    cmdUpower.canNotReplace = true;
    m_ListCmd.append(cmdUpower);
    m_ListUpdate.append(cmdUpower);

    // 添加lscpu命令
    Cmd cmdLscpu;
    cmdLscpu.cmd = "lscpu";//QString("%1 %2%3").arg("lscpu > ").arg(PATH).arg("lscpu.txt");
    cmdLscpu.file = "lscpu.txt";
    cmdLscpu.canNotReplace = true;
    m_ListCmd.append(cmdLscpu);
    m_ListUpdate.append(cmdLscpu);

    // 添加lsblk -d -o name,rota命令
    Cmd cmdLsblk;
    cmdLsblk.cmd = QString("%1 %2%3").arg("lsblk -d -o name,rota > ").arg(PATH).arg("lsblk_d.txt");
    cmdLsblk.file = "lsblk_d.txt";
    cmdLsblk.canNotReplace = false;
    m_ListCmd.append(cmdLsblk);
    m_ListUpdate.append(cmdLsblk);

    // 添加lsblk -d -o name,rota命令
    Cmd cmdLssg;
    cmdLssg.cmd = QString("%1 %2%3").arg("ls /dev/sg* > ").arg(PATH).arg("ls_sg.txt");
    cmdLssg.file = "ls_sg.txt";
    cmdLssg.canNotReplace = false;
    m_ListCmd.append(cmdLssg);
    m_ListUpdate.append(cmdLssg);

    // 添加lspci命令
    Cmd cmdLspci;
    cmdLspci.cmd = QString("%1 %2%3").arg("lspci > ").arg(PATH).arg("lspci.txt");
    cmdLspci.file = "lspci.txt";
    cmdLspci.canNotReplace = false;
    m_ListCmd.append(cmdLspci);
    m_ListUpdate.append(cmdLspci);

    // 添加lpstat -a命令
    Cmd cmdLpstate;
    cmdLpstate.cmd = QString("%1 %2%3").arg("lpstat -a > ").arg(PATH).arg("lpstat.txt");
    cmdLpstate.file = "lpstat.txt";
    cmdLpstate.canNotReplace = false;
    m_ListCmd.append(cmdLpstate);
    m_ListUpdate.append(cmdLpstate);

    // 添加dmesg命令
    Cmd cmdDmesg;
    cmdDmesg.cmd = QString("%1 %2%3").arg("dmesg > ").arg(PATH).arg("dmesg.txt");
    cmdDmesg.file = "dmesg.txt";
    cmdDmesg.canNotReplace = true;
    m_ListCmd.append(cmdDmesg);
    m_ListUpdate.append(cmdDmesg);

    // 添加hciconfig -a命令
    Cmd cmdHciconfig;
    cmdHciconfig.cmd = QString("%1 %2%3").arg("hciconfig -a > ").arg(PATH).arg("hciconfig.txt");
    cmdHciconfig.file = "hciconfig.txt";
    cmdHciconfig.canNotReplace = false;
    m_ListCmd.append(cmdHciconfig);
    m_ListUpdate.append(cmdHciconfig);

    // 添加bluetoothctl paired-devices命令
    Cmd cmdBluetooth;
    cmdBluetooth.cmd = QString("%1 %2%3").arg("bluetoothctl paired-devices > ").arg(PATH).arg("bt_device.txt");
    cmdBluetooth.file = "bt_device.txt";
    cmdBluetooth.canNotReplace = false;
    cmdBluetooth.waitingTime = 500;
    m_ListCmd.append(cmdBluetooth);
    m_ListUpdate.append(cmdBluetooth);

    // 获取cat /boot/config* | grep '=y'信息
    Cmd cmdLsMod;
    cmdLsMod.cmd = QString("%1 %2%3").arg("cat /boot/config* | grep '=y' > ").arg(PATH).arg("dr_config.txt");
    cmdLsMod.file = "dr_config.txt";
    cmdLsMod.canNotReplace = true;
    m_ListCmd.append(cmdLsMod);

    Cmd cmdHwinfo;     //同步"hwinfo --network"改为 "hwinfo --netcard"获取网卡信息
    cmdHwinfo.cmd = QString("%1 %2%3").arg("hwinfo --sound --netcard --keyboard --cdrom --disk --display --mouse --usb --fingerprint > ").arg(PATH).arg("hwinfo.txt");
    cmdHwinfo.file = "hwinfo.txt";
    cmdHwinfo.canNotReplace = false;
    m_ListCmd.append(cmdHwinfo);
    m_ListUpdate.append(cmdHwinfo);

    Cmd cmdHwinfoMonitor;     //hwinfo --framebuffer --monitor
    cmdHwinfoMonitor.cmd = QString("%1 %2%3").arg("hwinfo --monitor > ").arg(PATH).arg("hwinfo_monitor.txt");
    cmdHwinfoMonitor.file = "hwinfo_monitor.txt";
    cmdHwinfoMonitor.canNotReplace = false;
    m_ListCmd.append(cmdHwinfoMonitor);
    m_ListUpdate.append(cmdHwinfoMonitor);

    // ---------------------------------------------------------------------
    // 热插拔(USB)刷新列表
    //
    // 一次插拔只需要更新"会变"的信息。lshw / upower / lscpu / lspci /
    // lpstat / bluetoothctl / hwinfo --framebuffer 与插拔无关，却要跑数百
    // 毫秒到数秒，并发执行时会把 CPU 占满，正在播放的音频因此丢帧（滋滋声）。
    // 未列入的命令不执行，其缓存保持上一次的值，不会丢失数据。
    // ---------------------------------------------------------------------
    m_ListHotplug.append(cmdLsblk);      // 新增/移除磁盘
    m_ListHotplug.append(cmdLssg);       // 新增/移除 SCSI 通用设备
    m_ListHotplug.append(cmdHciconfig);  // 蓝牙适配器状态
    m_ListHotplug.append(cmdHwinfo);     // 含 --sound：声卡拓扑也可能变化
}
