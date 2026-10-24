// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceMemory.h"
#include "commonfunction.h"

#include "stub.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

/**
 * @brief UT_DeviceMemoryExtra 补充覆盖 DeviceMemory:
 * vendor / setInfoFromTomlOneByOne / 高优先级方法 loadOtherDeviceInfo 的
 * boardVendorType 空/非空分支。Common::boardVendorType 打桩,不触发真实
 * dmidecode 查询。
 */
class UT_DeviceMemoryExtra : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceMemory = new DeviceMemory;
    }
    void TearDown()
    {
        delete m_deviceMemory;
    }
    DeviceMemory *m_deviceMemory;
};

/**
 * @brief 打桩:boardVendorType 返回空(完整电压信息分支)
 */
static QString ut_memextra_boardvendor_empty()
{
    return QString();
}

/**
 * @brief 打桩:boardVendorType 返回非空(仅保留 Configured Speed 分支)
 */
static QString ut_memextra_boardvendor_klvv()
{
    return QString("KLVV");
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_vendor_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("product", "CT8G4DFS8266.M8FD");
    mapInfo.insert("vendor", "859B");
    mapInfo.insert("slot", "ChannelA-DIMM0");
    mapInfo.insert("size", "8GiB");

    // Act
    m_deviceMemory->setInfoFromLshw(mapInfo);
    const QString &vendor = m_deviceMemory->vendor();

    // Assert
    EXPECT_STREQ("859B", vendor.toStdString().c_str());
    EXPECT_STREQ("859B", m_deviceMemory->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_setInfoFromTomlOneByOne_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Size", "8GB");
    mapInfo.insert("Type", "DDR4");
    mapInfo.insert("Serial Number", "SN123");
    mapInfo.insert("Data Width", "64 bits");
    m_deviceMemory->m_DataBandwidth = "OLD";

    // Act
    TomlFixMethod ret = m_deviceMemory->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("8GB", m_deviceMemory->m_Size.toStdString().c_str());
    EXPECT_STREQ("DDR4", m_deviceMemory->m_Type.toStdString().c_str());
    EXPECT_STREQ("SN123", m_deviceMemory->m_SerialNumber.toStdString().c_str());
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_setInfoFromTomlOneByOne_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Data Width", "nouse");
    m_deviceMemory->m_DataBandwidth = "64 bits";

    // Act
    TomlFixMethod ret = m_deviceMemory->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_STREQ("", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_setInfoFromTomlOneByOne_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Size", "8GB");
    m_deviceMemory->m_DataBandwidth = "64 bits";

    // Act
    TomlFixMethod ret = m_deviceMemory->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("64 bits", m_deviceMemory->m_DataBandwidth.toStdString().c_str());
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_loadOtherDeviceInfo_001)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_memextra_boardvendor_empty);
    m_deviceMemory->m_ConfiguredVoltage = "1.2 V";
    m_deviceMemory->m_MaximumVoltage = "1.2 V";
    m_deviceMemory->m_MinimumVoltage = "1.2 V";
    m_deviceMemory->m_ConfiguredSpeed = "2400 MT/s";
    m_deviceMemory->m_DataBandwidth = "64 bits";
    m_deviceMemory->m_SerialID = "SN1";
    m_deviceMemory->m_UniqueID = "SN1";

    // Act
    m_deviceMemory->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(5, m_deviceMemory->m_LstOtherInfo.size());
    QStringList keys;
    for (int i = 0; i < m_deviceMemory->m_LstOtherInfo.size(); ++i)
        keys << m_deviceMemory->m_LstOtherInfo.at(i).first;
    EXPECT_TRUE(keys.contains("Configured Voltage"));
    EXPECT_TRUE(keys.contains("Maximum Voltage"));
    EXPECT_TRUE(keys.contains("Minimum Voltage"));
    EXPECT_TRUE(keys.contains("Configured Speed"));
    EXPECT_TRUE(keys.contains("Data Width"));
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_loadOtherDeviceInfo_002)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_memextra_boardvendor_klvv);
    m_deviceMemory->m_ConfiguredVoltage = "1.2 V";
    m_deviceMemory->m_MaximumVoltage = "1.2 V";
    m_deviceMemory->m_MinimumVoltage = "1.2 V";
    m_deviceMemory->m_ConfiguredSpeed = "2400 MT/s";
    m_deviceMemory->m_DataBandwidth = "64 bits";
    m_deviceMemory->m_SerialID = "SN1";
    m_deviceMemory->m_UniqueID = "SN1";

    // Act
    m_deviceMemory->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(2, m_deviceMemory->m_LstOtherInfo.size());
    QStringList keys;
    for (int i = 0; i < m_deviceMemory->m_LstOtherInfo.size(); ++i)
        keys << m_deviceMemory->m_LstOtherInfo.at(i).first;
    EXPECT_TRUE(keys.contains("Configured Speed"));
    EXPECT_TRUE(keys.contains("Data Width"));
    EXPECT_FALSE(keys.contains("Configured Voltage"));
}

TEST_F(UT_DeviceMemoryExtra, UT_DeviceMemoryExtra_loadOtherDeviceInfo_003)
{
    // Arrange
    Stub stub;
    stub.set(ADDR(Common, boardVendorType), ut_memextra_boardvendor_empty);
    m_deviceMemory->m_ConfiguredVoltage = "1.2 V";
    m_deviceMemory->m_MaximumVoltage = "1.2 V";
    m_deviceMemory->m_MinimumVoltage = "1.2 V";
    m_deviceMemory->m_ConfiguredSpeed = "2400 MT/s";
    m_deviceMemory->m_DataBandwidth = "64 bits";
    m_deviceMemory->m_MapOtherInfo.insert("Custom Key", "Custom Value");

    // Act
    m_deviceMemory->loadOtherDeviceInfo();

    // Assert
    EXPECT_EQ(6, m_deviceMemory->m_LstOtherInfo.size());
    QStringList keys;
    for (int i = 0; i < m_deviceMemory->m_LstOtherInfo.size(); ++i)
        keys << m_deviceMemory->m_LstOtherInfo.at(i).first;
    EXPECT_TRUE(keys.contains("Custom Key"));
    EXPECT_TRUE(keys.contains("Data Width"));
}

/**
 * @brief 声明表:本文件新增用例与被测方法映射
 * | method | level | factors | min | actual |
 * |---|---|---|---|---|
 * | vendor | low | - | 1 | 1 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 3 |
 * | loadOtherDeviceInfo | high | transitive_loop_depth:3 | 3 | 3 |
 */
