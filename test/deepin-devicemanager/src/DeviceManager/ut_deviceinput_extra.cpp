// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ===================== 用例计数声明（总用例数: 9） =====================
// 设备: DeviceInput（输入设备, TOML/Hwinfo map 解析部分）
//
// | method | level | factors | min | actual |
// |---|---|---|---|---|
// | setInfoFromHwinfo | high | 等价类/分支 | 4 | 6 |
// | setInfoFromTomlOneByOne | mid | 等价类/分支 | 2 | 3 |
//
// 环境限制: setInfoFromHwinfo 中 Touchpad 分支依赖 DBusTouchPad 单例,
// 用例数据刻意避开 Touchpad 关键字; isBluetoothDevice 分支由
// ut_deviceinput_bus.cpp 以桩覆盖。

#include "DeviceInput.h"
#include "stub.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

// ===================== 测试夹具 =====================
class UT_DeviceInputExtra : public ::testing::Test
{
protected:
    void SetUp() override
    {
        di = new DeviceInput;
    }
    void TearDown() override
    {
        delete di;
        di = nullptr;
    }
    DeviceInput *di = nullptr;
};

// ===================== setInfoFromTomlOneByOne =====================
// 用例: 完整 TOML map 覆盖设置各字段, Capabilities 返回 TOML_Cover
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromToml_FullMap_CoversCapabilities)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Model", "TestModel");
    map.insert("Interface", "USB");
    map.insert("Bus Info", "usb@1:1");
    map.insert("Speed", "480M");
    map.insert("Maximum Current", "100mA");
    map.insert("Capabilities", "mouse");
    // Act
    TomlFixMethod method = di->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(method, TOML_Cover);
    EXPECT_EQ(di->m_Model, QString("TestModel"));
    EXPECT_EQ(di->m_Interface, QString("USB"));
    EXPECT_EQ(di->m_MaximumPower, QString("100mA"));
    EXPECT_EQ(di->m_Capabilities, QString("mouse"));
}

// 用例: Capabilities 值含 nouse 时清空字段并返回 TOML_nouse
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromToml_NouseCapability_ClearsValue)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Model", "TestModel");
    map.insert("Capabilities", "nouse");
    // Act
    TomlFixMethod method = di->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(method, TOML_nouse);
    EXPECT_TRUE(di->m_Capabilities.isEmpty());
    EXPECT_EQ(di->m_Model, QString("TestModel"));
}

// 用例: 空 map 不设置任何字段, 返回 TOML_None（无效输入等价类）
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromToml_EmptyMap_ReturnsNone)
{
    // Arrange
    QMap<QString, QString> map;
    // Act
    TomlFixMethod method = di->setInfoFromTomlOneByOne(map);
    // Assert
    EXPECT_EQ(method, TOML_None);
    EXPECT_TRUE(di->m_Model.isEmpty());
    EXPECT_TRUE(di->m_Capabilities.isEmpty());
}

// ===================== setInfoFromHwinfo（path 分支） =====================
// 用例: 含 path 键的 hwinfo map, USB 设备解析 VID/PID（B path 分支）
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_PathBranch_ParsesVidPid)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("path", "/sys/devices/pci0000:00/usb1/1-1");
    map.insert("name", "USB Optical Mouse");
    map.insert("driver", "usbhid");
    map.insert("unique_id", "1234567890");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_Name, QString("USB Optical Mouse"));
    EXPECT_EQ(di->m_Driver, QString("usbhid"));
    EXPECT_EQ(di->m_VID, QString("3456"));
    EXPECT_EQ(di->m_PID, QString("7890"));
    EXPECT_EQ(di->m_Interface, QString("USB"));
    EXPECT_FALSE(di->m_Enable);
}

// 用例: unique_id 长度不足 10 时不解析 VID/PID（边界值）
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_ShortUniqueId_VidPidEmpty)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("path", "/sys/devices/pci0000:00/usb1/1-1");
    map.insert("name", "USB Optical Mouse");
    map.insert("unique_id", "12345678");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_VID_PID, QString("12345678"));
    EXPECT_TRUE(di->m_VID.isEmpty());
    EXPECT_TRUE(di->m_PID.isEmpty());
}

// 用例: path 设备默认禁用状态（B path 分支收尾）
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_PathBranch_DeviceDisabled)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("path", "/sys/devices/pci0000:00/usb2/2-1");
    map.insert("name", "USB Keyboard");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_VID_PID, QString(""));
    EXPECT_FALSE(di->m_Enable);
    EXPECT_EQ(di->m_Name, QString("USB Keyboard"));
}

// ===================== setInfoFromHwinfo（常规分支） =====================
// 用例: 无 Hotplug 键时接口回退为 PS/2, 空驱动不可卸载
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_NoHotplug_SetsPs2)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "AT Translated Keyboard");
    map.insert("name", "AT Translated Keyboard");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_Interface, QString("PS/2"));
    EXPECT_FALSE(di->m_CanUninstall);
    EXPECT_EQ(di->m_PhysID, QString(""));
}

// 用例: Hotplug 键设置接口, Serial ID 正常写入
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_HotplugKey_SetsInterface)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "USB Optical Mouse");
    map.insert("Vendor", "Logitech");
    map.insert("Serial ID", "ABC123");
    map.insert("Hotplug", "USB");
    map.insert("SysFS BusID", "usb-1:1");
    map.insert("Hardware Class", "mouse");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_Interface, QString("USB"));
    EXPECT_EQ(di->m_Vendor, QString("Logitech"));
    EXPECT_EQ(di->m_SerialID, QString("ABC123"));
    EXPECT_EQ(di->m_BusInfo, QString("usb-1:1"));
}

// 用例: Serial ID 缺失时回退取 Unique ID
TEST_F(UT_DeviceInputExtra, UT_DeviceInputExtra_SetInfoFromHwinfo_SerialEmpty_FallsBackToUniqueID)
{
    // Arrange
    QMap<QString, QString> map;
    map.insert("Device", "USB Optical Mouse");
    map.insert("Unique ID", "uHci.0");
    map.insert("Hotplug", "USB");
    // Act
    di->setInfoFromHwinfo(map);
    // Assert
    EXPECT_EQ(di->m_SerialID, QString("uHci.0"));
    EXPECT_EQ(di->m_UniqueID, QString("uHci.0"));
    EXPECT_EQ(di->m_Interface, QString("USB"));
}
