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

// 使用临时文件方式绕过 QFile 桩问题:在函数检查的系统路径下创建真实临时文件,
// 避免虚函数 vtable 桩破坏 QFile 内部状态导致 SEGV。

// 辅助:在系统路径下创建临时 TOML 文件,返回是否成功
static bool createTempToml(const QString &filename, const QString &content)
{
    QString path = "/usr/share/deepin-devicemanager/" + filename;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(content.toUtf8());
    file.close();
    return true;
}

static void removeTempToml(const QString &filename)
{
    QFile::remove("/usr/share/deepin-devicemanager/" + filename);
}

TEST_F(UT_CmdToolOem, parseOemTomlInfo_singleSection_returnsTrue)
{
    const QString fname = "ut_test_oem_single.toml";
    const QString toml =
        "[system]\n"
        "manufacturer = \"TestCorp\"\n"
        "product = \"TestProduct\"\n";

    if (!createTempToml(fname, toml)) {
        GTEST_SKIP() << "无法创建临时文件,跳过(需写权限: /usr/share/deepin-devicemanager/)";
    }

    // 单 section TOML:修复前返回 false,修复后应返回 true
    EXPECT_TRUE(tool->parseOemTomlInfo(fname));
    removeTempToml(fname);
}

TEST_F(UT_CmdToolOem, parseOemTomlInfo_multiSection_returnsTrue)
{
    const QString fname = "ut_test_oem_multi.toml";
    const QString toml =
        "[system]\n"
        "manufacturer = \"CorpA\"\n"
        "product = \"ProdA\"\n"
        "[board]\n"
        "vendor = \"VendorB\"\n"
        "version = \"2.0\"\n";

    if (!createTempToml(fname, toml)) {
        GTEST_SKIP() << "无法创建临时文件,跳过(需写权限: /usr/share/deepin-devicemanager/)";
    }

    // 多 section TOML:循环内第二个 section 头触发 tomlPars=true,始终返回 true
    EXPECT_TRUE(tool->parseOemTomlInfo(fname));
    removeTempToml(fname);
}

TEST_F(UT_CmdToolOem, parseOemTomlInfo_emptyFile_returnsFalse)
{
    const QString fname = "ut_test_oem_empty.toml";

    if (!createTempToml(fname, "")) {
        GTEST_SKIP() << "无法创建临时文件,跳过(需写权限: /usr/share/deepin-devicemanager/)";
    }

    // 空文件:info.isEmpty() 检查命中,函数提前返回 false
    EXPECT_FALSE(tool->parseOemTomlInfo(fname));
    removeTempToml(fname);
}
