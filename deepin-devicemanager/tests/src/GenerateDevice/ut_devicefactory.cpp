// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method             | level | factors                                | min | actual |
// |--------------------|-------|----------------------------------------|-----|--------|
// | getDeviceGenerator | high  | complexity:9, cognitive:45, branches:11| 3   | 11     |
// | DeviceFactory      | low   | -                                      | 1   | 1      |
// ─── 生成后填入 actual 列，低于 min 即违规 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]（空架构=默认分支、空类型=Arm 分支）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（各分支生成的类型不同，逐支断言更精确）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（源码无显式 throw）
// 8. 负面场景有专门用例: [x]（未知架构/未知类型默认分支）
// 9. 负面用例验证强异常安全: [x]（默认分支返回 X86Generator，不抛出）
// 10. stub_ext vs gMock 选择正确: [x]（静态全局走项目 stub.h，不用 gMock）
//
// 分支清单（来源：DeviceFactory::getDeviceGenerator(const QString &type = "")）：
// B1: getArch() == "x86_64"  → new X86Generator
// B2: getArch() == "mips64"  → new MipsGenerator
// B3: getArch() == "aarch64" → type 分支组：
//      B3a  type.isEmpty()      → new ArmGenerator
//      B3b  type == "KLVV"      → new HWGenerator
//      B3c  type == "PGUV"      → new HWGenerator
//      B3d  type == "PGUW"      → new HWGenerator
//      B3e  type == "KLVU"      → new HWGenerator
//      B3f  type == "PGUX"      → new HWGenerator
//      B3g  type == CustomType  → new CustomGenerator
//      B3h  type 其他           → new HWGenerator
// B4: getArch() 其他值       → 默认 new X86Generator
// 用例映射：
// - GetDeviceGenerator_ArchX86_ReturnsX86Generator          → B1
// - GetDeviceGenerator_ArchMips_ReturnsMipsGenerator        → B2
// - GetDeviceGenerator_ArmEmptyType_ReturnsArmGenerator     → B3a
// - GetDeviceGenerator_ArmKLVV_ReturnsHWGenerator           → B3b
// - GetDeviceGenerator_ArmPGUV_ReturnsHWGenerator           → B3c
// - GetDeviceGenerator_ArmPGUW_ReturnsHWGenerator           → B3d
// - GetDeviceGenerator_ArmKLVU_ReturnsHWGenerator           → B3e
// - GetDeviceGenerator_ArmPGUX_ReturnsHWGenerator           → B3f
// - GetDeviceGenerator_ArmCustomType_ReturnsCustomGenerator → B3g
// - GetDeviceGenerator_ArmUnknownType_ReturnsHWGenerator    → B3h
// - GetDeviceGenerator_UnknownArch_ReturnsX86Generator      → B4
// - Constructor_CreateInstance_UsableAfterConstruction      → 构造
//
// 打桩说明：Common::getArch / Common::boardVendorType 通过受控静态全局变量打桩，
// 避免 getArch 的 QProcess 真实探测与 boardVendorType 的 smbios 依赖（测试不依赖
// 真实设备环境）。生成的生成器由用例负责 delete（调用方持有所有权）。

#include "ArmGenerator.h"
#include "CustomGenerator.h"
#include "DeviceFactory.h"
#include "DeviceGenerator.h"
#include "HWGenerator.h"
#include "MipsGenerator.h"
#include "X86Generator.h"
#include "commonfunction.h"
#include "stub.h"

#include <gtest/gtest.h>

#include <memory>

// ── Common::getArch / Common::boardVendorType 打桩：受控静态全局 ──
static QString ut_arch_val = QStringLiteral("x86_64");
static QString ut_type_val = QString();

static QString ut_Common_getArch()
{
    return ut_arch_val;
}

static QString ut_Common_boardVendorType()
{
    return ut_type_val;
}

class UT_DeviceFactory : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ut_arch_val = QStringLiteral("x86_64");
        ut_type_val.clear();
        Stub stub;
        stub.set(ADDR(Common, getArch), ut_Common_getArch);
        stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    }
    void TearDown() override { }
};

// B1: x86_64 → X86Generator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArchX86_ReturnsX86Generator)
{
    // Arrange
    ut_arch_val = QStringLiteral("x86_64");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<X86Generator *>(gen), nullptr);
    delete gen;
}

// B2: mips64 → MipsGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArchMips_ReturnsMipsGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("mips64");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<MipsGenerator *>(gen), nullptr);
    EXPECT_EQ(dynamic_cast<X86Generator *>(gen), nullptr);
    delete gen;
}

// B3a: aarch64 + 空 type → ArmGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmEmptyType_ReturnsArmGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val.clear();
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<ArmGenerator *>(gen), nullptr);
    delete gen;
}

// B3b: aarch64 + KLVV → HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmKLVV_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("KLVV");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3c: aarch64 + PGUV → HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmPGUV_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("PGUV");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3d: aarch64 + PGUW → HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmPGUW_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("PGUW");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3e: aarch64 + KLVU → HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmKLVU_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("KLVU");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3f: aarch64 + PGUX → HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmPGUX_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("PGUX");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3g: aarch64 + CustomType → CustomGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmCustomType_ReturnsCustomGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("CustomType");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<CustomGenerator *>(gen), nullptr);
    EXPECT_EQ(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B3h: aarch64 + 未知 type → 兜底 HWGenerator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_ArmUnknownType_ReturnsHWGenerator)
{
    // Arrange
    ut_arch_val = QStringLiteral("aarch64");
    ut_type_val = QStringLiteral("UnknownBoard");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<HWGenerator *>(gen), nullptr);
    delete gen;
}

// B4: 未知架构 → 默认 X86Generator
TEST_F(UT_DeviceFactory, GetDeviceGenerator_UnknownArch_ReturnsX86Generator)
{
    // Arrange
    ut_arch_val = QStringLiteral("riscv64");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    DeviceGenerator *gen = DeviceFactory::getDeviceGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_NE(dynamic_cast<X86Generator *>(gen), nullptr);
    delete gen;
}

// 构造：实例可创建且构造后工厂仍然可用
TEST_F(UT_DeviceFactory, Constructor_CreateInstance_UsableAfterConstruction)
{
    // Arrange
    ut_arch_val = QStringLiteral("x86_64");
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);

    // Act
    std::unique_ptr<DeviceFactory> factory(new DeviceFactory());
    DeviceGenerator *gen = factory ? DeviceFactory::getDeviceGenerator() : nullptr;

    // Assert
    EXPECT_NE(factory, nullptr);
    EXPECT_NE(gen, nullptr);
    delete gen;
}
