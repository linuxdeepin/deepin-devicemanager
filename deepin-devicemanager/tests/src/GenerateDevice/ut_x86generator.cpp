// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "X86Generator.h"
#include "DeviceManager.h"

#include <gtest/gtest.h>

class UT_X86Generator : public ::testing::Test
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        m_gen = new X86Generator;
    }
    void TearDown() override
    {
        delete m_gen;
        DeviceManager::instance()->clear();
    }
    X86Generator *m_gen = nullptr;
};

TEST_F(UT_X86Generator, Constructor_CreateInstance_ReturnsNonNull)
{
    // Arrange
    const size_t expectedSize = 0;
    // Act
    X86Generator *gen = new X86Generator();

    // Assert
    EXPECT_NE(gen, nullptr);
    EXPECT_EQ(gen->getBusIDFromHwinfo().size(), expectedSize);
    delete gen;
}

TEST_F(UT_X86Generator, GeneratorComputerDevice_EmptyCmdInfo_AddsOneDevice)
{
    // Arrange
    const int expectedCount = 1;
    // Act
    m_gen->generatorComputerDevice();

    // Assert
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), expectedCount);
    EXPECT_NE(DeviceManager::instance()->m_ListDeviceComputer[0], nullptr);
}

TEST_F(UT_X86Generator, AddBusIDFromHwinfo_ValidIDs_ReturnsAllIDs)
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

TEST_F(UT_X86Generator, AddBusIDFromHwinfo_EmptyString_NoIDAdded)
{
    // Arrange
    m_gen->addBusIDFromHwinfo("");

    // Act
    const QStringList &busIDs = m_gen->getBusIDFromHwinfo();

    // Assert
    EXPECT_EQ(busIDs.size(), 0);
    EXPECT_TRUE(busIDs.isEmpty());
}

TEST_F(UT_X86Generator, AddBusIDFromHwinfo_TrailingDotNumber_StripsSuffix)
{
    // Arrange
    m_gen->addBusIDFromHwinfo("pci:0:1:2.0");

    // Act
    const QStringList &busIDs = m_gen->getBusIDFromHwinfo();

    // Assert
    EXPECT_EQ(busIDs.size(), 1);
    EXPECT_EQ(busIDs[0], "pci:0:1:2");
}
