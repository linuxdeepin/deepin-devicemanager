/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QMap>
#include <QString>
#include "singletondef.h"
#include "deviceattributedefine.h"
#include"DeviceManager/DeviceInfo.h"
#include <QObject>
#include <QSet>

class QDeviceMouse;

class DeviceInfoParser: public QObject
{
    Q_OBJECT
public:

    static DeviceInfoParser &Instance()
    {
        static DeviceInfoParser _instance;
        return _instance;
    }
    DeviceInfoParser();
    ~DeviceInfoParser();

    void refreshDabase();
    static void clear();
    void loadComputerInfo();        // 计算机概况
    void loadMouseInfo();           // 计算机鼠标信息
    void loadCpuInfo();             // 计算机Cpu
    void loadGpuInfo();             // 计算机显存
    void loadMemoryInfo();          // 计算机内存
    void loadMonitorInfo();         // 计算机显示屏
    void loadBiosInfo();            // 计算机Bios
    void loadAudioInfo();           // 计算机Audio
    void loadImageInfo();           // 计算机 图像社别
    void loadBluetoothInfo();       // 计算机蓝牙设备
    void loadNetworkInfo();         // 计算机网络设备信息
    void loadKeyboardInfo();        // 计算机键盘信息
    void loadOtherDevices();        // 其它设备信息
    void loadPowerInfo();           // 电源信息
    void loadCdromInfo();           // Cdrom信息
    void loadPrinterInfo();         // 打印机信息
private:
    void loadDiskInfo();            // 计算机存储信息
    void test();


signals:
    void loadFinished(const QString &msg);

private:
    /**
     * @brief:获取计算机基本信息，获取基本信息的命令
     *      cat /etc/os-release
     *      cat /proc/version
     *      sudo dmidecode -t system
     *      sudo dmidecode -t Chassis
     */
    void loadComputerInfoFromCatOsRelease();
    void loadUOS();
    void loadComputerInfoFromCatVersion();
    void loadComputerInfoFromDmidecode();
    void loadComputerInfoFromLshw();
    void loadComputerInfoFromCatBoardinfo();

    /**
     * @brief:获取鼠标信息，获取鼠标信息的命令主要有三个
     *        cat /proc/bus/input/devices
     *        sudo hwinfo --mouse
     *        sudo lshw -C input       // 这个命令在x86机器对于ps2不支持
     * @brief[loadMouseInfoFromCatDevices]:
     * @brief[loadMouseInfoFromHwinfo]:
     * @brief[loadMouseInfoFromlshw]:
     */
    bool loadMouseInfoFromCatDevices();
    bool loadMouseInfoFromHwinfo();
    bool loadMouseInfoFromlshw();

    /**
     * @brief:获取cpu信息，获取cpu信息主要有两个命令
     *          lscpu
     *          sudo lshw -C processor
     *          sudo dmidecode -t processor
     *          cat /proc/cpuinfo
     * **** 特别注意，目前只能或一个cpu信息，因为没有环境(找不到哪台机器有两个cpu)
     * @brief[loadCpuInfoFromLscpu]:获取cpu基本信息
     * @brief[loadCpuInfoFromLshw]:获取cpu基本信息
     * @brief[loadCpuInfoFromDmidecode]:获取cpu基本信息
     * @brief[loadCpuInfoFromCatCpuinfo]:获取cpu里面核的信息
     */
    void loadCpuInfoFromLscpu(QMap<QString, QString> &mapLscpu);
    void loadCpuInfoFromLshw(QMap<QString, QString> &mapLshw);
    void loadCpuInfoFromDmidecode(QMap<QString, QString> &mapDmidecode);
    void loadCpuInfoFromCatCpuinfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode);


    /**
     * @brief:获取存储设备信息,获取存储设备的命令是
     *        sudo hwinfo --disk;
     *        lsblk
     *        sudo lshw -C disk
     * @brief[loadDiskInfoFromHwinfo]:从hwinfo中获取信息
     * @brief[loadDiskInfoFromLsblk]:从lsblk获取信息
     * @brief[loadDiskInfoFromLshw]:从lshw获取信息
     * @brief[loadDiskInfoFromSmartCtl]:从smartctl获取信息
     */
    void loadDiskInfoFromHwinfo();
    void loadDiskInfoFromLsblk(QStringList &logicNameList);
    void loadDiskInfoFromLshw();
    void loadDiskInfoFromSmartCtl(QStringList &logicNameList);

    /**
     * @brief:获取gpu信息，获取gpu信息的命令有
     *      sudo lshw -C display
     *      xrandr --current 获取最大分辨率， 最小分辨率， 当前分辨率，也可以获得 刷新率。
     * @brief[loadGpuInfoFromLshw]:从sudo lshw -C display中获取信息
     * @brief[loadGpuInfoFromHwinfo]:从sudo hwinfo --display中获取信息
     * @brief[loadInfoFromXrandr]: 从 xrandr 命令获取
     */
    void loadGpuInfoFromLshw();
    void loadGpuInfoFromHwinfo();
    void loadGpuInfoFromXrandr();
    void loadGpuSizeFromDmesg();


    /**
     * @brief:获取memory信息，获取memory信息的命令有
     *      sudo lshw -C memory
     *      sudo dmidecode -t memory  这个命令在龙芯下不管用
     * @brief[loadMemoryFromLshw]:从 sudo lshw -C memory 中获取命令
     * @brief[loadMemoryFromDmidecode]:从 sudo dmidecode -t memory 中获取命令
     */
    void loadMemoryFromLshw();
    void loadMemoryFromDmidecode();


    /**
     * @brief:获取显示设备,获取显示设备的命令
     *      sudo hwinfo --monitor
     *      xrandr 获取最大分辨率， 最小分辨率， 当前分辨率，也可以获得 刷新率。
     * @brief[loadMonitorInfoFromHwinfo]:从 sudo hwinfo --monitor 中获取信息
     * @brief[loadMonitorInfoFromXrandr]:从 xrandr --verbose 中获取信息
     */
    void loadMonitorInfoFromHwinfo();
    void loadMonitorInfoFromXrandr();
    void loadMonitorRefreshRateFromXrandr();

    /**
     * @brief:获取主板信息，获取主板信息的命令有
     *      sudo dmidecode -t 0  // BIOS
     *      lspci | grep ISA
     * @brief[loadBiosInfoFromDmidecode]:从 sudo dmidecode -t 0 中获取信息
     * @brief[loadBiosInfoFromLspci]:从 lspci 中获取信息,用于获取芯片组，但是龙芯的不是在这获取
     * @brief[loadBiosInfoFromCatBoardinfo]:从 cat /proc/boardinfo 中获取信息  这个仅仅用龙芯机器
     */
    void loadSystemInfoFromDmidecode();
    void loadMotherBoardInfoFromDmidecode();
    void loadBiosInfoFromDmidecode();
    void loadChassisInfoFromDmidecode();
    void loadMemoryInfoFromDmidecode();
    void loadBiosInfoFromLspci(QString &chipsetFamliy);
    void loadBiosInfoFromCatBoardinfo(const QString chipsetFamliy);

    /**
     * @brief:获取音频设备,获取音频设备的命令主要有
     *      sudo lshw -C multimedia
     *      sudo hwinfo --sound
     * @brief[loadAudioInfoFromLshw]:从lshw中获取信息
     * @brief[loadAudioInfoFromHwinfo]:从hwinfo中获取信息
     */
    void loadAudioInfoFromLshw();
    void loadAudioInfoFromHwinfo();

    /**
     * @brief:获取图像设备,获取图像设备的命令有
     *      sudo lshw -C multimedia
     *      sudo hwinfo --usb  注意这里不能用 --camera 因为 --camera不能获取外接摄像头
     */
    void loadImageInfoFromLshw();
    void loadImageInfoFromHwinfo();


    /**
     * @brief:获取蓝牙设备信息,获取蓝牙设备信息的命令有
     *      sudo hwinfo --usb   注意这里要用 --usb 不能用 --bluetooth
     *      sudo lshw -C communication -C generic
     * @brief[loadBlueToothInfoFromHwinfo]:从hwinfo中获取信息
     * @brief[loadBluetoothInfoFromLshw]:从lshw中获取信息
     */
    void loadBluetoothInfoFromHciconfig();
    void loadBlueToothInfoFromHwinfo();
    void loadBluetoothInfoFromLshw();

    /**
     * @brief:获取网络信息，获取网络信息的命令有
     *      sudo lshw -C network
     *      sudo hwinfo --network
     */
    void loadNetworkInfoFromLshw();
    void loadNetworkInfoFromHwinfo();

    /**
     * @brief:获取键盘信息，获取键盘信息的命令有
     *      cat /proc/bus/input/devices
     *      sudo lshw -C input
     *      sudo hwinfo --keyboard
     */
    void loadKeyboardInfoFromCatDevices();
    void loadKeyboardInfoFromLshw();
    void loadKeyboardInfoFromHwinfo();

    /**
     * @brief:获取其它设备信息,获取其他设备信息的命令有
     *      sudo lshw -C storage
     *      sudo lshw -C generic
     */
    void loadOtherDevicesFromLshwStorage();
    void loadOtherDevicesFromLshwGeneric();
    void loadOtherDevicesFromHwinfo();

    /**@brief:获取电源信息*/
    void loadPowerInfoFromUpower();

    /**
     * @brief:获取cdrom设备信息
     */
    void loadCdromInfoFromHwinfo();
    void loadCdromInfoFromLshw();

    /**
     * @brief:获取打印机信息
     */
    void loadPrinterInfoFromCups();
    void loadPrinterInfoFromHwinfo();

    void getMapInfoFromHwinfo(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));
    void getMapInfoFromLshw(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));
    void getMapInfoFromDmidecode(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));
    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));
public:
    bool getRootPassword();
    bool executeProcess(const QString &cmd);
    bool runCmd(const QString &cmd);
    bool runCmd(const QStringList &cmdList);

    QString standOutput_;

    // 由于同一个命令会执行多次，下面的变量是防止同一个命令执行多次
    static QString    s_lshwInput;          // sudo lshw -C input
    static QString    s_lshwSystem;         // sudo lshw -C system
    static QString    s_lshwGeneric;        // sudo lshw -C generic
    static QString    s_lshwDisk;           // sudo lshw -C disk
    static QString    s_lshwMemory;         // sudo lshw -C memory
    static QString    s_catDevice;          // cat /proc/bus/input/devices
    static QString    s_dmidecodeBaseboard; // sudo dmidecode -t baseboard
    static QString    s_xrandr;             // xrandr
    static QString    s_hwinfoUsb;          // hwinfo --usb
    static QMap<QString, QString>  s_hwinfoSound;

    static QStringList s_usbDeiveUniq;      //hwinfo --usb unique ID
};

