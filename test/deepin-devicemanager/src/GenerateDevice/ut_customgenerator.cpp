// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method               | level | factors                              | min | actual |
// |----------------------|-------|--------------------------------------|-----|--------|
// | generatorGpuDevice   | high  | complexity:6, loop:4, branches:5     | 3   | 5      |
// | generatorMonitorDevice | mid | fs:sysfs, branches:3                 | 2   | 3      |
// | CustomGenerator      | low   | -                                    | 1   | 1      |
// ─── 生成后填入 actual 列，低于 min 即违规 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [x]（各块结构不同，逐块用例更精确）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（源码无显式 throw）
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]（空输入不改变设备列表）
// 10. stub_ext vs gMock 选择正确: [x]（Qt 内置类走项目 stub.h，不用 gMock）
//
// 分支清单（来源：CustomGenerator::generatorGpuDevice()）：
// B1: info.isEmpty()                     → return（不添加设备）
// B2: split("\n\n") 按 "\n\n" 分块       → 每块解析为一个设备
// B3: line.indexOf(':') == -1            → 跳过该行
// B4: 内层逐块循环                       → 块数 = 设备数
// B5: infoMap.isEmpty()                  → 跳过该块（不 push）
// 分支清单（来源：CustomGenerator::generatorMonitorDevice()）：
// B6: toDir_.exists() == false           → return（不解析）
// B7: fileName == "." / ".."             → continue
// B8: !fileName.startsWith("card")       → continue
// B9: !QFile::exists(edid 路径)          → continue（不调 parseEDID）
// B10: card 条目命中                     → parseEDID(edid 路径, 接口名, false)
// 用例映射：
// - Constructor_CreateInstance_IsDeviceGenerator          → 构造
// - GeneratorGpuDevice_EmptyInfo_AddsNoDevice             → B1
// - GeneratorGpuDevice_MultiBlockInfo_CreatesDevicePerBlock → B2/B4
// - GeneratorGpuDevice_MalformedLines_SkipColonlessLines  → B3
// - GeneratorGpuDevice_WhitespaceOnlyBlocks_AddsNoDevice  → B5
// - GeneratorGpuDevice_PaddedFields_TrimsKeyAndValue      → B3（trim 边界）
// - GeneratorMonitorDevice_DrmDirMissing_NoParsing        → B6
// - GeneratorMonitorDevice_CardWithEdid_ParsesEdid        → B7/B8/B10
// - GeneratorMonitorDevice_CardWithoutEdid_SkipsParsing   → B9
//
// 打桩说明：preGenerateGpuInfo 依赖真实硬件探测（QProcess/hwinfo 等），统一打桩
// 为受控文本；generatorMonitorDevice 依赖 /sys/class/drm 目录树，打桩 QDir 存在性、
// 目录枚举与 QFile::exists；CommonTools::parseEDID 仅做计数捕获（解析依赖真实 EDID
// 文件，超出单测范围）。测试不依赖真实设备环境。

#include "CustomGenerator.h"
#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "commontools.h"
#include "stub.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

#include <gtest/gtest.h>

// ── CommonTools::preGenerateGpuInfo 打桩：受控 GPU 探测文本 ──
static QString ut_pre_gpuinfo_empty()
{
    return QString();
}

static QString ut_pre_gpuinfo_multi()
{
    return QString::fromUtf8("Name: HD Graphics\nVendor: Intel\n\nName: GTX 1060\nVendor: NVIDIA");
}

static QString ut_pre_gpuinfo_malformed()
{
    return QString::fromUtf8("Name: Malformed\nno-colon-line\n\nName: Good");
}

static QString ut_pre_gpuinfo_whitespace()
{
    return QString::fromUtf8("   \n\t\n");
}

static QString ut_pre_gpuinfo_padded()
{
    return QString::fromUtf8("  Name : Spaced Value  ");
}

// ── CommonTools::parseEDID 打桩：捕获调用参数 ──
static int s_parseEDID_calls = 0;
static QStringList s_parseEDID_files;
static QString s_parseEDID_interface;
static void ut_parseEDID_capture(const QStringList &listFiles, const QString &interfaceName, bool /*flag*/)
{
    ++s_parseEDID_calls;
    s_parseEDID_files = listFiles;
    s_parseEDID_interface = interfaceName;
}

// ── QDir::exists / entryInfoList 打桩：模拟 /sys/class/drm 目录树 ──
static bool ut_dir_exists_true(QDir * /*self*/)
{
    return true;
}

static bool ut_dir_exists_false(QDir * /*self*/)
{
    return false;
}

static QFileInfoList s_drm_entries;
static QFileInfoList ut_dir_entries(QDir * /*self*/, QDir::Filters /*filters*/, QDir::SortFlags /*sort*/)
{
    return s_drm_entries;
}

// ── QFile::exists 打桩：控制 edid 文件存在性 ──
static bool ut_qfile_exists_true(const QString & /*fileName*/)
{
    return true;
}

static bool ut_qfile_exists_false(const QString & /*fileName*/)
{
    return false;
}

class UT_CustomGenerator : public ::testing::Test
{
protected:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        s_parseEDID_calls = 0;
        s_parseEDID_files.clear();
        s_parseEDID_interface.clear();
        s_drm_entries.clear();
        m_gen = new CustomGenerator;
    }
    void TearDown() override
    {
        delete m_gen;
        m_gen = nullptr;
        DeviceManager::instance()->clear();
    }
    CustomGenerator *m_gen = nullptr;
};

// 构造：实例可创建且可多态使用，基类 busID 列表初始为空
TEST_F(UT_CustomGenerator, Constructor_CreateInstance_IsDeviceGenerator)
{
    // Arrange
    CustomGenerator gen;

    // Act
    DeviceGenerator *base = &gen;
    const QStringList &busIDs = gen.getBusIDFromHwinfo();

    // Assert
    EXPECT_NE(base, nullptr);
    EXPECT_TRUE(busIDs.isEmpty());
}

// B1: 探测结果为空 → 直接返回，不添加任何 GPU 设备
TEST_F(UT_CustomGenerator, GeneratorGpuDevice_EmptyInfo_AddsNoDevice)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_pre_gpuinfo_empty);
    const size_t expectedCount = 0;

    // Act
    m_gen->generatorGpuDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), expectedCount);
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.isEmpty());
}

// B2/B4: 两个 "\n\n" 分隔块 → 每块生成一个设备，字段正确入库
TEST_F(UT_CustomGenerator, GeneratorGpuDevice_MultiBlockInfo_CreatesDevicePerBlock)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_pre_gpuinfo_multi);
    const int expectedCount = 2;

    // Act
    m_gen->generatorGpuDevice();

    // Assert
    const auto &gpus = DeviceManager::instance()->m_ListDeviceGPU;
    ASSERT_EQ(gpus.size(), expectedCount);
    EXPECT_EQ(gpus[0]->name(), QStringLiteral("HD Graphics"));
    EXPECT_EQ(gpus[0]->vendor(), QStringLiteral("Intel"));
    EXPECT_EQ(gpus[1]->name(), QStringLiteral("GTX 1060"));
    EXPECT_EQ(gpus[1]->vendor(), QStringLiteral("NVIDIA"));
}

// B3: 无冒号行被跳过，同块其余行仍正常解析
TEST_F(UT_CustomGenerator, GeneratorGpuDevice_MalformedLines_SkipColonlessLines)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_pre_gpuinfo_malformed);
    const int expectedCount = 2;

    // Act
    m_gen->generatorGpuDevice();

    // Assert
    const auto &gpus = DeviceManager::instance()->m_ListDeviceGPU;
    ASSERT_EQ(gpus.size(), expectedCount);
    EXPECT_EQ(gpus[0]->name(), QStringLiteral("Malformed"));
    EXPECT_EQ(gpus[1]->name(), QStringLiteral("Good"));
}

// B5: 仅空白行/无冒号行 → infoMap 为空 → 不添加设备
TEST_F(UT_CustomGenerator, GeneratorGpuDevice_WhitespaceOnlyBlocks_AddsNoDevice)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_pre_gpuinfo_whitespace);
    const int expectedCount = 0;

    // Act
    m_gen->generatorGpuDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), expectedCount);
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceGPU.isEmpty());
}

// B3 边界：键值两侧空白被 trim（"  Name : Spaced Value  " → "Spaced Value"）
TEST_F(UT_CustomGenerator, GeneratorGpuDevice_PaddedFields_TrimsKeyAndValue)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_pre_gpuinfo_padded);
    const int expectedCount = 1;

    // Act
    m_gen->generatorGpuDevice();

    // Assert
    const auto &gpus = DeviceManager::instance()->m_ListDeviceGPU;
    ASSERT_EQ(gpus.size(), expectedCount);
    EXPECT_EQ(gpus[0]->name(), QStringLiteral("Spaced Value"));
    EXPECT_EQ(gpus[0]->getOverviewInfo(), QStringLiteral("Spaced Value"));
}

// B6: drm 目录不存在 → 直接返回，不触发 parseEDID
TEST_F(UT_CustomGenerator, GeneratorMonitorDevice_DrmDirMissing_NoParsing)
{
    // Arrange
    Stub stub;
    stub.set(((bool (QDir::*)() const)ADDR(QDir, exists)), ut_dir_exists_false);
    const int expectedCalls = 0;

    // Act
    EXPECT_NO_THROW(m_gen->generatorMonitorDevice());

    // Assert
    EXPECT_EQ(s_parseEDID_calls, expectedCalls);
    EXPECT_TRUE(s_parseEDID_files.isEmpty());
}

// B7/B8/B10: card 条目命中（"."、".."、"version" 被跳过），parseEDID 收到
// edid 路径与去掉 "card0-" 前缀的接口名
TEST_F(UT_CustomGenerator, GeneratorMonitorDevice_CardWithEdid_ParsesEdid)
{
    // Arrange
    Stub stub;
    stub.set(((bool (QDir::*)() const)ADDR(QDir, exists)), ut_dir_exists_true);
    stub.set(((QFileInfoList (QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList)),
             ut_dir_entries);
    stub.set(((bool (*)(const QString &))ADDR(QFile, exists)), ut_qfile_exists_true);
    stub.set(ADDR(CommonTools, parseEDID), ut_parseEDID_capture);
    s_drm_entries = QFileInfoList()
                    << QFileInfo(QStringLiteral("/sys/class/drm/."))
                    << QFileInfo(QStringLiteral("/sys/class/drm/.."))
                    << QFileInfo(QStringLiteral("/sys/class/drm/card0-DP-1"))
                    << QFileInfo(QStringLiteral("/sys/class/drm/version"));
    const int expectedCalls = 1;

    // Act
    m_gen->generatorMonitorDevice();

    // Assert
    EXPECT_EQ(s_parseEDID_calls, expectedCalls);
    EXPECT_EQ(s_parseEDID_files, QStringList() << QStringLiteral("/sys/class/drm/card0-DP-1/edid"));
    EXPECT_EQ(s_parseEDID_interface, QStringLiteral("DP-1"));
}

// B9: edid 文件不存在 → 该条目跳过，不触发 parseEDID
TEST_F(UT_CustomGenerator, GeneratorMonitorDevice_CardWithoutEdid_SkipsParsing)
{
    // Arrange
    Stub stub;
    stub.set(((bool (QDir::*)() const)ADDR(QDir, exists)), ut_dir_exists_true);
    stub.set(((QFileInfoList (QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList)),
             ut_dir_entries);
    stub.set(((bool (*)(const QString &))ADDR(QFile, exists)), ut_qfile_exists_false);
    stub.set(ADDR(CommonTools, parseEDID), ut_parseEDID_capture);
    s_drm_entries = QFileInfoList() << QFileInfo(QStringLiteral("/sys/class/drm/card0-HDMI-A-1"));
    const int expectedCalls = 0;

    // Act
    m_gen->generatorMonitorDevice();

    // Assert
    EXPECT_EQ(s_parseEDID_calls, expectedCalls);
    EXPECT_TRUE(s_parseEDID_files.isEmpty());
}
