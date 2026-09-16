// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#define private public
#define protected public

#include "enableutils.h"

#undef private
#undef protected

#include <QMap>

class EnableUtilsTest : public ::testing::Test
{
public:
    void SetUp() override {}
    void TearDown() override {}
};

// validateAndNormalizeVidPid

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_validWith0x_returnsTrue)
{
    // Arrange
    QString vid = "0x8086";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "8086");
    EXPECT_EQ(normPid, "1502");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_validWithout0x_returnsTrue)
{
    // Arrange
    QString vid = "8086";
    QString pid = "1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "8086");
    EXPECT_EQ(normPid, "1502");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_uppercaseNormalized_returnsTrue)
{
    // Arrange
    QString vid = "0xABCD";
    QString pid = "0x1234";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(normVid, "abcd");
    EXPECT_EQ(normPid, "1234");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_emptyVid_returnsFalse)
{
    // Arrange
    QString vid = "";
    QString pid = "1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid.isEmpty(), true);
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_emptyPid_returnsFalse)
{
    // Arrange
    QString vid = "8086";
    QString pid = "";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normPid.isEmpty(), true);
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_wrongLength_returnsFalse)
{
    // Arrange
    QString vid = "0x86";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid, "86");
}

TEST_F(EnableUtilsTest, validateAndNormalizeVidPid_nonHex_returnsFalse)
{
    // Arrange
    QString vid = "0xZZZZ";
    QString pid = "0x1502";
    QString normVid, normPid;

    // Act
    bool result = EnableUtils::validateAndNormalizeVidPid(vid, pid, normVid, normPid);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(normVid, "zzzz");
}

// getMapInfo (private static, accessed via private hack)

TEST_F(EnableUtilsTest, getMapInfo_tooFewLines_returnsFalse)
{
    // Arrange
    QString item = "line1\nline2\nline3";
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(mapInfo.isEmpty(), true);
}

TEST_F(EnableUtilsTest, getMapInfo_hubFiltered_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: hub";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(mapInfo.isEmpty());
}

TEST_F(EnableUtilsTest, getMapInfo_noSysfsBusid_returnsFalse)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(mapInfo.isEmpty());
}

TEST_F(EnableUtilsTest, getMapInfo_validWithBusid_returnsTrue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: Value%1").arg(i);
    lines[0] = "Hardware Class: network";
    lines[1] = "SysFS BusID: 0000:00:01.0";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Hardware Class"], "network");
    EXPECT_EQ(mapInfo["SysFS BusID"], "0000:00:01.0");
}

TEST_F(EnableUtilsTest, getMapInfo_stripsQuotes_returnsStrippedValue)
{
    // Arrange
    QStringList lines;
    for (int i = 0; i < 12; ++i)
        lines << QString("Key%1: \"Value%1\"").arg(i);
    lines[0] = "Hardware Class: network";
    lines[1] = "SysFS BusID: 0000:00:01.0";
    QString item = lines.join("\n");
    QMap<QString, QString> mapInfo;

    // Act
    bool result = EnableUtils::getMapInfo(item, mapInfo);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(mapInfo["Key2"], "Value2");
}

// enableNetworkByIoctl

TEST_F(EnableUtilsTest, enableNetworkByIoctl_emptyName_returnsFalse)
{
    // Arrange
    QString name = "";
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name.length(), 0);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_nameTooLong_returnsFalse)
{
    // Arrange
    QString name(16, 'a');
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name.length(), 16);
}

TEST_F(EnableUtilsTest, enableNetworkByIoctl_nonexistentInterface_returnsFalse)
{
    // Arrange
    QString name = "fake_iface_xyz";
    bool enable = true;

    // Act
    bool result = EnableUtils::enableNetworkByIoctl(name, enable);

    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(name, QString("fake_iface_xyz"));
}
