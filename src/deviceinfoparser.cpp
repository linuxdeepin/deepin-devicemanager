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

#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>
#include <com_deepin_daemon_power.h>
#include "commondefine.h"
#include "deviceattributedefine.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMouse.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceKeyboard.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"
#include <DApplication>
#include <DSysInfo>
//#include "logpasswordauth.h"
#include "DMessageBox"
#include <cups.h>
#include "Logger.h"

QString DeviceInfoParser::s_catOsRelease = "";
QString DeviceInfoParser::s_catVersion = "";
QString DeviceInfoParser::s_catBoardInfo = "";
QString DeviceInfoParser::s_catDevice = "";
QString DeviceInfoParser::s_catCpuInfo = "";

QString DeviceInfoParser::s_dmidecode0 = "";
QString DeviceInfoParser::s_dmidecode1 = "";
QString DeviceInfoParser::s_dmidecode2 = "";
QString DeviceInfoParser::s_dmidecode3 = "";
QString DeviceInfoParser::s_dmidecode4 = "";
QString DeviceInfoParser::s_dmidecode16 = "";
QString DeviceInfoParser::s_dmidecode17 = "";

QString DeviceInfoParser::s_lshwSystem = "";
QString DeviceInfoParser::s_lshwInput = "";
QString DeviceInfoParser::s_lshwProcessor = "";
QString DeviceInfoParser::s_lshwDisk = "";
QString DeviceInfoParser::s_lshwDisplay = "";
QString DeviceInfoParser::s_lshwMemory = "";
QString DeviceInfoParser::s_lshwNetwork = "";
QString DeviceInfoParser::s_lshwMultimedia = "";
QString DeviceInfoParser::s_lshwCommunication = "";
QString DeviceInfoParser::s_lshwGeneric = "";
QString DeviceInfoParser::s_lshwStorage = "";

QString DeviceInfoParser::s_hwinfoMouse = "";
QString DeviceInfoParser::s_lscpu = "";
QString DeviceInfoParser::s_lsblkD = "";
QString DeviceInfoParser::s_xrandr = "";
QString DeviceInfoParser::s_xrandrVerbose = "";
QString DeviceInfoParser::s_dmesg = "";

QString DeviceInfoParser::s_hwinfoMonitor = "";
QString DeviceInfoParser::s_hwinfoSound = "";
QString DeviceInfoParser::s_hwinfoUsb = "";
QString DeviceInfoParser::s_hwinfoHub = "";
QString DeviceInfoParser::s_hwinfoNetwork = "";
QString DeviceInfoParser::s_hwinfoKeyboard = "";
QString DeviceInfoParser::s_hwinfoCdrom = "";
QString DeviceInfoParser::s_hwinfoDisk = "";
QString DeviceInfoParser::s_hwinfoDisplay = "";
QStringList    DeviceInfoParser::s_usbDeiveUniq = {};
QStringList    DeviceInfoParser::s_usbDevicebus = {};

DWIDGET_USE_NAMESPACE

using PowerInter = com::deepin::daemon::Power;

DCORE_USE_NAMESPACE
//const QString g_lsblkDbKey = "lsblk";
DeviceInfoParser::DeviceInfoParser(): QObject()
{

}

DeviceInfoParser::~DeviceInfoParser()
{

}

void DeviceInfoParser::clear()
{
    //s_catOsRelease = "";
    //s_catVersion = "";
    //s_catBoardInfo = "";
    s_catDevice = "";
    //s_catCpuInfo = "";

    //s_dmidecode0 = "";
    //s_dmidecode1 = "";
    //s_dmidecode2 = "";
    //s_dmidecode3 = "";
    //s_dmidecode4 = "";
    //s_dmidecode16 = "";
    //s_dmidecode17 = "";

    //s_lshwSystem = "";
    s_lshwInput = "";
    //s_lshwProcessor = "";
    s_lshwDisk = "";
    s_lshwDisplay = "";
    //s_lshwMemory = "";
    s_lshwMultimedia = "";
    s_lshwCommunication = "";
    s_lshwGeneric = "";
    //s_lshwNetwork = "";
    s_lshwStorage = "";

    s_hwinfoMouse = "";
    //s_lscpu = "";
    s_lsblkD = "";
    s_xrandr = "";
    s_xrandrVerbose = "";
    s_dmesg = "";

    s_hwinfoMonitor = "";
    s_hwinfoSound = "";
    s_hwinfoUsb = "";
    //s_hwinfoNetwork = "";
    s_hwinfoKeyboard = "";
    s_hwinfoCdrom = "";
    s_hwinfoDisk = "";
    s_hwinfoDisplay = "";

    QString defaultLanguage = getenv("LANGUAGE");
    setenv("LANGUAGE", "en_US", 1);
    setenv("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
}
void DeviceInfoParser::loadComputerInfo()
{
    if (DeviceManager::instance()->getComputerDevices().size() > 0) {
        return;
    }
    loadComputerInfoFromCatOsRelease();
    loadComputerInfoFromCatBoardinfo();
    loadUOS();
    loadComputerInfoFromCatVersion();
    loadComputerInfoFromDmidecode();
    loadComputerInfoFromLshw();
}
void DeviceInfoParser::loadComputerInfoFromCatOsRelease()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /etc/os-release"), s_catOsRelease, "cat_os_release.txt")) {
        return;
    }

    DeviceComputer device;
    device.setInfoFromCatOsRelease(s_catOsRelease);
    DeviceManager::instance()->addComputerDevice(device);
}
void DeviceInfoParser::loadUOS()
{
    QString os = "UOS";
    DSysInfo::DeepinType type = DSysInfo::deepinType();
    if (DSysInfo::DeepinProfessional == type) {
        os =  "UOS 20";
    } else if (DSysInfo::DeepinPersonal == type) {
        os =  "UOS 20 Home";
    } else if (DSysInfo::DeepinDesktop == type) {
        os =  "Deepin 20 Beta";
    }
    DeviceManager::instance()->setComputerUOSInfoFromDSysInfo(os);
}
void DeviceInfoParser::loadComputerInfoFromCatVersion()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /proc/version"), s_catVersion, "cat_version.txt")) {
        return;
    }
    DeviceManager::instance()->setComputerInfoFromCatVersion(s_catVersion);
}

void DeviceInfoParser::loadComputerInfoFromDmidecode()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo dmidecode -t 3"), s_dmidecode3, "dmidecode_3.txt")) {
        return;
    }
    DeviceManager::instance()->setComputerChassisInfoFromDmidecode(s_dmidecode3);
}

void DeviceInfoParser::loadComputerInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C system"), s_lshwSystem, "lshw_system.txt")) {
        return;
    }

    // 解析设备信息
    QStringList paragraphs = s_lshwSystem.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.contains("-pc")) {
            DeviceManager::instance()->setComputerInfoFromLshw(paragraph);
            break;
        }
    }
}

void DeviceInfoParser::loadComputerInfoFromCatBoardinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /proc/boardinfo"), s_catBoardInfo, "cat_boardinfo.txt")) {
        return;
    }

    QStringList paragraphs = s_catBoardInfo.split("\n\n");
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        if (paragraph.contains("Base Board Information")) {
            DeviceManager::instance()->setComputerBoardInfoFromCatBoardinfo(paragraph);
            break;
        }
    }
}

void DeviceInfoParser::loadMouseInfo()
{

    // 第二步从 sudo hwinfo --mouse 中获取信息
    loadMouseInfoFromHwinfo();

    // 第三步从 sudo lshw -C input       // 这个命令在x86机器对于ps2不支持
    loadMouseInfoFromlshw();

    // 第一步先从 cat /proc/bus/input/devices
    loadMouseInfoFromCatDevices();
}

bool DeviceInfoParser::loadMouseInfoFromCatDevices()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /proc/bus/input/devices"), s_catDevice, "cat_devices.txt")) {
        return false;
    }


    QStringList listInfo = s_catDevice.split(QString("\n\n"));
    for (QStringList::iterator it = listInfo.begin(); it != listInfo.end(); ++it) {

        if ((*it).isEmpty() || !(*it).contains(QString("Mouse"))) {
            continue;
        }
        DeviceManager::instance()->setCatDevicesIntoMouseDevice(*it);
    }
    return true;
}
bool DeviceInfoParser::loadMouseInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --mouse"), s_hwinfoMouse, "hwinfo_mouse.txt")) {
        return false;
    }

    // 解析设备信息
    QStringList listInfo = s_hwinfoMouse.split(QString("\n\n"));
    for (QStringList::iterator it = listInfo.begin(); it != listInfo.end(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }

        // 将从hwinfo --mouse获取的信息添加到相应设备
        DeviceMouse mouseDevice;
        mouseDevice.setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMouseDevice(mouseDevice);
    }
    return true;
}
bool DeviceInfoParser::loadMouseInfoFromlshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C input"), s_lshwInput, "lshw_input.txt")) {
        return false;
    }

    // 解析设备信息
    QStringList listInfo = s_lshwInput.split(QString("*-"));
    for (QStringList::iterator it = listInfo.begin(); it != listInfo.end(); ++it) {
        if ((*it).isEmpty()) {
            continue;
        }

        // 将从sudo lshw -C input获取的信息添加到相应设备
        DeviceManager::instance()->setLshwinfoIntoMouseDevice(*it);
    }
    return true;
}


void DeviceInfoParser::loadCpuInfo()
{
    if (DeviceManager::instance()->getCPUDevices().size() > 0) {
        return;
    }
    QMap<QString, QString> mapLscpu, mapLshw, mapDmidecode;
    loadCpuInfoFromLscpu(mapLscpu);
    loadCpuInfoFromLshw(mapLshw);
    loadCpuInfoFromDmidecode(mapDmidecode);
    loadCpuInfoFromCatCpuinfo(mapLscpu, mapLshw, mapDmidecode);
}
void DeviceInfoParser::loadCpuInfoFromLscpu(QMap<QString, QString> &mapLscpu)
{
    // 获取设备信息
    if (!getDeviceInfo(QString("lscpu"), s_lscpu, "lscpu.txt")) {
        return;
    }

    QStringList lines = s_lscpu.split(QString("\n"));
    foreach (QString line, lines) {
        if (line.isEmpty()) {
            continue;
        }

        QStringList words = line.split(": ");
        if (words.size() != 2) {
            continue;
        }

        mapLscpu.insert(words[0].trimmed(), words[1].trimmed());
    }
}


void DeviceInfoParser::loadCpuInfoFromLshw(QMap<QString, QString> &mapLshw)
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C processor"), s_lshwProcessor, "lshw_processor.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwProcessor.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        getMapInfoFromLshw(mapLshw, paragraph);
        break;
    }


}
void DeviceInfoParser::loadCpuInfoFromDmidecode(QMap<QString, QString> &mapDmidecode)
{
    if (!getDeviceInfo(QString("sudo dmidecode -t 4"), s_dmidecode4, "dmidecode_4.txt")) {
        return;
    }

    QStringList paragraphs = s_dmidecode4.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.contains("Processor Information")) {
            continue;
        }
        getMapInfoFromDmidecode(mapDmidecode, paragraph);
    }
}

void DeviceInfoParser::loadCpuInfoFromCatCpuinfo(const QMap<QString, QString> &mapLscpu, const QMap<QString, QString> &mapLshw, const QMap<QString, QString> &mapDmidecode)
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /proc/cpuinfo"), s_catCpuInfo, "cat_cpuinfo.txt")) {
        return;
    }

    QStringList paragraphs = s_catCpuInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            return;
        }

        DeviceCpu deviceCpu;
        deviceCpu.setCpuInfo(mapLscpu, mapLshw, mapDmidecode, paragraph);
        DeviceManager::instance()->addCpuDevice(deviceCpu);

    }
}

void DeviceInfoParser::loadDiskInfo()
{
    loadDiskInfoFromHwinfo();

    QStringList logicNameList;
    // 从 lsblk -d -o name,rota 中获取介质类型
    loadDiskInfoFromLsblk(logicNameList);

    // 从 lshw -C disk 中获取设备属性
    loadDiskInfoFromLshw();

    // 从 smartctl 中获取设备属性
    loadDiskInfoFromSmartCtl(logicNameList);
}
void DeviceInfoParser::loadDiskInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --disk"), s_hwinfoDisk, "hwinfo_disk.txt")) {
        return;
    }
    QStringList paragraphs = s_hwinfoDisk.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            return;
        }

        DeviceStorage device;
        if (device.setHwinfoInfo(paragraph) && device.isValid()) {
            DeviceManager::instance()->addStorageDeivce(device);
        }
    }
}
void DeviceInfoParser::loadDiskInfoFromLsblk(QStringList &logicNameList)
{
    // 获取设备信息
    if (!getDeviceInfo(QString("lsblk -d -o name,rota"), s_lsblkD, "lsblk_d.txt")) {
        return;
    }

    QStringList lines = s_lsblkD.split("\n");
    foreach (QString line, lines) {
        QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
        if (words.size() != 2) {
            continue;
        }

        bool ret = DeviceManager::instance()->setStorageDeviceMediaType(words[0].trimmed(), words[1].trimmed());

        // 设置介质类型成功,表示该磁盘存在
        if (ret) {
            logicNameList.append(words[0].trimmed());
        }
    }
}

void DeviceInfoParser::loadDiskInfoFromLshw()
{
    // 获取设备信息
    qint64 beginDT = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 curDT = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int diskNum = DeviceManager::instance()->getStorageDevices().size();
    QStringList paragraphs;
    while (diskNum > paragraphs.size() && curDT - beginDT < 5000) {
        if (s_lshwDisk.isEmpty()) {
            if (!getDeviceInfo(QString("sudo lshw -C disk"), s_lshwDisk, "lshw_disk.txt")) {
                return;
            }
        }
        paragraphs = s_lshwDisk.split(QString("*-"));
        curDT = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }

    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        if (!paragraph.contains("disk")) {
            continue;
        }
        // 将从sudo lshw -C disk获取的信息添加到相应设备
        DeviceManager::instance()->addLshwinfoIntoStorageDevice(paragraph);
    }
}

void DeviceInfoParser::loadDiskInfoFromSmartCtl(QStringList &logicNameList)
{
    // smartctl --all /dev/nvme0n1
    // smartctl --all /dev/sda

    foreach (auto logicName, logicNameList) {
        QString diskName = QString("/dev/%1").arg(logicName);
        // 获取设备信息
        QString deviceInfo;
        if (!getDeviceInfo(QString("sudo smartctl --all %1").arg(diskName), deviceInfo, QString("smartctl_%1.txt").arg(logicName))) {
            break;
        }
        DeviceManager::instance()->setStorageInfoFromSmartctl(logicName, deviceInfo);
    }
}

void DeviceInfoParser::loadGpuInfo()
{
//    if (DeviceManager::instance()->getGPUDevices().size() > 0) {
//        return;
//    }
    loadGpuInfoFromHwinfo();
    loadGpuInfoFromLshw();
    loadGpuInfoFromXrandr();
    loadGpuSizeFromDmesg();
}

void DeviceInfoParser::loadGpuInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C display"), s_lshwDisplay, "lshw_display.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwDisplay.split(QString("*-display"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.trimmed().isEmpty()) {
            continue;
        }
        DeviceManager::instance()->setGpuInfoFromLshw(paragraph);
    }
}

void DeviceInfoParser::loadGpuInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --display"), s_hwinfoDisplay, "hwinfo_display.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoDisplay.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }

        // 过滤信息
        if (!paragraph.contains("Unique ID", Qt::CaseInsensitive)) {
            continue;
        }

        DeviceGpu deviceGpu;
        deviceGpu.setHwinfoInfo(paragraph);
        DeviceManager::instance()->addGpuDevice(deviceGpu);
    }
}

//可以从xrandr命令中获取最大、最小、当前分辨率，同时可以获得刷新率
void DeviceInfoParser::loadGpuInfoFromXrandr()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("xrandr"), s_xrandr, "xrandr.txt")) {
        return;
    }

    DeviceManager::instance()->setGpuInfoFromXrandr(s_xrandr);
}

void DeviceInfoParser::loadGpuSizeFromDmesg()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo dmesg"), s_dmesg, "dmesg.txt")) {
        return;
    }
    QStringList lines = s_dmesg.split("\n");
    foreach (const QString &line, lines) {
        QRegExp reg(".*RAM=([0-9]*)M.*");
        if (reg.exactMatch(line)) {
            double size = reg.cap(1).toDouble();
            QString sizeS = QString("%1GB").arg(size / 1024);
            DeviceManager::instance()->setGpuSizeFromDmesg(sizeS);
        } else {
            reg.setPattern(".*RAM: ([0-9]*) M.*");
            if (reg.exactMatch(line)) {
                double size = reg.cap(1).toDouble();
                QString sizeS = QString("%1GB").arg(size / 1024);
                DeviceManager::instance()->setGpuSizeFromDmesg(sizeS);
            }
        }
    }
}

void DeviceInfoParser::loadMemoryInfo()
{
    if (DeviceManager::instance()->getMemoryDevices().size()) {
        return;
    }
    loadMemoryFromLshw();
    loadMemoryFromDmidecode();
}
void DeviceInfoParser::loadMemoryFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C memory"), s_lshwMemory, "lshw_memory.txt")) {
        return;
    }

    bool withBank = false;
    if (s_lshwMemory.contains("bank")) {
        withBank = true;
    }

    QStringList paragraphs = s_lshwMemory.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        if (withBank) {
            if (!paragraph.startsWith("bank")) {
                continue;
            }
            if (paragraph.contains("description: DIMM [empty]")
                    || paragraph.contains("description: [empty]")
                    || paragraph.contains("description: DDR4 [empty]")) {
                continue;
            }
        } else { // 这里是针对龙芯做的特殊处理
            if (!paragraph.startsWith("memory")) {
                continue;
            }
        }
        DeviceMemory memoryDevice;
        memoryDevice.setInfoFromLshw(paragraph);
        DeviceManager::instance()->addMemoryDevice(memoryDevice);
    }
}
void DeviceInfoParser::loadMemoryFromDmidecode()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo dmidecode -t 17"), s_dmidecode17, "dmidecode_17.txt")) {
        return;
    }

    QStringList paragraphs = s_dmidecode17.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.contains("Memory Device")) {
            continue;
        }
        if (paragraph.contains("No Module Installed")) {
            continue;
        }

        DeviceManager::instance()->setMemoryInfoFromDmidecode(paragraph);
    }
}

void DeviceInfoParser::loadMonitorInfo()
{
    loadMonitorInfoFromHwinfo();
    loadMonitorInfoFromXrandr();
    loadMonitorRefreshRateFromXrandr();
}
void DeviceInfoParser::loadMonitorInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --monitor"), s_hwinfoMonitor, "hwinfo_monitor.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoMonitor.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {

        if (paragraph.isEmpty()) {
            continue;
        }

        DeviceMonitor monitor;
        monitor.setInfoFromHwinfo(paragraph);
        DeviceManager::instance()->addMonitor(monitor);
    }
}

void DeviceInfoParser::loadMonitorInfoFromXrandr()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("xrandr --verbose"), s_xrandrVerbose, "xrandr_verbose.txt")) {
        return;
    }

    QStringList lines = s_xrandrVerbose.split(QRegExp("\n"));
    QString mainInfo("");
    QString edid("");
    foreach (QString line, lines) {
        if (line.startsWith("Screen")) {
            continue;
        }
        QRegExp reResolution("^[\\s]{2}([0-9]{3,4}x[0-9]{3,4}).*");
        if (reResolution.exactMatch(line)) {
            continue;
        }
        QRegExp reMain("^[a-zA-Z].*");
        if (reMain.exactMatch(line)) {
            if (!mainInfo.isEmpty()) {
                DeviceManager::instance()->setMonitorInfoFromXrandr(mainInfo.trimmed(), edid.trimmed());
            }
            mainInfo = line;
            edid = "";
            continue;
        }
        QRegExp reEdid("^[\\t]{2}[0-9]{1}.*");
        if (reEdid.exactMatch(line)) {
            edid.append(line.trimmed());
            edid.append("\n");
            continue;
        }
    }
    DeviceManager::instance()->setMonitorInfoFromXrandr(mainInfo.trimmed(), edid.trimmed());
}

void DeviceInfoParser::loadMonitorRefreshRateFromXrandr()
{
    // 获取设备信息
    if (s_xrandr.isEmpty()) {
        if (!getDeviceInfo(QString("xrandr"), s_xrandr, "xrandr.txt")) {
            return;
        }
    }

    QStringList lines = s_xrandr.split(QRegExp("\n"));
    QString mainInfo("");
    foreach (QString line, lines) {
        if (line.startsWith("Screen")) {
            continue;
        }
        QRegExp reResolution("^[\\s]{3}([0-9]{3,5}x[0-9]{3,5}).*([0-9]{2,3}.[0-9]{2,3}\\*).*");
        if (reResolution.exactMatch(line)) {
            QString rate = reResolution.cap(2).replace("*", "");
            DeviceManager::instance()->setMonitorRefreshRate(mainInfo, line);
        }
        QRegExp reMain("^[A-Z].*");
        if (reMain.exactMatch(line)) {
            if (!mainInfo.isEmpty()) {
            }
            mainInfo = line;
            continue;
        }
    }
}

void DeviceInfoParser::loadBiosInfo()
{
    if (DeviceManager::instance()->getBiosDevices().size() > 0) {
        return;
    }
    // 获取主办信息
    loadMotherBoardInfoFromDmidecode();

    // 获取bios信息
    loadBiosInfoFromDmidecode();

    // 获取系统信息
    loadSystemInfoFromDmidecode();

    // 获取机箱信息
    loadChassisInfoFromDmidecode();

    // 获取内存卡槽信息
    loadMemoryInfoFromDmidecode();
}
void DeviceInfoParser::loadSystemInfoFromDmidecode()
{
    if (!getDeviceInfo(QString("sudo dmidecode -t 1"), s_dmidecode1, "dmidecode_1.txt")) {
        return;
    }
    DeviceBios device;
    if (device.setSystemInfo(s_dmidecode1)) {
        DeviceManager::instance()->addBiosDevice(device);
    }
}
void DeviceInfoParser::loadMotherBoardInfoFromDmidecode()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo dmidecode -t 2"), s_dmidecode2, "dmidecode_2.txt")) {
        return;
    }
    QString chipsetFamliy = "";
    loadBiosInfoFromLspci(chipsetFamliy);
    DeviceBios device;
    if (device.setBaseBoardInfo(s_dmidecode2, chipsetFamliy)) {
        DeviceManager::instance()->addBiosDevice(device);
    } else {
        // 龙芯环境下无法通过dmidecode获取信息 需要通过 cat /proc/boardinfo 来获取信息
        loadBiosInfoFromCatBoardinfo(chipsetFamliy);
    }
}
void DeviceInfoParser::loadBiosInfoFromDmidecode()
{
    if (!getDeviceInfo(QString("sudo dmidecode -t 0"), s_dmidecode0, "dmidecode_0.txt")) {
        return;
    }
    DeviceBios device;
    if (device.setBiosInfo(s_dmidecode0)) {
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceInfoParser::loadChassisInfoFromDmidecode()
{
    if (!getDeviceInfo(QString("sudo dmidecode -t 3"), s_dmidecode3, "dmidecode_3.txt")) {
        return;
    }
    DeviceBios device;
    if (device.setChassisInfo(s_dmidecode3)) {
        DeviceManager::instance()->addBiosDevice(device);
    }
}

void DeviceInfoParser::loadMemoryInfoFromDmidecode()
{
    if (!getDeviceInfo(QString("sudo dmidecode -t 16"), s_dmidecode16, "dmidecode_16.txt")) {
        return;
    }
    DeviceBios device;
    if (device.setMemoryInfo(s_dmidecode16)) {
        DeviceManager::instance()->addBiosDevice(device);
    }
}


void DeviceInfoParser::loadBiosInfoFromLspci(QString &chipsetFamliy)
{
    // 获取设备信息
    QString command;
    QString deviceInfo;
    if (!getDeviceInfo(QString("lspci"), deviceInfo, "lspci.txt")) {
        return;
    }
    QStringList lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        QStringList words = line.split(" ");
        if (words.size() < 2) {
            continue;
        }
        if (words[1] == QString("ISA")) {
            command = QString("lspci -v -s %1").arg(words[0].trimmed());
        }
    }

    if (command.isEmpty()) {
        return;
    }
    if (!getDeviceInfo(command, deviceInfo, "")) {
        return;
    }
    lines = deviceInfo.split("\n");
    foreach (const QString &line, lines) {
        if (line.contains("Subsystem")) {
            QStringList words = line.split(": ");
            if (words.size() == 2) {
                chipsetFamliy = words[1].trimmed();
            }
            break;
        }
    }

}

void DeviceInfoParser::loadBiosInfoFromCatBoardinfo(const QString chipsetFamliy)
{
    // 获取设备信息
    if (!getDeviceInfo(QString("cat /proc/boardinfo"), s_catBoardInfo, "cat_boardinfo.txt")) {
        return;
    }
    QStringList paragraphs = s_catBoardInfo.split("\n\n");
    foreach (const QString &paragraph, paragraphs) {
        DeviceBios device;
        if (paragraph.contains("BIOS Information")) {
            if (device.setBiosInfo(paragraph)) {
                DeviceManager::instance()->addBiosDevice(device);
            }
        } else if (paragraph.contains("Base Board Information")) {
            if (device.setBaseBoardInfo(paragraph, chipsetFamliy)) {
                DeviceManager::instance()->addBiosDevice(device);
            }
        }
    }
}

void DeviceInfoParser::loadAudioInfo()
{
    loadAudioInfoFromHwinfo();
    loadAudioInfoFromLshw();
}
void DeviceInfoParser::loadAudioInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C multimedia"), s_lshwMultimedia, "lshw_multimedia.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwMultimedia.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {

        if (paragraph.isEmpty()) {
            continue;
        }
        if (!paragraph.contains("multimedia")) {
            continue;
        }

        DeviceManager::instance()->setAudioInfoFromLshw(paragraph);

    }
}
void DeviceInfoParser::loadAudioInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --sound"), s_hwinfoSound, "hwinfo_sound.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoSound.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {

        if (paragraph.isEmpty()) {
            continue;
        }

        DeviceAudio device;
        device.setInfoFromHwinfo(paragraph);
        DeviceManager::instance()->addAudioDevice(device);

    }
}

void DeviceInfoParser::loadImageInfo()
{
    loadImageInfoFromHwinfo();
    loadImageInfoFromLshw();
}
void DeviceInfoParser::loadImageInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C multimedia"), s_lshwMultimedia, "lshw_multimedia.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwMultimedia.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.contains("Camera", Qt::CaseInsensitive)) {
            continue;
        }
        DeviceManager::instance()->setCameraInfoFromLshw(paragraph);
    }
}
void DeviceInfoParser::loadImageInfoFromHwinfo()
{
    // 获取设备信息
    if (s_hwinfoUsb.isEmpty()) {
        if (!getDeviceInfo(QString("sudo hwinfo --usb"), s_hwinfoUsb, "hwinfo_usb.txt")) {
            return;
        }
    }

    QStringList paragraphs = s_hwinfoUsb.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        // 判断是否是蓝牙设备
        if (!paragraph.contains("Camera", Qt::CaseInsensitive)) {
            continue;
        }
        DeviceImage device;
        device.setInfoFromHwinfo(paragraph);
        DeviceManager::instance()->addImageDevice(device);
    }
}

void DeviceInfoParser::loadBluetoothInfo()
{
    loadBluetoothInfoFromHciconfig();
    loadBlueToothInfoFromHwinfo();
    loadBluetoothInfoFromLshw();
}
void DeviceInfoParser::loadBluetoothInfoFromHciconfig()
{
    // 获取设备信息
    QString deviceInfo;
    if (!getDeviceInfo(QString("hciconfig -a"), deviceInfo, "hciconfig.txt")) {
        return;
    }
    QStringList paragraphs = deviceInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        DeviceBluetooth device;
        device.setInfoFromHciconfig(paragraph);
        DeviceManager::instance()->addBluetoothDevice(device);
    }
}
void DeviceInfoParser::loadBlueToothInfoFromHwinfo()
{
    // 获取设备信息
    if (s_hwinfoUsb.isEmpty()) {
        if (!getDeviceInfo(QString("sudo hwinfo --usb"), s_hwinfoUsb, "hwinfo_usb.txt")) {
            return;
        }
    }

    QStringList paragraphs = s_hwinfoUsb.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        DeviceManager::instance()->setBluetoothInfoFromHwinfo(paragraph);
    }
}

void DeviceInfoParser::loadBluetoothInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C communication"), s_lshwCommunication, "lshw_communication.txt")) {
        return;
    }
    QStringList paragraphs = s_lshwCommunication.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.contains("usb@")) {
            continue;
        }
        DeviceManager::instance()->setBluetoothInfoFromLshw(paragraph);
    }


    if (!getDeviceInfo(QString("sudo lshw -C generic"), s_lshwGeneric, "lshw_generic.txt")) {
        return;
    }
    paragraphs = s_lshwGeneric.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.contains("usb@")) {
            continue;
        }
        DeviceManager::instance()->setBluetoothInfoFromLshw(paragraph);
    }
}

void DeviceInfoParser::loadNetworkInfo()
{
    if (DeviceManager::instance()->getNetworkDevices().size() > 0) {
        return;
    }
    loadNetworkInfoFromLshw();
}
void DeviceInfoParser::loadNetworkInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C network"), s_lshwNetwork, "lshw_network.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwNetwork.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty() || !paragraph.contains("description") || !paragraph.contains("product")) {
            continue;
        }

        DeviceNetwork device;
        device.setInfoFromLshw(paragraph);
        DeviceManager::instance()->addNetworkDevice(device);
    }

}

void DeviceInfoParser::loadNetworkInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --network"), s_hwinfoNetwork, "hwinfo_network.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoNetwork.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        DeviceManager::instance()->setNetworkInfoFromHwinfo(paragraph);
    }

}

void DeviceInfoParser::loadKeyboardInfo()
{
    loadKeyboardInfoFromHwinfo();
    loadKeyboardInfoFromLshw();
    loadKeyboardInfoFromCatDevices();
}
void DeviceInfoParser::loadKeyboardInfoFromCatDevices()
{
    // 获取设备信息
    if (s_catDevice.isEmpty()) {
        if (!getDeviceInfo(QString("cat /proc/bus/input/devices"), s_catDevice, "cat_devices.txt")) {
            return;
        }
    }

    QStringList paragraphs = s_catDevice.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        if (!paragraph.contains("Keyboard", Qt::CaseInsensitive) ||
                paragraph.contains("Mouse", Qt::CaseInsensitive)) {
            continue;
        }
        DeviceManager::instance()->setKeyboardInfoFromCatDevices(paragraph);
    }
}
void DeviceInfoParser::loadKeyboardInfoFromLshw()
{
    // 获取设备信息
    if (s_lshwInput.isEmpty()) {
        if (!getDeviceInfo(QString("sudo lshw -C input"), s_lshwInput, "lshw_input.txt")) {
            return;
        }
    }

    QStringList paragraphs = s_lshwInput.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty() || !paragraph.contains("Keyboard")) {
            continue;
        }

        DeviceManager::instance()->setKeyboardInfoFromLshw(paragraph);
    }
}
void DeviceInfoParser::loadKeyboardInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo hwinfo --keyboard"), s_hwinfoKeyboard, "hwinfo_keyboard.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoKeyboard.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {

        if (paragraph.isEmpty() || paragraph.contains("Control Driver", Qt::CaseInsensitive)) {
            continue;
        }
        DeviceKeyboard device;
        device.setInfoFromHwinfo(paragraph);
        DeviceManager::instance()->addKeyboardDevice(device);
    }
}

void DeviceInfoParser::loadOtherDevices()
{
    // 在获取其它设备之前必须先获取硬盘设备
    loadDiskInfo();
    loadPrinterInfoFromHwinfo();
    loadUSBHubInfoFromHwinfo();

//    loadOtherDevicesFromLshwStorage();
//    loadOtherDevicesFromLshwGeneric();
    loadOtherDevicesFromHwinfo();
}
void DeviceInfoParser::loadOtherDevicesFromLshwStorage()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C storage"), s_lshwStorage, "lshw_storage.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwStorage.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.contains("Mass storage device") || paragraph.contains("Human interface device")) {
            DeviceOthers device;
            device.setInfoFromLshw(paragraph);
            DeviceManager::instance()->addOthersDevice(device);
        }
    }

    //*****************
//    if (!getDeviceInfo(QString("sudo lshw -C communication"), s_lshwCommunication, "lshw_communication.txt")) {
//        return;
//    }
//    paragraphs = s_lshwCommunication.split(QString("*-"));
//    foreach (const QString &paragraph, paragraphs) {
//        if (paragraph.contains("Modem")) {
//            DeviceOthers device;
//            device.setInfoFromLshw(paragraph);
//            DeviceManager::instance()->addOthersDevice(device);
//        }
//    }
}
void DeviceInfoParser::loadOtherDevicesFromLshwGeneric()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C generic"), s_lshwGeneric, "lshw_generic.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwGeneric.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty() || !paragraph.contains("Generic USB device") || paragraph.contains("driver=btusb")) {
            continue;
        }
        DeviceOthers device;
        device.setInfoFromLshw(paragraph);
        DeviceManager::instance()->addOthersDevice(device);
    }
}

void DeviceInfoParser::loadOtherDevicesFromHwinfo()
{
    // 获取设备信息
    if (s_hwinfoUsb.isEmpty()) {
        if (!getDeviceInfo(QString("hwinfo --usb"), s_hwinfoUsb, "hwinfo_usb.txt")) {
            return;
        }
    }

    QStringList paragraphs = s_hwinfoUsb.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        /*if (paragraph.contains("Linux Foundation", Qt::CaseInsensitive) == false) */{
            DeviceOthers device;
            device.setInfoFromHwinfo(paragraph);
            if (device.isExist() == false) {
                DeviceManager::instance()->addOthersDeviceFromHwinfo(device);
                device.addUniqIDAndBusID();
            }
        }
    }
}

void DeviceInfoParser::loadPowerInfo()
{
    loadPowerInfoFromUpower();
}
void DeviceInfoParser::loadPowerInfoFromUpower()
{
    // 获取设备信息
    QString deviceInfo;
    if (!getDeviceInfo(QString("upower --dump"), deviceInfo, "upower_dump.txt")) {
        return;
    }

    QStringList paragraphs = deviceInfo.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (!paragraph.startsWith("Daemon")) {
            DevicePower device;
            device.setInfoFromUpower(paragraph);
            DeviceManager::instance()->addPowerDevice(device);
        } else if (paragraph.startsWith("Daemon")) {
            DeviceManager::instance()->setPowerDaemonInfo(paragraph);
        }
    }
}

void DeviceInfoParser::loadCdromInfo()
{
    loadCdromInfoFromHwinfo();
    loadCdromInfoFromLshw();
}
void DeviceInfoParser::loadCdromInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("hwinfo --cdrom"), s_hwinfoCdrom, "hwinfo_cdrom.txt")) {
        return;
    }
    QStringList paragraphs = s_hwinfoCdrom.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        DeviceCdrom device;
        device.setInfoFromHwinfo(paragraph);
        DeviceManager::instance()->addCdromDevice(device);
    }
}
void DeviceInfoParser::loadCdromInfoFromLshw()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("sudo lshw -C disk"), s_lshwDisk, "lshw_disk.txt")) {
        return;
    }

    QStringList paragraphs = s_lshwDisk.split(QString("*-"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.isEmpty()) {
            continue;
        }
        if (!paragraph.contains("cdrom")) {
            continue;
        }
        DeviceManager::instance()->setCdromInfoFromLshw(paragraph);
    }
}


void showDetailedInfo(cups_dest_t *dest, const char *option, QMap<QString, QString> &DeviceInfoMap)
{
    cups_dinfo_t *info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

    if (true == cupsCheckDestSupported(CUPS_HTTP_DEFAULT, dest, info, option, nullptr)) {
        ipp_attribute_t *finishings = cupsFindDestSupported(CUPS_HTTP_DEFAULT, dest, info, option);
        int i, count = ippGetCount(finishings);

        //std::cout << option << " support" << std::endl;
        for (i = 0; i < count; i ++) {
            if (option == CUPS_FINISHINGS || option == CUPS_COPIES /*|| option == CUPS_MEDIA */ || \
                    option == CUPS_NUMBER_UP || option == CUPS_ORIENTATION || option == CUPS_PRINT_QUALITY) {
                if (DeviceInfoMap.contains(option)) {
                    DeviceInfoMap[option] += ", ";
                    DeviceInfoMap[option] += QString::number(ippGetInteger(finishings, i));
                } else {
                    DeviceInfoMap[option] = QString::number(ippGetInteger(finishings, i));
                }

                //std::cout << option<<" : "<<ippGetInteger(finishings, i)  << std::endl;
            } else {
                if (DeviceInfoMap.contains(option)) {
                    DeviceInfoMap[option] += ", ";
                    DeviceInfoMap[option] += ippGetString(finishings, i, nullptr);
                } else {
                    DeviceInfoMap[option] = ippGetString(finishings, i, nullptr);
                }

                //std::cout << option<<" : " << ippGetString(finishings, i, nullptr) << std::endl;
            }
        }
    } else {
        //std::cout << option << " not supported." << std::endl;
    }
}
//获取相关打印机目标字段的信息
int getDestInfo(void *user_data, unsigned flags, cups_dest_t *dest)
{
    if (flags & CUPS_DEST_FLAGS_REMOVED) {
        return -1;
    }

    QMap<QString, QMap<QString, QString> > &cupsDatabase = *(reinterpret_cast<QMap<QString, QMap<QString, QString> > *>(user_data));

    if (dest == nullptr) {
        return -1;
    }

    QMap<QString, QString> DeviceInfoMap;

    showDetailedInfo(dest, CUPS_FINISHINGS, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_COPIES, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_MEDIA, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_MEDIA_SOURCE, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_MEDIA_TYPE, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_NUMBER_UP, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_ORIENTATION, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_PRINT_COLOR_MODE, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_PRINT_QUALITY, DeviceInfoMap);
    showDetailedInfo(dest, CUPS_SIDES, DeviceInfoMap);


    //cups_dinfo_t* info = cupsCopyDestInfo(CUPS_HTTP_DEFAULT, dest);

    for (int i = 0; i < dest->num_options; ++i) {
        DeviceInfoMap[(dest->options + i)->name] = (dest->options + i)->value;
        //std::cout << (dest->options + i)->name <<" : " << (dest->options + i)->value << std::endl;
    }

    cupsDatabase[dest->name] = DeviceInfoMap;

    return (1);
}

void DeviceInfoParser::loadPrinterInfo()
{
    loadPrinterInfoFromCups();
}

void DeviceInfoParser::loadPrinterInfoFromCups()
{
    QMap<QString, QMap<QString, QString> > info;
    //获得所有的句柄
    cupsEnumDests(CUPS_DEST_FLAGS_NONE, 1000, nullptr, CUPS_PRINTER_CLASS, 0, getDestInfo, &info);

    foreach (const QString &key, info.keys()) {
        DevicePrint device;
        info[key]["Name"] = key;
        device.setInfo(info[key]);
        DeviceManager::instance()->addPrintDevice(device);
    }
}
void DeviceInfoParser::loadPrinterInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("hwinfo --usb"), s_hwinfoUsb, "hwinfo_usb.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoUsb.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        if (paragraph.contains("Hardware Class: printer")) {
//            DeviceManager::instance()->setPrinterInfoFromHwinfo(paragraph);
            QMap<QString, QString> mapInfo;
            getMapInfoFromHwinfo(mapInfo, paragraph);
            if (mapInfo["Unique ID"].isEmpty() == false) {
                DeviceInfoParser::Instance().s_usbDeiveUniq.append(mapInfo["Unique ID"]);
                DeviceInfoParser::Instance().s_usbDevicebus.append(mapInfo["SysFS BusID"].replace(QRegExp("\\.[0-9]*$"), ""));
            }
        }
    }
}

void DeviceInfoParser::getMapInfoFromHwinfo(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() != 2) {
            continue;
        }
        if (mapInfo.find(words[0].trimmed()) != mapInfo.end()) {
            mapInfo[words[0].trimmed()] += QString(" ");
        }
        QRegExp re(".*\"(.*)\".*");
        if (re.exactMatch(words[1].trimmed())) {
            mapInfo[words[0].trimmed()] += re.cap(1);
        } else {
            if (words[0].trimmed() == "Resolution") {
                mapInfo[words[0].trimmed()] += words[1].trimmed();
            } else {
                mapInfo[words[0].trimmed()] = words[1].trimmed();
            }
        }
    }
}

void DeviceInfoParser::getMapInfoFromLshw(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2 && words[0].contains("configuration") == false) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }

        // 将configuration的内容进行拆分
        if (words[0].contains("configuration") == true) {
            QStringList keyValues = words[1].split(" ");

            for (QStringList::iterator it = keyValues.begin(); it != keyValues.end(); ++it) {
                QStringList attr = (*it).split("=");
                if (attr.size() != 2) {
                    continue;
                }

                mapInfo.insert(attr[0].trimmed(), attr[1].trimmed());
            }
        }
    }
}
void DeviceInfoParser::getMapInfoFromDmidecode(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
}


bool DeviceInfoParser::getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile)
{
    if (!deviceInfo.isEmpty()) {
        return true;
    }
//    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    if (false == executeProcess(command)) {
        return false;
    }
//    qint64 end = QDateTime::currentMSecsSinceEpoch();
//    qDebug() << command << " ******************************* " << (end - begin) / 1000.0;
    deviceInfo = standOutput_;
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    return true;
}

void DeviceInfoParser::loadUSBHubInfoFromHwinfo()
{
    // 获取设备信息
    if (!getDeviceInfo(QString("hwinfo --hub"), s_hwinfoHub, "hwinfo_hub.txt")) {
        return;
    }

    QStringList paragraphs = s_hwinfoHub.split(QString("\n\n"));
    foreach (const QString &paragraph, paragraphs) {
        QMap<QString, QString> mapInfo;
        getMapInfoFromHwinfo(mapInfo, paragraph);

        if (mapInfo["Unique ID"].isEmpty() == false) {
            DeviceInfoParser::Instance().s_usbDeiveUniq.append(mapInfo["Unique ID"]);
            DeviceInfoParser::Instance().s_usbDevicebus.append(mapInfo["SysFS BusID"].replace(QRegExp("\\.[0-9]*$"), ""));
        }
    }
}

bool DeviceInfoParser::getRootPassword()
{
    bool res = runCmd("id -un");  // file path is fixed. So write cmd direct
    if (res == true && standOutput_.trimmed() == "root") {
        return true;
    }

#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if (false == executeProcess("sudo whoami")) {
        return false;
    }

    return true;
}

bool DeviceInfoParser::executeProcess(const QString &cmd)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd);
    }

    runCmd("id -un");
    if (standOutput_.trimmed() == "root") {
        return runCmd(cmd);
    }

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd);
}

bool DeviceInfoParser::runCmd(const QString &proxy)
{
    QString key = "devicemanager";
    QString cmd = proxy;
    QProcess process_;
    int msecs = 10000;
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy")) {
        cmd = proxy + QString(" ") + key;
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    standOutput_ = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126)) {
        dError("Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");
        return false;
    }

    if (res == false) {
        dError("Run \'" + cmd + "\' failed\n");
    }

    return res;
}

bool DeviceInfoParser::runCmd(const QStringList &cmdList)
{
    QProcess process_;
    process_.start("/bin/bash", cmdList);
    bool res = process_.waitForFinished(10000);
    standOutput_ = process_.readAllStandardOutput();
    return res;
}
