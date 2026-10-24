// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceOthers.h"

#include "stub.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

/**
 * @brief UT_DeviceOtherExtra 补充覆盖 DeviceOthers:
 * available / vendor / setInfoFromTomlOneByOne,以及 setInfoFromHwinfo 的
 * fingerprint 分支和 setInfoFromLshw 的 avail-yes 分支。
 * 纯内存逻辑,驱动为空不触发 modinfo 查询,无需外部打桩。
 */
class UT_DeviceOtherExtra : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceOther = new DeviceOthers;
    }
    void TearDown()
    {
        delete m_deviceOther;
    }
    DeviceOthers *m_deviceOther;
};

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_available_001)
{
    // Arrange
    m_deviceOther->m_HardwareClass = "";

    // Act
    bool avail = m_deviceOther->available();

    // Assert
    EXPECT_TRUE(avail);
    EXPECT_EQ(m_deviceOther->m_Available, avail);
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_available_002)
{
    // Arrange
    m_deviceOther->m_HardwareClass = "others";

    // Act
    bool avail = m_deviceOther->available();

    // Assert
    EXPECT_FALSE(avail);
    EXPECT_EQ(m_deviceOther->m_Available, avail);
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_available_003)
{
    // Arrange
    m_deviceOther->m_HardwareClass = "others";
    m_deviceOther->m_Driver = "usb";

    // Act
    bool avail = m_deviceOther->available();

    // Assert
    EXPECT_TRUE(avail);
    EXPECT_EQ(m_deviceOther->m_Available, avail);
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_vendor_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "Generic Vendor");
    mapInfo.insert("Device", "Other Device");
    mapInfo.insert("Serial ID", "SN02");
    mapInfo.insert("Unique ID", "UID02");

    // Act
    m_deviceOther->setInfoFromHwinfo(mapInfo);
    const QString &vendor = m_deviceOther->vendor();

    // Assert
    EXPECT_STREQ("Generic Vendor", vendor.toStdString().c_str());
    EXPECT_STREQ("Generic Vendor", m_deviceOther->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromTomlOneByOne_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelX");
    mapInfo.insert("Bus Info", "bus01");
    mapInfo.insert("Serial Number", "SN123");
    m_deviceOther->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceOther->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("SN123", m_deviceOther->m_SerialID.toStdString().c_str());
    EXPECT_STREQ("ModelX", m_deviceOther->m_Model.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromTomlOneByOne_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Serial Number", "nouse");
    m_deviceOther->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceOther->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_STREQ("", m_deviceOther->m_SerialID.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromTomlOneByOne_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelX");
    m_deviceOther->m_SerialID = "OLD";

    // Act
    TomlFixMethod ret = m_deviceOther->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("OLD", m_deviceOther->m_SerialID.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromHwinfo_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Device", "Fingerprint Device");
    mapInfo.insert("Hardware Class", "fingerprint");
    mapInfo.insert("Unique ID", "UID9");

    // Act
    m_deviceOther->setInfoFromHwinfo(mapInfo);

    // Assert
    EXPECT_STREQ("fingerprint", m_deviceOther->m_HardwareClass.toStdString().c_str());
    EXPECT_FALSE(m_deviceOther->canEnable());
    EXPECT_FALSE(m_deviceOther->canUninstall());
    EXPECT_STREQ("UID9", m_deviceOther->m_SerialID.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromLshw_003)
{
    // Arrange
    m_deviceOther->m_Avail = "yes";
    m_deviceOther->m_HwinfoToLshw = "AA:BB";
    QMap<QString, QString> mapInfo;
    mapInfo.insert("bus info", "AA:BB");

    // Act
    m_deviceOther->setInfoFromLshw(mapInfo);

    // Assert
    EXPECT_FALSE(m_deviceOther->canEnable());
    EXPECT_FALSE(m_deviceOther->canUninstall());
    EXPECT_TRUE(m_deviceOther->available());
    EXPECT_STREQ("yes", m_deviceOther->m_Avail.toStdString().c_str());
}

TEST_F(UT_DeviceOtherExtra, UT_DeviceOtherExtra_setInfoFromLshw_004)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("product", "Unmatched Device");

    // Act
    m_deviceOther->setInfoFromLshw(mapInfo);

    // Assert
    EXPECT_TRUE(m_deviceOther->driver().isEmpty());
    EXPECT_TRUE(m_deviceOther->canUninstall());
    EXPECT_STREQ("", m_deviceOther->driver().toStdString().c_str());
}

/**
 * @brief 声明表:本文件新增用例与被测方法映射
 * | method | level | factors | min | actual |
 * |---|---|---|---|---|
 * | available | mid | in_degree:1 | 2 | 3 |
 * | vendor | low | - | 1 | 1 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 3 |
 * | setInfoFromHwinfo | mid | complexity:5, lines:55 | 2 | 2 |
 * | setInfoFromLshw | mid | complexity:6, in_degree:1 | 2 | 2 |
 */
