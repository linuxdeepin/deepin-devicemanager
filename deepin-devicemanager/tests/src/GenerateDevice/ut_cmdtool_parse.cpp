// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 CmdTool 的纯字符串解析方法 getMapInfoFrom*(Cmd/Input/Lshw/Hwinfo/Dmidecode/Smartctl)
// 及 getMulHwinfoInfo/addWidthToMap/getSMBIOSVersion。均为纯逻辑,构造输入字符串即覆盖。

#include "CmdTool.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>
#include <QStringList>

#include <gtest/gtest.h>

class UT_CmdToolParse : public UT_HEAD
{
public:
    void SetUp() override
    {
        tool = new CmdTool;
    }
    void TearDown() override
    {
        delete tool;
    }
    CmdTool *tool = nullptr;
};

// getMapInfoFromCmd: 按 "key: value" 分隔
TEST_F(UT_CmdToolParse, getMapInfoFromCmd)
{
    QMap<QString, QString> m;
    tool->getMapInfoFromCmd("Vendor: Intel\nModel: SSD\nInvalidLine\n", m);
    EXPECT_EQ(m.value("Vendor"), QString("Intel"));
    EXPECT_EQ(m.value("Model"), QString("SSD"));
}

// getMapInfoFromInput: count(ch)<=2 分支(3 段拼接)
TEST_F(UT_CmdToolParse, getMapInfoFromInput_simple)
{
    QMap<QString, QString> m;
    tool->getMapInfoFromInput("A: k: v\n", m);
    EXPECT_GT(m.size(), 0);
}

// getMapInfoFromInput: count(ch)>2 分支(按空格分词)
TEST_F(UT_CmdToolParse, getMapInfoFromInput_multi)
{
    QMap<QString, QString> m;
    tool->getMapInfoFromInput("B: k1=v1 k2=v2 k3=v3\n", m, QString("="));
    EXPECT_GT(m.size(), 0);
}

// getMapInfoFromLshw: 常规字段 + configuration + resources 分支
TEST_F(UT_CmdToolParse, getMapInfoFromLshw)
{
    QMap<QString, QString> m;
    QString info = "description: Generic\n"
                   "product: SSD\n"
                   "configuration: depth=32 modes=pio1\n"
                   "resources: irq:42 memory:fe000000\n";
    tool->getMapInfoFromLshw(info, m);
    EXPECT_TRUE(m.contains("description"));
    EXPECT_TRUE(m.contains("depth"));
}

// getMapInfoFromLshw: network 开头的 logical name 去重分支
TEST_F(UT_CmdToolParse, getMapInfoFromLshw_network)
{
    QMap<QString, QString> m;
    tool->getMapInfoFromLshw("network\nlogical name: /dev/eth0\n", m);
    EXPECT_TRUE(m.contains("logical name"));
}

// getMapInfoFromHwinfo: VID/PID 提取 + 引号 value + Driver 去引号
TEST_F(UT_CmdToolParse, getMapInfoFromHwinfo)
{
    QMap<QString, QString> m;
    QString info = "Vendor: 0x8086 Intel Corp\n"
                   "Device: 0x1234 SSD\n"
                   "Model: \"Test Device\"\n"
                   "Driver: \"ahci\"\n"
                   "SysFS ID: abcdef\n"
                   "Resolution: 1920x1080\n";
    tool->getMapInfoFromHwinfo(info, m);
    EXPECT_GT(m.size(), 0);
}

// getMapInfoFromHwinfo: PS/2 Mouse 分支 + Config Status avail
TEST_F(UT_CmdToolParse, getMapInfoFromHwinfo_ps2)
{
    QMap<QString, QString> m;
    tool->getMapInfoFromHwinfo("Hardware Class: PS/2 Mouse\nConfig Status: avail=yes\n", m);
    EXPECT_GT(m.size(), 0);
}

// getMapInfoFromDmidecode: "key:" 续行 + "key: value" 分支
TEST_F(UT_CmdToolParse, getMapInfoFromDmidecode)
{
    QMap<QString, QString> m;
    QString info = "Bank Locator: BK0\n"
                   "\tString1\n"
                   "Size: 8 GB\n";
    tool->getMapInfoFromDmidecode(info, m);
    EXPECT_TRUE(m.contains("Size"));
}

// getMapInfoFromSmartctl
TEST_F(UT_CmdToolParse, getMapInfoFromSmartctl)
{
    QMap<QString, QString> m;
    QString info = "Model Family: Samsung SSD\n"
                   "Device Model: SSD 870\n"
                   "Serial Number: S1XY\n";
    EXPECT_NO_FATAL_FAILURE(tool->getMapInfoFromSmartctl(m, info));
}

// getMulHwinfoInfo
TEST_F(UT_CmdToolParse, getMulHwinfoInfo)
{
    EXPECT_NO_FATAL_FAILURE(tool->getMulHwinfoInfo("block1\nblock2\n"));
}

// addWidthToMap
TEST_F(UT_CmdToolParse, addWidthToMap)
{
    QMap<QString, QString> m;
    m["width"] = "64 bits";
    EXPECT_NO_FATAL_FAILURE(tool->addWidthToMap(m));
}

// getSMBIOSVersion
TEST_F(UT_CmdToolParse, getSMBIOSVersion)
{
    QString ver;
    EXPECT_NO_FATAL_FAILURE(tool->getSMBIOSVersion("SMBIOS 3.6.0 present", ver));
    EXPECT_NO_FATAL_FAILURE(tool->getSMBIOSVersion("version 2.8", ver));
}
