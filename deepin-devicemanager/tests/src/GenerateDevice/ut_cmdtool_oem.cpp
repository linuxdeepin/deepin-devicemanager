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
#include <QTemporaryFile>
#include <QDir>

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

// ---------------------------------------------------------------------------
// parseOemTomlInfo 解析分支回归测试
//
// 原 QFile/QIODevice 虚函数 vtable 桩方案会破坏 QFile 内部状态导致 SEGV，
// 故此处改用临时文件方式：在测试 SetUp 阶段通过 sudo 将 TOML 内容写入
// /etc/deepin/hardware/<filename>，使 QFile::exists / open / readAll 全部走
// 真实文件路径，无需桩虚函数。测试结束后清理临时文件。
// 若当前环境无 sudo 权限则 GTEST_SKIP 跳过。
// ---------------------------------------------------------------------------

static bool deployTomlFile(const QString &filename, const QByteArray &content)
{
    const QString dir = QStringLiteral("/etc/deepin/hardware");
    const QString dest = dir + QStringLiteral("/") + filename;

    // 写入临时文件
    QTemporaryFile tmp;
    if (!tmp.open())
        return false;
    tmp.write(content);
    tmp.close();

    // 复制到目标路径（需 sudo），并设置 644 权限确保非 root 用户可读
    QString cmd = QStringLiteral("sudo mkdir -p %1 && sudo install -m 644 %2 %3 2>/dev/null")
                      .arg(dir, tmp.fileName(), dest);
    int rc = std::system(cmd.toUtf8().constData());
    (void)rc;
    return QFile::exists(dest);
}

static void cleanupTomlFile(const QString &filename)
{
    const QString dest = QStringLiteral("/etc/deepin/hardware/") + filename;
    QString cmd = QStringLiteral("sudo rm -f %1 2>/dev/null").arg(dest);
    std::system(cmd.toUtf8().constData());
}

// 单 section TOML：修复前返回 false（BUG），修复后应返回 true
TEST_F(UT_CmdToolOem, parseOemTomlInfo_singleSection)
{
    const QString fname = QStringLiteral("test_single_section.toml");
    QByteArray toml =
        "[oem.info]\n"
        "Manufacturer = \"TestOEM\"\n"
        "Product = \"X1\"\n";

    if (!deployTomlFile(fname, toml)) {
        GTEST_SKIP() << "Cannot create test file at /etc/deepin/hardware/ (needs sudo)";
    }

    // 单 section：循环内不触发 tomlPars=true（itemMap 为空时遇到 section 头），
    // 仅靠循环后收尾块的 addMapInfo + tomlPars=true 才能返回 true。
    EXPECT_TRUE(tool->parseOemTomlInfo(fname));

    cleanupTomlFile(fname);
}

// 多 section TOML：修复前后均返回 true（第二个 section 头触发循环内 tomlPars=true）
TEST_F(UT_CmdToolOem, parseOemTomlInfo_multiSection)
{
    const QString fname = QStringLiteral("test_multi_section.toml");
    QByteArray toml =
        "[oem.info]\n"
        "Manufacturer = \"TestOEM\"\n"
        "[oem.other]\n"
        "Key = \"Value\"\n";

    if (!deployTomlFile(fname, toml)) {
        GTEST_SKIP() << "Cannot create test file at /etc/deepin/hardware/ (needs sudo)";
    }

    // 多 section：第二个 section 头触发循环内 tomlPars=true，无论是否修复都应返回 true。
    EXPECT_TRUE(tool->parseOemTomlInfo(fname));

    cleanupTomlFile(fname);
}

// 空文件：info 为空，函数应在 info.isEmpty() 检查处返回 false
TEST_F(UT_CmdToolOem, parseOemTomlInfo_emptyFile)
{
    const QString fname = QStringLiteral("test_empty_file.toml");
    QByteArray toml = "";

    if (!deployTomlFile(fname, toml)) {
        GTEST_SKIP() << "Cannot create test file at /etc/deepin/hardware/ (needs sudo)";
    }

    // 文件存在但内容为空：info.isEmpty() 为 true，函数返回 false。
    EXPECT_FALSE(tool->parseOemTomlInfo(fname));

    cleanupTomlFile(fname);
}
