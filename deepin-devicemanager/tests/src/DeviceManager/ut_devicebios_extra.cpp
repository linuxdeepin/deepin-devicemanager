// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceBios.h"

#include "stub.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

/**
 * @brief UT_DeviceBiosExtra 补充覆盖 DeviceBios 中未覆盖的方法:
 * isBoard / tomlname / setInfoFromTomlOneByOne。
 * 纯内存逻辑,无外部依赖,无需打桩。
 */
class UT_DeviceBiosExtra : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceBios = new DeviceBios;
    }
    void TearDown()
    {
        delete m_deviceBios;
    }
    DeviceBios *m_deviceBios;
};

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_isBoard_001)
{
    // Arrange
    m_deviceBios->m_IsBoard = false;

    // Act
    bool isBoard = m_deviceBios->isBoard();

    // Assert
    EXPECT_FALSE(isBoard);
    EXPECT_EQ(m_deviceBios->m_IsBoard, isBoard);
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_isBoard_002)
{
    // Arrange
    m_deviceBios->m_IsBoard = true;

    // Act
    bool isBoard = m_deviceBios->isBoard();

    // Assert
    EXPECT_TRUE(isBoard);
    EXPECT_EQ(m_deviceBios->m_IsBoard, isBoard);
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_tomlname_001)
{
    // Arrange
    m_deviceBios->m_tomlName = "bios.toml";

    // Act
    const QString &name = m_deviceBios->tomlname();

    // Assert
    EXPECT_STREQ("bios.toml", name.toStdString().c_str());
    EXPECT_EQ(&m_deviceBios->m_tomlName, &name);
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_tomlname_002)
{
    // Arrange
    m_deviceBios->m_tomlName = "custom-bios.toml";

    // Act
    m_deviceBios->m_tomlName = "changed.toml";
    const QString &name = m_deviceBios->tomlname();

    // Assert
    EXPECT_STREQ("changed.toml", name.toStdString().c_str());
    EXPECT_EQ(&m_deviceBios->m_tomlName, &name);
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_setInfoFromTomlOneByOne_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Version", "1.2.3");
    mapInfo.insert("Product Name", "ProductX");
    mapInfo.insert("Chipset", "ChipsetY");
    m_deviceBios->m_Version = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBios->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("1.2.3", m_deviceBios->m_Version.toStdString().c_str());
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_setInfoFromTomlOneByOne_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Version", "nouse");
    m_deviceBios->m_Version = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBios->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_STREQ("", m_deviceBios->m_Version.toStdString().c_str());
}

TEST_F(UT_DeviceBiosExtra, UT_DeviceBiosExtra_setInfoFromTomlOneByOne_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Product Name", "ProductX");
    m_deviceBios->m_Version = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBios->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("OLD", m_deviceBios->m_Version.toStdString().c_str());
}

/**
 * @brief 声明表:本文件新增用例与被测方法映射
 * | method | level | factors | min | actual |
 * |---|---|---|---|---|
 * | isBoard | mid | in_degree:1 | 2 | 2 |
 * | tomlname | mid | in_degree:1 | 2 | 2 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 3 |
 */
