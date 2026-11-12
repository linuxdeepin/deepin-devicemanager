// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 CmdTool::loadOemTomlFileName(纯逻辑构造文件名) + parseOemTomlInfo
// (文件不存在返回 false / 单节-多节解析 / 空文件)。

#include "CmdTool.h"
#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QString>
#include <QFile>
#include <QIODevice>
#include <QByteArray>
#include <dlfcn.h>

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
// parseOemTomlInfo 解析分支回归测试：QFile 函数入口打桩（与 ut_gapcloser.cpp 同款）
//
// 历史方案1 - vtable 虚函数桩:破坏 QFile 内部状态导致 SEGV,勿回退;
// 历史方案2 - sudo 部署真实文件到 /etc/deepin/hardware:无 sudo 终端时
//   std::system("sudo ...") 读密码挂起,拖死整个测试进程,勿回退。
// 现方案:序列式 QFile::exists / QFile::open(dlsym 入口桩) + readAll 返回
// 预置内容,无系统副作用,无 sudo 依赖。
// ---------------------------------------------------------------------------

// QFile::exists(const QString&) 静态函数桩：按调用次序返回预置结果
static QList<bool> ut_oem_existsSeq;
static int         ut_oem_existsIdx = 0;
static bool ut_oem_fileExists(const QString & /*path*/)
{
    if (ut_oem_existsIdx < ut_oem_existsSeq.size())
        return ut_oem_existsSeq.at(ut_oem_existsIdx++);
    return false;
}
static void ut_oem_installQFileExistsStub(Stub &stub, QList<bool> seq)
{
    ut_oem_existsSeq = seq;
    ut_oem_existsIdx = 0;
    stub.set(reinterpret_cast<void *>(static_cast<bool (*)(const QString &)>(&QFile::exists)),
             reinterpret_cast<void *>(ut_oem_fileExists));
}

// QFile::open / QFile::readAll 桩：open 按调用次序返回预置结果，readAll 返回预置内容
static QList<bool> ut_oem_openSeq;
static int         ut_oem_openIdx = 0;
static QByteArray  ut_oem_fileContent;
static bool ut_oem_fileOpen(QFile * /*self*/, QIODevice::OpenMode /*mode*/)
{
    if (ut_oem_openIdx < ut_oem_openSeq.size())
        return ut_oem_openSeq.at(ut_oem_openIdx++);
    return false;
}
static QByteArray ut_oem_fileReadAll(void * /*self*/)
{
    return ut_oem_fileContent;
}
static void ut_oem_installQFileStubs(Stub &stub, QList<bool> openSeq, const QByteArray &content)
{
    void *openFn = dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    if (nullptr != openFn)
        stub.set(openFn, reinterpret_cast<void *>(ut_oem_fileOpen));
    stub.set(ADDR(QFile, readAll), ut_oem_fileReadAll);
    ut_oem_openSeq = openSeq;
    ut_oem_openIdx = 0;
    ut_oem_fileContent = content;
}

// 单 section TOML：修复前返回 false（BUG），修复后应返回 true
TEST_F(UT_CmdToolOem, parseOemTomlInfo_singleSection)
{
    // Arrange: 主路径存在且可读（调试文件/安装路径不再触达）
    Stub stub;
    ut_oem_installQFileExistsStub(stub, { true });
    ut_oem_installQFileStubs(stub, { true },
                             "[oem.info]\n"
                             "Manufacturer = \"TestOEM\"\n"
                             "Product = \"X1\"\n");

    // Act
    const bool ok = tool->parseOemTomlInfo("test_single_section.toml");

    // Assert: 单 section：循环内不触发 tomlPars=true（itemMap 为空时遇到 section 头），
    // 仅靠循环后收尾块的 addMapInfo + tomlPars=true 才能返回 true。
    EXPECT_TRUE(ok);
    EXPECT_TRUE(tool->m_cmdInfo.contains("tomloem"));
    EXPECT_EQ(tool->m_cmdInfo["tomloem"].size(), 1);
    EXPECT_EQ(tool->m_cmdInfo["tomloem"][0]["Manufacturer"], QString("TestOEM"));
    EXPECT_EQ(tool->m_cmdInfo["tomloem"][0]["Product"], QString("X1"));
}

// 多 section TOML：修复前后均返回 true（第二个 section 头触发循环内 tomlPars=true）
TEST_F(UT_CmdToolOem, parseOemTomlInfo_multiSection)
{
    // Arrange
    Stub stub;
    ut_oem_installQFileExistsStub(stub, { true });
    ut_oem_installQFileStubs(stub, { true },
                             "[oem.info]\n"
                             "Manufacturer = \"TestOEM\"\n"
                             "[oem.other]\n"
                             "Key = \"Value\"\n");

    // Act
    const bool ok = tool->parseOemTomlInfo("test_multi_section.toml");

    // Assert: 第二个 section 头触发循环内 addMapInfo，循环后收尾块再入库末节。
    // 注意 classkey 取节名首个 '.' 之前的部分（oem.info/oem.other 均为 "oem"），
    // 故两节数据都挂在 "tomloem" 键下（列表追加 2 条）。
    EXPECT_TRUE(ok);
    ASSERT_EQ(tool->m_cmdInfo["tomloem"].size(), 2);
    EXPECT_EQ(tool->m_cmdInfo["tomloem"][0]["Manufacturer"], QString("TestOEM"));
    EXPECT_EQ(tool->m_cmdInfo["tomloem"][1]["Key"], QString("Value"));
}

// 空文件：info 为空，函数应在 info.isEmpty() 检查处返回 false
TEST_F(UT_CmdToolOem, parseOemTomlInfo_emptyFile)
{
    // Arrange: 文件存在且可读，但内容为空
    Stub stub;
    ut_oem_installQFileExistsStub(stub, { true });
    ut_oem_installQFileStubs(stub, { true }, QByteArray(""));

    // Act / Assert: info.isEmpty() 为 true，函数返回 false，无数据入库。
    EXPECT_FALSE(tool->parseOemTomlInfo("test_empty_file.toml"));
    EXPECT_FALSE(tool->m_cmdInfo.contains("tomloem"));
}
