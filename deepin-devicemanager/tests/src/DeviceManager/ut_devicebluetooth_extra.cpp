// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceBluetooth.h"
#include "DBusEnableInterface.h"

#include "stub.h"

#include <QCoreApplication>
#include <gtest/gtest.h>

/**
 * @brief UT_DeviceBluetoothExtra 补充覆盖 DeviceBluetooth:
 * vendor / setInfoFromTomlOneByOne / setEnable 的 DBus 失败分支。
 * DBus 调用统一通过 stub 打桩,不依赖真实设备环境。
 */
class UT_DeviceBluetoothExtra : public ::testing::Test
{
public:
    void SetUp()
    {
        m_deviceBluetooth = new DeviceBluetooth;
    }
    void TearDown()
    {
        delete m_deviceBluetooth;
    }
    DeviceBluetooth *m_deviceBluetooth;
};

/**
 * @brief 打桩:DBusEnableInterface::enable 返回失败
 */
static bool ut_btextra_enable_false(const QString &, const QString &, const QString &, const QString &, bool, const QString &)
{
    return false;
}

TEST_F(UT_DeviceBluetoothExtra, UT_DeviceBluetoothExtra_vendor_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "Intel Corp.");
    mapInfo.insert("Device", "Bluetooth Device");
    mapInfo.insert("Serial ID", "SN01");
    mapInfo.insert("Unique ID", "UID01");

    // Act
    m_deviceBluetooth->setInfoFromHwinfo(mapInfo);
    const QString &vendor = m_deviceBluetooth->vendor();

    // Assert
    EXPECT_STREQ("Intel Corp.", vendor.toStdString().c_str());
    EXPECT_STREQ("Intel Corp.", m_deviceBluetooth->m_Vendor.toStdString().c_str());
}

TEST_F(UT_DeviceBluetoothExtra, UT_DeviceBluetoothExtra_setInfoFromTomlOneByOne_001)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelX");
    mapInfo.insert("Name", "NameY");
    mapInfo.insert("Vendor", "VendorZ");
    mapInfo.insert("Version", "1.0");
    mapInfo.insert("MAC Address", "AA:BB:CC:DD:EE:FF");
    m_deviceBluetooth->m_MAC = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBluetooth->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_Cover, ret);
    EXPECT_STREQ("AA:BB:CC:DD:EE:FF", m_deviceBluetooth->m_MAC.toStdString().c_str());
    EXPECT_STREQ("ModelX", m_deviceBluetooth->m_Model.toStdString().c_str());
}

TEST_F(UT_DeviceBluetoothExtra, UT_DeviceBluetoothExtra_setInfoFromTomlOneByOne_002)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("MAC Address", "nouse");
    m_deviceBluetooth->m_MAC = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBluetooth->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_nouse, ret);
    EXPECT_STREQ("", m_deviceBluetooth->m_MAC.toStdString().c_str());
}

TEST_F(UT_DeviceBluetoothExtra, UT_DeviceBluetoothExtra_setInfoFromTomlOneByOne_003)
{
    // Arrange
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Model", "ModelX");
    m_deviceBluetooth->m_MAC = "OLD";

    // Act
    TomlFixMethod ret = m_deviceBluetooth->setInfoFromTomlOneByOne(mapInfo);

    // Assert
    EXPECT_EQ(TOML_None, ret);
    EXPECT_STREQ("OLD", m_deviceBluetooth->m_MAC.toStdString().c_str());
}

TEST_F(UT_DeviceBluetoothExtra, UT_DeviceBluetoothExtra_setEnable_001)
{
    // Arrange
    m_deviceBluetooth->m_SerialID = "SerialID";
    m_deviceBluetooth->m_UniqueID = "UniqueID";
    m_deviceBluetooth->m_SysPath = "SysPath";
    Stub stub;
    stub.set(ADDR(DBusEnableInterface, enable), ut_btextra_enable_false);

    // Act
    EnableDeviceStatus status = m_deviceBluetooth->setEnable(false);

    // Assert
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, status);
    EXPECT_TRUE(m_deviceBluetooth->enable());
}

/**
 * @brief 声明表:本文件新增用例与被测方法映射
 * | method | level | factors | min | actual |
 * |---|---|---|---|---|
 * | vendor | low | - | 1 | 1 |
 * | setInfoFromTomlOneByOne | mid | in_degree:1 | 2 | 3 |
 * | setEnable | low | - | 1 | 1 |
 */
