// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ArmGenerator.h"
#include "DeviceGenerator.h"
#include "DeviceManager.h"

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
    // Act
    m_gen->generatorCpuDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceCPU.size(), 0);
    EXPECT_TRUE(DeviceManager::instance()->m_ListDeviceCPU.isEmpty());
}
