// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusError>
#include <QDBusAbstractInterface>
#include <QVariant>
#include <QStringList>
#include <QDir>

#include "stub.h"

#define private public
#include "DBusAnythingInterface.h"
#undef private

// --- Stub helpers ---
// The template call() routes through private doCall(); we stub doCall directly.
// Stub functions include the this pointer as first parameter (project convention).

QDBusMessage ut_doCall_hasLFT_true(
    QDBusAbstractInterface * /*self*/,
    QDBus::CallMode /*mode*/,
    const QString &method,
    const QVariant * /*args*/,
    size_t /*numArgs*/)
{
    QDBusMessage msg;
    if (method == "hasLFT") {
        msg = msg.createReply(QVariant(true));
    } else if (method == "search") {
        QStringList drivers;
        drivers << "driver1.deb"
                << "driver2.ko";
        msg = msg.createReply(QVariant(drivers));
    }
    return msg;
}

QDBusMessage ut_doCall_hasLFT_false(
    QDBusAbstractInterface * /*self*/,
    QDBus::CallMode /*mode*/,
    const QString &method,
    const QVariant * /*args*/,
    size_t /*numArgs*/)
{
    QDBusMessage msg;
    if (method == "hasLFT") {
        msg = msg.createReply(QVariant(false));
    }
    return msg;
}

QDBusMessage ut_doCall_error(
    QDBusAbstractInterface * /*self*/,
    QDBus::CallMode /*mode*/,
    const QString & /*method*/,
    const QVariant * /*args*/,
    size_t /*numArgs*/)
{
    return QDBusMessage::createError(QDBusError::Failed, "stub error");
}

bool ut_QDir_exists_false()
{
    return false;
}

// Function pointer type for QDBusAbstractInterface::doCall (private, but
// -fno-access-control lets us take its address for stubbing).
typedef QDBusMessage (QDBusAbstractInterface::*DoCallFunc)(
    QDBus::CallMode, const QString &, const QVariant *, size_t);

// Function pointer type for QDir::exists() const
typedef bool (QDir::*ExistsFunc)() const;

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

// searchDriver: hasLFT returns true and search returns driver list → success
TEST_F(UT_DBusAnythingInterface, SearchDriver_HasLFTTrue_ReturnsDriverList)
{
    // Arrange
    Stub stub;
    DoCallFunc p = &QDBusAbstractInterface::doCall;
    stub.set(p, ut_doCall_hasLFT_true);
    QStringList drivers;

    // Act
    bool result = m_iface->searchDriver(QDir::tempPath(), drivers);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(drivers.size(), 2);
}

// searchDriver: hasLFT returns false and directory does not exist → failure
TEST_F(UT_DBusAnythingInterface, SearchDriver_HasLFTFalse_DirNotExists_ReturnsFalse)
{
    // Arrange
    Stub stub;
    DoCallFunc p = &QDBusAbstractInterface::doCall;
    stub.set(p, ut_doCall_hasLFT_false);
    ExistsFunc pe = &QDir::exists;
    stub.set(pe, ut_QDir_exists_false);
    QStringList drivers;
    QString testPath = QDir::tempPath();

    // Act
    bool result = m_iface->searchDriver(testPath, drivers);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(drivers.size(), 0);
}

// hasLFT: reply valid and value is true → returns true
TEST_F(UT_DBusAnythingInterface, HasLFT_ValidReplyTrue_ReturnsTrue)
{
    // Arrange
    Stub stub;
    DoCallFunc p = &QDBusAbstractInterface::doCall;
    stub.set(p, ut_doCall_hasLFT_true);
    QString testPath = QDir::tempPath();

    // Act
    bool result = m_iface->hasLFT(testPath);

    // Assert
    EXPECT_NE(m_iface, nullptr);
    EXPECT_TRUE(result);
}

// hasLFT: reply valid and value is false → returns false
TEST_F(UT_DBusAnythingInterface, HasLFT_ValidReplyFalse_ReturnsFalse)
{
    // Arrange
    Stub stub;
    DoCallFunc p = &QDBusAbstractInterface::doCall;
    stub.set(p, ut_doCall_hasLFT_false);
    QString testPath = QDir::tempPath();

    // Act
    bool result = m_iface->hasLFT(testPath);

    // Assert
    EXPECT_NE(m_iface, nullptr);
    EXPECT_FALSE(result);
}

// hasLFT: reply invalid (error message) → returns false
TEST_F(UT_DBusAnythingInterface, HasLFT_InvalidReply_ReturnsFalse)
{
    // Arrange
    Stub stub;
    DoCallFunc p = &QDBusAbstractInterface::doCall;
    stub.set(p, ut_doCall_error);
    QString testPath = QDir::tempPath();

    // Act
    bool result = m_iface->hasLFT(testPath);

    // Assert
    EXPECT_NE(m_iface, nullptr);
    EXPECT_FALSE(result);
}
