// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#define protected public

#include "wakeuputils.h"

#undef private
#undef protected

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

class WakeupUtilsTest : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

// wakeupPath

TEST_F(WakeupUtilsTest, wakeupPath_validPath_returnsConstructedPath)
{
    // Arrange
    QString syspath = "/devices/fake_usb_test_xyz/identity";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString("/sys/devices/fake_usb_test_xyz/power/wakeup"));
}

TEST_F(WakeupUtilsTest, wakeupPath_noSlash_returnsFalse)
{
    // Arrange
    QString syspath = "nouspath";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

TEST_F(WakeupUtilsTest, wakeupPath_emptyInput_returnsFalse)
{
    // Arrange
    QString syspath = "";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

TEST_F(WakeupUtilsTest, wakeupPath_singleSlash_returnsFalse)
{
    // Arrange
    QString syspath = "/noslashafter";
    QString result;

    // Act
    bool ok = WakeupUtils::wakeupPath(syspath, result);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(result, QString());
}

// writeWakeupFile

TEST_F(WakeupUtilsTest, writeWakeupFile_nonexistentPath_returnsFalse)
{
    // Arrange
    QString path = "/nonexistent/path/to/wakeup";
    bool wakeup = true;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(QFile::exists(path), false);
}

TEST_F(WakeupUtilsTest, writeWakeupFile_writeEnabled_writesEnabled)
{
    // Arrange
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString path = tmpDir.path() + "/wakeup_test";
    bool wakeup = true;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_TRUE(result);
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), "enabled");
    f.close();
}

TEST_F(WakeupUtilsTest, writeWakeupFile_writeDisabled_writesDisabled)
{
    // Arrange
    QTemporaryDir tmpDir;
    ASSERT_TRUE(tmpDir.isValid());
    QString path = tmpDir.path() + "/wakeup_test2";
    bool wakeup = false;

    // Act
    bool result = WakeupUtils::writeWakeupFile(path, wakeup);

    // Assert
    EXPECT_TRUE(result);
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), "disabled");
    f.close();
}

// getMapInfo (private static, accessed via private hack)

TEST_F(WakeupUtilsTest, getMapInfo_tooFewLines_returnsFalse)
{
    // Arrange
    QString item = "line1\nline2\nline3";
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(mapInfo.isEmpty(), true);
}

TEST_F(WakeupUtilsTest, getMapInfo_validKeyboard_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: keyboard";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "keyboard");
}

TEST_F(WakeupUtilsTest, getMapInfo_notKeyboardOrMouse_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "network");
}

TEST_F(WakeupUtilsTest, getMapInfo_validMouse_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: mouse";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "mouse");
}

TEST_F(WakeupUtilsTest, getMapInfo_stripsQuotes_returnsStrippedValue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: \"Value%1\"").arg(i);
    lines[0] = "Hardware Class: keyboard";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = WakeupUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Key1"], "Value1");
}

// getPS2Syspath (private static)

TEST_F(WakeupUtilsTest, getPS2Syspath_noEventMatch_returnsEmpty)
{
    // Arrange
    QString dfs = "/dev/input/noevent";

    // Act
    QString result = WakeupUtils::getPS2Syspath(dfs);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.length(), 0);
}

TEST_F(WakeupUtilsTest, getPS2Syspath_emptyInput_returnsEmpty)
{
    // Arrange
    QString dfs = "";

    // Act
    QString result = WakeupUtils::getPS2Syspath(dfs);

    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.length(), 0);
}
