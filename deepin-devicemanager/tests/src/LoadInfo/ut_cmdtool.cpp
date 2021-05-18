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

TEST_F(CmdTool_UT, CmdTool_UT_loadCmdInfo)
{
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
    m_cmdTool->loadCmdInfo("lscpu-", "Model");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadXrandrInfo)
{
    m_cmdTool->loadXrandrInfo("/");
}

TEST_F(CmdTool_UT, CmdTool_UT_loadXrandrVerboseInfo)
{
    m_cmdTool->loadXrandrVerboseInfo("/");
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
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
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
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
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
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromSmartctl(mapInfo, "Power_On_Hours\nbus Power_Cycle_Count Raw_Read_Error_Rate Spin_Up_Time Start_Stop_Count", "");
}

TEST_F(CmdTool_UT, CmdTool_UT_getMapInfoFromHciconfig)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("SysFS BusI", "model");
    m_cmdTool->getMapInfoFromHciconfig(mapInfo, "IO\nBUS MODEL");
}
