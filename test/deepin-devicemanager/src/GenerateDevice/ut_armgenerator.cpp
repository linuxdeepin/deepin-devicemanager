// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                  | level | factors                        | min | actual |
// |-------------------------|-------|--------------------------------|-----|--------|
// | ArmGenerator            | low   | -                              | 1   | 1      |
// | addBusIDFromHwinfo      | low   | -                              | 1   | 3      |
// | getBusIDFromHwinfo      | low   | -                              | 1   | 3      |
// | generatorComputerDevice | mid   | loop:2, cmdInfo branches       | 2   | 2      |
// | generatorCpuDevice      | mid   | early-return, sort branch      | 2   | 2      |
// ─── 生成后填入 actual 列，低于 min 即违规 ───

#include "ArmGenerator.h"
#include "DeviceComputer.h"
#include "DeviceCpu.h"
#include "DeviceGenerator.h"
#include "DeviceManager.h"
#include "stub.h"

#include <gtest/gtest.h>

class UT_ArmGenerator : public ::testing::Test
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        m_gen = new ArmGenerator;
    }
    void TearDown() override
    {
        delete m_gen;
        DeviceManager::instance()->clear();
    }
    ArmGenerator *m_gen = nullptr;
};

TEST_F(UT_ArmGenerator, Constructor_CreateInstance_ReturnsNonNull)
{
    // Arrange
    const size_t expectedSize = 0;
    // Act
    ArmGenerator *gen = new ArmGenerator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_EQ(gen->getBusIDFromHwinfo().size(), expectedSize);
    delete gen;
}

TEST_F(UT_ArmGenerator, GeneratorComputerDevice_EmptyCmdInfo_AddsOneDevice)
{
    // Arrange
    const int expectedCount = 1;
    // Act
    m_gen->generatorComputerDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), expectedCount);
    EXPECT_NE(DeviceManager::instance()->m_ListDeviceComputer[0], nullptr);
}

TEST_F(UT_ArmGenerator, AddBusIDFromHwinfo_ValidIDs_ReturnsAllIDs)
{
    // Arrange
    m_gen->addBusIDFromHwinfo("pci:0:1:2:3");
    m_gen->addBusIDFromHwinfo("usb:1:2");

    // Act
    const QStringList &busIDs = m_gen->getBusIDFromHwinfo();

    // Assert
    EXPECT_EQ(busIDs.size(), 2);
    EXPECT_TRUE(busIDs.contains("pci:0:1:2:3"));
    EXPECT_TRUE(busIDs.contains("usb:1:2"));
}

TEST_F(UT_ArmGenerator, AddBusIDFromHwinfo_EmptyString_NoIDAdded)
{
    // Arrange
    m_gen->addBusIDFromHwinfo("");

    // Act
    const QStringList &busIDs = m_gen->getBusIDFromHwinfo();

    // Assert
    EXPECT_EQ(busIDs.size(), 0);
    EXPECT_TRUE(busIDs.isEmpty());
}

TEST_F(UT_ArmGenerator, AddBusIDFromHwinfo_TrailingDotNumber_StripsSuffix)
{
    // Arrange
    m_gen->addBusIDFromHwinfo("pci:0:1:2.0");

    // Act
    const QStringList &busIDs = m_gen->getBusIDFromHwinfo();

    // Assert
    EXPECT_EQ(busIDs.size(), 1);
    EXPECT_EQ(busIDs[0], "pci:0:1:2");
}

// Inheritance: ArmGenerator can be used polymorphically as DeviceGenerator
TEST_F(UT_ArmGenerator, Inheritance_IsDeviceGenerator_Polymorphic)
{
    // Arrange
    ArmGenerator gen;

    // Act
    DeviceGenerator *base = &gen;

    // Assert
    EXPECT_NE(base, nullptr);
    EXPECT_EQ(base, static_cast<DeviceGenerator *>(&gen));
}

// generatorCpuDevice: cmdInfo empty → returns early, no CPU device added
TEST_F(UT_ArmGenerator, GeneratorCpuDevice_EmptyCmdInfo_AddsNoDevice)
{
    // Arrange
    const int expectedCount = 0;

    // Act
    m_gen->generatorCpuDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCPU.size(), expectedCount);
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCPU.isEmpty());
}

// generatorCpuDevice 全路径：打桩 DeviceManager::cmdInfo 提供乱序 lscpu 数据，
// 覆盖 lsCpu.size()>1 排序交换分支，验证按 processor 升序建设备
static const QList<QMap<QString, QString>> &ut_cmdinfo_cpu(DeviceManager * /*self*/, const QString &key)
{
    static const QList<QMap<QString, QString>> lscpu = {
        QMap<QString, QString> {{"processor", "1"}, {"model name", "Test CPU B"}, {"vendor_id", "UT"}},
        QMap<QString, QString> {{"processor", "0"}, {"model name", "Test CPU A"}, {"vendor_id", "UT"}},
    };
    static const QList<QMap<QString, QString>> lscpuNum = {
        QMap<QString, QString> {{"core", "8"}, {"logical", "16"}}};
    static const QList<QMap<QString, QString>> dmidecode = {
        QMap<QString, QString> {{"Socket Designation", "CPU0"}, {"Core Count", "8"}, {"Thread Count", "16"}}};
    static const QList<QMap<QString, QString>> lshwCpu = {
        QMap<QString, QString> {{"product", "Test CPU"}}};
    static const QList<QMap<QString, QString>> empty;
    if (key == QStringLiteral("lscpu"))
        return lscpu;
    if (key == QStringLiteral("lscpu_num"))
        return lscpuNum;
    if (key == QStringLiteral("dmidecode4"))
        return dmidecode;
    if (key == QStringLiteral("lshw_cpu"))
        return lshwCpu;
    return empty;
}

TEST_F(UT_ArmGenerator, GeneratorCpuDevice_StubsCmdInfo_CreatesSortedCpuDevices)
{
    // Arrange
    Stub stub;
    stub.set(((const QList<QMap<QString, QString>> & (DeviceManager::*)(const QString &))ADDR(DeviceManager, cmdInfo)),
             ut_cmdinfo_cpu);
    const int expectedCount = 2;

    // Act
    m_gen->generatorCpuDevice();

    // Assert
    const auto &cpus = DeviceManager::instance()->m_ListDeviceCPU;
    ASSERT_EQ(cpus.size(), expectedCount);
    EXPECT_EQ(cpus[0]->name(), QStringLiteral("Test CPU A"));
    EXPECT_EQ(cpus[1]->name(), QStringLiteral("Test CPU B"));
}

// generatorComputerDevice 全路径：打桩 cmdInfo 提供 os-release 与 lshw_system 数据，
// 验证厂商/名称入库
static const QList<QMap<QString, QString>> &ut_cmdinfo_computer(DeviceManager * /*self*/, const QString &key)
{
    static const QList<QMap<QString, QString>> osRelease = {
        QMap<QString, QString> {{"HOME_URL", "\"https://www.uniontech.com\""}}};
    static const QList<QMap<QString, QString>> lshwSystem = {
        QMap<QString, QString> {{"description", "Desktop Computer"}, {"vendor", "UT Vendor"}, {"product", "TestPC"}}};
    static const QList<QMap<QString, QString>> empty;
    if (key == QStringLiteral("cat_os_release"))
        return osRelease;
    if (key == QStringLiteral("lshw_system"))
        return lshwSystem;
    return empty;
}

TEST_F(UT_ArmGenerator, GeneratorComputerDevice_StubsCmdInfo_FillsVendorAndName)
{
    // Arrange
    Stub stub;
    stub.set(((const QList<QMap<QString, QString>> & (DeviceManager::*)(const QString &))ADDR(DeviceManager, cmdInfo)),
             ut_cmdinfo_computer);
    const int expectedCount = 1;

    // Act
    m_gen->generatorComputerDevice();

    // Assert
    const auto &computers = DeviceManager::instance()->m_ListDeviceComputer;
    ASSERT_EQ(computers.size(), expectedCount);
    EXPECT_EQ(computers[0]->name(), QStringLiteral("TestPC"));
    EXPECT_EQ(computers[0]->vendor(), QStringLiteral("UT Vendor"));
}
