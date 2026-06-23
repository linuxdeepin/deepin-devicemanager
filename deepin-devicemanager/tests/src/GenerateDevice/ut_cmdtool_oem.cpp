// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 CmdTool::loadOemTomlFileName(纯逻辑构造文件名) + parseOemTomlInfo(文件不存在返回 false)。

#include "CmdTool.h"
#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QByteArray>

#include <gtest/gtest.h>

class UT_CmdToolOem : public UT_HEAD
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

TEST_F(UT_CmdToolOem, loadOemTomlFileName_normal)
{
    QMap<QString, QString> m;
    m["Manufacturer"] = "Intel";
    m["Product Name"] = "X1 Carbon";
    m["Version"] = "1.0";
    QString r = tool->loadOemTomlFileName(m);
    EXPECT_TRUE(r.startsWith("oeminfo_"));
    EXPECT_TRUE(r.endsWith(".toml"));
}

TEST_F(UT_CmdToolOem, loadOemTomlFileName_emptyMap)
{
    EXPECT_TRUE(tool->loadOemTomlFileName(QMap<QString, QString>()).isEmpty());
}

TEST_F(UT_CmdToolOem, loadOemTomlFileName_naValues)
{
    QMap<QString, QString> m;
    m["Manufacturer"] = "N/A";
    m["Product Name"] = "N/A";
    m["Version"] = "N/A";
    EXPECT_NO_FATAL_FAILURE(tool->loadOemTomlFileName(m));
}

TEST_F(UT_CmdToolOem, loadOemTomlFileName_missingKeys)
{
    QMap<QString, QString> m;
    m["Manufacturer"] = "Intel";   // 缺 Product Name/Version
    EXPECT_TRUE(tool->loadOemTomlFileName(m).isEmpty());
}

TEST_F(UT_CmdToolOem, parseOemTomlInfo_emptyFilename)
{
    EXPECT_FALSE(tool->parseOemTomlInfo(""));
}

TEST_F(UT_CmdToolOem, parseOemTomlInfo_notExist)
{
    // 测试环境无 /etc/deepin/hardware/<file>,文件不存在返回 false
    EXPECT_FALSE(tool->parseOemTomlInfo("nonexistent_oem.toml"));
}

// parseOemTomlInfo 解析分支需桩 QFile/QIODevice,但虚函数 vtable 桩会破坏 QFile
// 内部状态导致 SEGV,故解析分支暂不覆盖(保留文件不存在路径)。
