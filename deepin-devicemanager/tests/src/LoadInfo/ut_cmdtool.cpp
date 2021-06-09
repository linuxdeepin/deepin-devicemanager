/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../src/LoadInfo/CmdTool.h"
#include "../src/LoadInfo/DeviceFactory.h"
#include "../src/ThreadPool/GenerateDevicePool.h"
#include "../src/DBus/DBusInterface.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class CmdTool_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_cmdTool = new CmdTool;
    }
    void TearDown()
    {
        delete m_cmdTool;
    }
    CmdTool *m_cmdTool = nullptr;
};

TEST_F(CmdTool_UT, CmdTool_UT_addMapInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "model");
    m_cmdTool->addMapInfo("Model", mapInfo);
}

TEST_F(CmdTool_UT, CmdTool_UT_loadSmartCtlInfo)
{
    QMap<QString, QString> mapInfo;
    m_cmdTool->loadSmartCtlInfo("/", "/");
}

bool ut_isEmpty()
{
    return false;
}

TEST_F(CmdTool_UT, CmdTool_UT_loadCmdInfo)
{
    Stub stub;
    stub.set(ADDR(QString, isEmpty), ut_isEmpty);
    m_cmdTool->loadCmdInfo("lshw", "Model");
    m_cmdTool->loadCmdInfo("lsblk_d", "Model");
    m_cmdTool->loadCmdInfo("ls_sg", "Model");
    m_cmdTool->loadCmdInfo("printer", "Model");
    m_cmdTool->loadCmdInfo("hciconfig", "Model");
    m_cmdTool->loadCmdInfo("upower", "Model");
    m_cmdTool->loadCmdInfo("hwinfo", "Model");
    m_cmdTool->loadCmdInfo("dmidecode", "Model");
    m_cmdTool->loadCmdInfo("cat_devices", "Model");
    m_cmdTool->loadCmdInfo("gpuinfo", "Model");
    m_cmdTool->loadCmdInfo("cat_audio", "Model");
    m_cmdTool->loadCmdInfo("hciconfig", "Model");
    m_cmdTool->loadCmdInfo("EDID_HDMI", "Model");
    m_cmdTool->loadCmdInfo("bootdevice", "Model");
    m_cmdTool->loadCmdInfo("bt_device", "Model");
    m_cmdTool->loadCmdInfo("lscpu", "Model");
    m_cmdTool->loadCmdInfo("dmesg", "Model");
    m_cmdTool->loadCmdInfo("dr_config", "Model");
    m_cmdTool->loadCmdInfo("dr", "Model");
}

bool ut_startsWith()
{
    return true;
}

bool ut_CmdTool_exacMatch()
{
    return false;
}

bool ut_CmdTool_exacMatch1()
{
    return true;
}

TEST_F(CmdTool_UT, CmdTool_UT_loadXrandrInfo)
{
    Stub stub;
    stub.set((bool (QString::*)(QChar, Qt::CaseSensitivity) const)ADDR(QString, startsWith), ut_startsWith);
    stub.set(ADDR(QRegExp, exactMatch), ut_CmdTool_exacMatch1);
    m_cmdTool->loadXrandrInfo("/");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadXrandrVerboseInfo)
{
    Stub stub;
    //    stub.set((bool (QString::*)(QChar,Qt::CaseSensitivity) const)ADDR(QString,startsWith),ut_startsWith);
    stub.set(ADDR(QRegExp, exactMatch), ut_CmdTool_exacMatch);
    m_cmdTool->loadXrandrVerboseInfo("/");
    m_cmdTool->loadDmesgInfo("/");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadBiosInfoFromLspci)
{
    QString chipsetFamliy = "/n";
    m_cmdTool->loadBiosInfoFromLspci(chipsetFamliy);
}

TEST_F(CmdTool_UT, CmdTool_UT_loadBluetoothCtlInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("BD Address", "model");
    m_cmdTool->loadBluetoothCtlInfo(mapInfo);
}

TEST_F(CmdTool_UT, CmdTool_UT_loadHwinfoUsbInfo)
{
    QList<QMap<QString, QString>> list;
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    list.append(mapInfo);
    m_cmdTool->m_cmdInfo.insert("hwinfo_usb", list);
    m_cmdTool->loadHwinfoUsbInfo("Printer", mapInfo);
}

TEST_F(CmdTool_UT, CmdTool_UT_loadDmidecode2Info)
{
    m_cmdTool->loadDmidecode2Info("SMBIOS Version", "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_getSMBIOSVersion)
{
    QString ver = "5.0.6";
    m_cmdTool->getSMBIOSVersion("SMBIOS\n", ver);
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromCmd)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromCmd("SMBIOS\n", mapInfo, "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromInput)
{
    QList<QMap<QString, QString>> list;
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusID", "1");
    QMap<QString, QString> mapInfo1;
    mapInfo1.insert("SysFS BusID", "2");
    list.append(mapInfo);
    list.append(mapInfo1);
    m_cmdTool->m_cmdInfo.insert("hwinfo_usb", list);

    m_cmdTool->getMapInfoFromInput("SMBIOS\n", mapInfo, "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromLshw)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromLshw("SMBIOS\n", mapInfo, "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromHwinfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromHwinfo("SMBIOS\nSys", mapInfo, "IO");
}

TEST_F(CmdTool_UT, CmdTool_UT_addWidthToMap)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS ID", "BUS");
    m_cmdTool->addWidthToMap(mapInfo);
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromDmidecode)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromDmidecode("SMBIOS\nSys", mapInfo, "IO");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromSmartctl)
{
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_CmdTool_exacMatch);
    stub.set((bool (QString::*)(QChar, Qt::CaseSensitivity) const)ADDR(QString, endsWith), ut_startsWith);
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromSmartctl(mapInfo, "Power_On_Hours\n Multi_Zone_Error_Rate\n UDMA_CRC_Error_Count Offline_Uncorrectable\n Current_Pending_Sector\n Reallocated_Event_Count\n (bus Power_Cycle_Count)", "");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromHciconfig)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromHciconfig(mapInfo, "IO\nBUS MODEL");
}

bool ut_getDeviceInfo(QString deviceInfo, QString file)
{
    file = "lshw_cpu.txt";
    deviceInfo = "/*-cpu*-disk*-storage*-bank*-diaplay*-multimedia*-network*-usb*-cdrom";
    return true;
}

QStringList ut_lshw_split()
{
    return QStringList() << "cpu"
                         << "disk"
                         << "storage"
                         << "bank"
                         << "display"
                         << "multimedia"
                         << "network"
                         << "usb"
                         << "cdrom";
}

bool ut_lshw_startsWith()
{
    return true;
}

TEST_F(CmdTool_UT, CmdTool_UT_loadLshwInfo)
{
    Stub stub;
    stub.set((QStringList(QString::*)(QChar, QString::SplitBehavior, Qt::CaseSensitivity) const)ADDR(QString, split), ut_lshw_split);
    stub.set((bool (QString::*)(QChar, Qt::CaseSensitivity) const)ADDR(QString, startsWith), ut_lshw_startsWith);
    m_cmdTool->loadLshwInfo("lshw_cpu.txt");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadHwinfoInfo)
{
    Stub stub;
    stub.set(ADDR(QString, isEmpty), ut_isEmpty);
    m_cmdTool->loadHwinfoInfo("hwinfo_monitor", "hwinfo_monitor.txt");
}

bool ut_getCatDeviceInfo()
{
    return true;
}

TEST_F(CmdTool_UT, CmdTool_UT_loadCatInputDeviceInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_getCatDeviceInfo);
    stub.set(ADDR(QString, isEmpty), ut_isEmpty);
    stub.set(ADDR(QRegExp, exactMatch), ut_CmdTool_exacMatch);
    m_cmdTool->loadCatInputDeviceInfo("/", "/");
    m_cmdTool->loadCatConfigInfo("/", "/");
    m_cmdTool->loadCatAudioInfo("/", "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_getCurNetworkLinkStatus)
{
    m_cmdTool->getCurNetworkLinkStatus("en2ps0");
}

TEST_F(CmdTool_UT, CmdTool_UT_getCurPowerInfo)
{
    m_cmdTool->getCurPowerInfo();
}

DSysInfo::UosEdition ut_CmdTool_uosEditionType()
{
    return DSysInfo::UosHome;
}

TEST_F(CmdTool_UT, CmdTool_UT_loadHciconfigInfo)
{
    Stub stub;
    stub.set(ADDR(DSysInfo, uosEditionType), ut_CmdTool_uosEditionType);
    m_cmdTool->loadHciconfigInfo("/");
}

bool ut_CmdTool_getDeviceInfo()
{
    return false;
}

TEST_F(CmdTool_UT, CmdTool_UT_getDeviceInfo)
{
    Stub stub;
    stub.set(ADDR(DBusInterface, getInfo), ut_CmdTool_getDeviceInfo);
    const QString info = "/";
    QString it = "/";
    m_cmdTool->getDeviceInfo(it, info);
}

TEST_F(CmdTool_UT, CmdTool_UT_loadDmidecodeInfo)
{
    Stub stub;
    stub.set(ADDR(QString, isEmpty), ut_isEmpty);
    m_cmdTool->loadDmidecodeInfo("dmidecode2", "/");
    m_cmdTool->loadDmidecodeInfo("dmidecode3", "/");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadCatInfo)
{
    Stub stub;
    stub.set(ADDR(CmdTool, getCatDeviceInfo), ut_getCatDeviceInfo);
    m_cmdTool->loadCatInfo("dmidecode2", "/");
}
