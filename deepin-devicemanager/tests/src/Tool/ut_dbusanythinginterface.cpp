// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDBusInterface>
#include <QStringList>

#include "stub.h"

#define private public
#include "DBusAnythingInterface.h"
#undef private

class UT_DBusAnythingInterface : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_iface = DBusAnythingInterface::getInstance();
    }

    void TearDown() override
    {
    }

    DBusAnythingInterface *m_iface = nullptr;
};

// getInstance: singleton returns non-null and stable across calls
TEST_F(UT_DBusAnythingInterface, GetInstance_CalledTwice_ReturnsSameSingleton)
{
    // Arrange
    DBusAnythingInterface *p1 = DBusAnythingInterface::getInstance();

    // Act
    DBusAnythingInterface *p2 = DBusAnythingInterface::getInstance();

    // Assert
    EXPECT_NE(nullptr, p1);
    EXPECT_NE(nullptr, p2);
    EXPECT_EQ(p1, p2);
}

// Constructor: init() creates mp_Iface via QDBusInterface
TEST_F(UT_DBusAnythingInterface, Constructor_AfterInit_IfacePointerValid)
{
    // Arrange
    DBusAnythingInterface *ifaceObj = DBusAnythingInterface::getInstance();

    // Act
    QDBusInterface *iface = ifaceObj->mp_Iface;

    // Assert
    EXPECT_NE(nullptr, iface);
    EXPECT_EQ(iface, ifaceObj->mp_Iface);
}

// searchDriver: with no real D-Bus service, hasLFT reply invalid → returns false, list unchanged
TEST_F(UT_DBusAnythingInterface, SearchDriver_NoDbusService_ReturnsFalseAndListUnchanged)
{
    // Arrange
    QStringList lstDriver;
    lstDriver << "pre-existing";

    // Act
    bool result = m_iface->searchDriver("/tmp", lstDriver);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(1, lstDriver.size());
    EXPECT_EQ(QString("pre-existing"), lstDriver.first());
}
