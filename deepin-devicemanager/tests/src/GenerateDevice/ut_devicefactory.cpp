// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceFactory.h"
#include "X86Generator.h"
#include "MipsGenerator.h"
#include "ArmGenerator.h"
#include "HWGenerator.h"
#include "CustomGenerator.h"
#include "commonfunction.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

// 通过全局变量控制桩返回值，覆盖 getDeviceGenerator 的所有架构/厂商分支
static QString ut_arch_val = QStringLiteral("x86_64");
static QString ut_type_val = QStringLiteral("");

static QString ut_Common_getArch()
{
    return ut_arch_val;
}

static QString ut_Common_boardVendorType()
{
    return ut_type_val;
}

class UT_DeviceFactory : public UT_HEAD
{
};

// arch == "x86_64" -> X86Generator
TEST_F(UT_DeviceFactory, getDeviceGenerator_x86_64)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    ut_arch_val = "x86_64";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(g, nullptr);
    EXPECT_NE(dynamic_cast<X86Generator *>(g), nullptr);
    delete g;
}

// arch == "mips64" -> MipsGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_mips64)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    ut_arch_val = "mips64";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<MipsGenerator *>(g), nullptr);
    delete g;
}

// arch == "aarch64" && type 空 -> ArmGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_empty)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<ArmGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + KLVV -> HWGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_KLVV)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "KLVV";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + PGUV/PGUW -> HWGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_PGUV)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "PGUV";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_PGUW)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "PGUW";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + KLVU -> HWGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_KLVU)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "KLVU";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + PGUX -> HWGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_PGUX)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "PGUX";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + CustomType -> CustomGenerator
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_CustomType)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "CustomType";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<CustomGenerator *>(g), nullptr);
    delete g;
}

// aarch64 + 其它非空 -> HWGenerator(else)
TEST_F(UT_DeviceFactory, getDeviceGenerator_aarch64_other)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    stub.set(ADDR(Common, boardVendorType), ut_Common_boardVendorType);
    ut_arch_val = "aarch64";
    ut_type_val = "SomeUnknownType";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<HWGenerator *>(g), nullptr);
    delete g;
}

// 未识别架构 -> default X86Generator
TEST_F(UT_DeviceFactory, getDeviceGenerator_default)
{
    Stub stub;
    stub.set(ADDR(Common, getArch), ut_Common_getArch);
    ut_arch_val = "riscv64";

    DeviceGenerator *g = DeviceFactory::getDeviceGenerator();
    EXPECT_NE(dynamic_cast<X86Generator *>(g), nullptr);
    delete g;
}
