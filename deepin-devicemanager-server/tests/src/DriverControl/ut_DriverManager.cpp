// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "drivercontrol/drivermanager.h"
#include "DDLog.h"

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/*
 * Branch list for DriverManager::parsePrinterInfo (private, pure JSON parsing):
 *
 *   B1: empty byteArray                -> empty list    | ParsePrinterInfo_EmptyInput_ReturnsEmptyList
 *   B2: invalid JSON                   -> empty list    | ParsePrinterInfo_InvalidJson_ReturnsEmptyList
 *   B3: valid JSON, no "solutions" key -> empty list    | ParsePrinterInfo_NoSolutionsKey_ReturnsEmptyList
 *   B4: valid JSON, empty solutions    -> empty list    | ParsePrinterInfo_EmptySolutions_ReturnsEmptyList
 *   B5: valid JSON, single solution    -> 1 item        | ParsePrinterInfo_SingleSolution_ReturnsOneItem
 *   B6: valid JSON, multiple solutions -> N items       | ParsePrinterInfo_MultipleSolutions_ReturnsAllItems
 *   B7: solution with missing fields   -> default vals  | ParsePrinterInfo_MissingFields_ReturnsDefaults
 *   B8: solution entry is not object   -> skipped       | ParsePrinterInfo_NonObjectEntry_HandledGracefully
 *
 * NOTE: -fno-access-control is set in CMakeLists.txt, so private methods
 * are directly accessible without addr_pri macros.
 */

class DriverManager_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    // Helper: build a JSON byte array with a "solutions" array
    static QByteArray makeSolutionsJson(const QJsonArray &solutions)
    {
        QJsonObject root;
        root["solutions"] = solutions;
        QJsonDocument doc(root);
        return doc.toJson(QJsonDocument::Compact);
    }

    // Helper: build a single solution JSON object
    static QJsonObject makeSolution(int sid, const QString &describe,
                                    const QString &ppd, bool excat,
                                    const QString &driver)
    {
        QJsonObject obj;
        obj["sid"] = sid;
        obj["describe"] = describe;
        obj["ppd"] = ppd;
        obj["excat"] = excat;
        obj["driver"] = driver;
        return obj;
    }
};

// ===== parsePrinterInfo =====

TEST_F(DriverManager_UT, ParsePrinterInfo_EmptyInput_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QByteArray empty;
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(empty);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_InvalidJson_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QByteArray invalidJson = "{this is not valid json}";
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(invalidJson);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_NoSolutionsKey_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QJsonObject root;
    root["other_key"] = "some_value";
    QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_EmptySolutions_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QJsonArray emptyArray;
    QByteArray json = makeSolutionsJson(emptyArray);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_SingleSolution_ReturnsOneItem)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    solutions.append(makeSolution(1, "HP LaserJet", "hp-laserjet.ppd", true, "hplip"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].intSid, 1);
    EXPECT_EQ(result[0].strDescribe.toStdString(), "HP LaserJet");
    EXPECT_EQ(result[0].strPpd.toStdString(), "hp-laserjet.ppd");
    EXPECT_TRUE(result[0].isExcat);
    EXPECT_EQ(result[0].strDriver.toStdString(), "hplip");
}

TEST_F(DriverManager_UT, ParsePrinterInfo_MultipleSolutions_ReturnsAllItems)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    solutions.append(makeSolution(1, "HP LaserJet", "hp-laserjet.ppd", true, "hplip"));
    solutions.append(makeSolution(2, "Canon PIXMA", "canon-pixma.ppd", false, "cnijfilter"));
    solutions.append(makeSolution(3, "Epson EcoTank", "epson-ecotank.ppd", true, "epson-inkjet"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].intSid, 1);
    EXPECT_EQ(result[1].intSid, 2);
    EXPECT_EQ(result[2].intSid, 3);
    EXPECT_EQ(result[1].strDescribe.toStdString(), "Canon PIXMA");
    EXPECT_FALSE(result[1].isExcat);
    EXPECT_EQ(result[2].strDriver.toStdString(), "epson-inkjet");
}

TEST_F(DriverManager_UT, ParsePrinterInfo_MissingFields_ReturnsDefaults)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    // Solution object with only "sid", other fields missing
    QJsonObject partial;
    partial["sid"] = 42;
    solutions.append(partial);
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].intSid, 42);
    EXPECT_TRUE(result[0].strDescribe.isEmpty());
    EXPECT_TRUE(result[0].strPpd.isEmpty());
    EXPECT_FALSE(result[0].isExcat);
    EXPECT_TRUE(result[0].strDriver.isEmpty());
}

TEST_F(DriverManager_UT, ParsePrinterInfo_NonObjectEntry_HandledGracefully)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    // Add a non-object value (string) followed by a valid object
    solutions.append("not_an_object");
    solutions.append(makeSolution(5, "Brother HL", "brother.ppd", false, "brother-driver"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert — toObject() on a non-object returns empty QJsonObject,
    // so the first entry produces default values; the second is parsed correctly
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].intSid, 0);
    EXPECT_TRUE(result[0].strDescribe.isEmpty());
    EXPECT_EQ(result[1].intSid, 5);
    EXPECT_EQ(result[1].strDescribe.toStdString(), "Brother HL");
}
