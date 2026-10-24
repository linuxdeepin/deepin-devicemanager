// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ===================== 用例计数声明（总用例数: 7） =====================
// 设备: DeviceInput（输入设备, /proc/bus/input/devices 解析部分）
//
// | method | level | factors | min | actual |
// |---|---|---|---|---|
// | getPS2Syspath | high | 等价类/分支 | 3 | 4 |
// | isBluetoothDevice | high | 等价类/分支 | 3 | 3 |
//
// 打桩说明: QIODevice::readAll 返回可控的 /proc/bus/input/devices 内容;
// QProcess(hcitool) 打桩避免真实蓝牙栈依赖, 全部用例不依赖真实设备。

#include "DeviceInput.h"
#include "stub.h"

#include <QByteArray>
#include <QIODevice>
#include <QProcess>
#include <QString>

#include <gtest/gtest.h>

// ===================== 桩辅助 =====================
// QIODevice::readAll 桩: 返回可控的 /proc/bus/input/devices 伪内容
static QByteArray ut_input_fakeProcContent;
static QByteArray ut_input_stubReadAll(QIODevice * /*self*/)
{
    return ut_input_fakeProcContent;
}

// QProcess 桩: 屏蔽真实 hcitool 调用
static void ut_input_stubProcessStart(QProcess * /*self*/, const QString & /*program*/,
                                      const QStringList & /*arguments*/, QIODevice::OpenMode /*mode*/)
{
}
static bool ut_input_stubProcessWait(QProcess * /*self*/, int /*msecs*/)
{
    return true;
}
static QByteArray ut_input_stubProcessRead(QProcess * /*self*/)
{
    return QByteArray();
}

// ===================== 测试夹具 =====================
class UT_DeviceInputBus : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ut_input_fakeProcContent.clear();
        di = new DeviceInput;
    }
    void TearDown() override
    {
        delete di;
        di = nullptr;
    }
    DeviceInput *di = nullptr;
    Stub m_stub;
};

// ===================== getPS2Syspath =====================
// 用例: 设备文件串无 event 关键字直接返回 false（B1 无效输入等价类）
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_GetPS2Syspath_NoEvent_ReturnsFalse)
{
    // Arrange
    const QString deviceFiles = "psmouse serio1";
    // Act
    bool result = di->getPS2Syspath(deviceFiles);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(di->m_SysPath, QString(""));
}

// 用例: 匹配非 i2c 设备的 Sysfs 行提取 syspath（B3/B4）
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_GetPS2Syspath_EventMatch_ExtractsSysPath)
{
    // Arrange
    ut_input_fakeProcContent = QByteArray(
        "I: Bus=0011 Vendor=0002 Product=0006 Version=0000\n"
        "N: Name=\"ImExPS/2 Generic Wheel Mouse\"\n"
        "P: Phys=isa0060/serio1/input0\n"
        "S: Sysfs=/devices/platform/i8042/serio1/input/input5\n"
        "U: Uniq=\n"
        "H: Handlers=mouse0 event5\n"
        "B: EV=7\n\n");
    m_stub.set(ADDR(QIODevice, readAll), ut_input_stubReadAll);
    // Act
    bool result = di->getPS2Syspath("event5");
    // Assert
    EXPECT_TRUE(result);
    // 注: 非i2c正则为贪婪匹配, 捕获到末段 input 为止, 与 i2c 正则行为不一致(已列入缺陷候选)
    EXPECT_EQ(di->m_SysPath, QString("/devices/platform/i8042/serio1/input"));
    EXPECT_TRUE(di->m_SysPath.startsWith("/devices/platform/i8042"));
}

// 用例: i2c_designware 设备使用 i2c 专用正则提取（B3 分支）
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_GetPS2Syspath_I2cDesignware_UsesI2cRegex)
{
    // Arrange
    ut_input_fakeProcContent = QByteArray(
        "I: Bus=0018 Vendor=06cb Product=0000 Version=0000\n"
        "N: Name=\"Synaptics s3203\"\n"
        "P: Phys=i2c-2/input0\n"
        "S: Sysfs=/devices/platform/INT33D3:00/i2c_designware.0/input/input10\n"
        "U: Uniq=\n"
        "H: Handlers=mouse1 event10\n"
        "B: EV=b\n\n");
    m_stub.set(ADDR(QIODevice, readAll), ut_input_stubReadAll);
    // Act
    bool result = di->getPS2Syspath("event10");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(di->m_SysPath, QString("/devices/platform/INT33D3:00/i2c_designware.0"));
    EXPECT_TRUE(di->m_SysPath.contains("i2c_designware"));
}

// 用例: event 无匹配项时 syspath 保持原值, 函数仍返回 true
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_GetPS2Syspath_EventNotFound_KeepsSysPath)
{
    // Arrange
    di->m_SysPath = "/devices/original/path";
    ut_input_fakeProcContent = QByteArray(
        "I: Bus=0011 Vendor=0002 Product=0006 Version=0000\n"
        "N: Name=\"ImExPS/2 Generic Wheel Mouse\"\n"
        "S: Sysfs=/devices/platform/i8042/serio1/input/input5\n"
        "H: Handlers=mouse0 event5\n"
        "B: EV=7\n\n");
    m_stub.set(ADDR(QIODevice, readAll), ut_input_stubReadAll);
    // Act
    bool result = di->getPS2Syspath("event7");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(di->m_SysPath, QString("/devices/original/path"));
    EXPECT_FALSE(di->m_SysPath.contains("i8042"));
}

// ===================== isBluetoothDevice =====================
// 用例: 设备文件串无 event 直接返回 false（B1 无效输入等价类）
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_IsBluetoothDevice_NoEvent_ReturnsFalse)
{
    // Arrange
    const QString deviceFiles = "usb mouse";
    // Act
    bool result = di->isBluetoothDevice(deviceFiles);
    // Assert
    EXPECT_FALSE(result);
    EXPECT_EQ(di->m_SysPath, QString(""));
}

// 用例: 伪 proc 内容中 Uniq 匹配 MAC 正则, hcitool 输出不含该地址时标记未连接
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_IsBluetoothDevice_UniqMatch_MarksDisconnected)
{
    // Arrange
    di->m_BluetoothIsConnected = true;
    ut_input_fakeProcContent = QByteArray(
        "I: Bus=0005 Vendor=046D Product=B010 Version=0100\n"
        "N: Name=\"Bluetooth Mouse\"\n"
        "P: Phys=xx:xx\n"
        "S: Sysfs=/devices/pci0000:00/usb3/3-1/input/input20\n"
        "U: Uniq=AA:BB:CC:DD:EE:FF\n"
        "H: Handlers=sysrq kbd event8\n"
        "B: EV=10001f\n\n");
    m_stub.set(ADDR(QIODevice, readAll), ut_input_stubReadAll);
    m_stub.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start)),
               ut_input_stubProcessStart);
    m_stub.set(ADDR(QProcess, waitForFinished), ut_input_stubProcessWait);
    m_stub.set(ADDR(QProcess, readAllStandardOutput), ut_input_stubProcessRead);
    // Act
    bool result = di->isBluetoothDevice("event8");
    // Assert
    EXPECT_TRUE(result);
    EXPECT_FALSE(di->m_BluetoothIsConnected);
    EXPECT_EQ(di->m_Name, QString(""));
}

// 用例: 伪 proc 内容中 event 无匹配项返回 false, 连接状态不变
TEST_F(UT_DeviceInputBus, UT_DeviceInputBus_IsBluetoothDevice_EventNotFound_ReturnsFalse)
{
    // Arrange
    di->m_BluetoothIsConnected = true;
    ut_input_fakeProcContent = QByteArray(
        "I: Bus=0005 Vendor=046D Product=B010 Version=0100\n"
        "N: Name=\"Bluetooth Mouse\"\n"
        "U: Uniq=AA:BB:CC:DD:EE:FF\n"
        "H: Handlers=sysrq kbd event8\n"
        "B: EV=10001f\n\n");
    m_stub.set(ADDR(QIODevice, readAll), ut_input_stubReadAll);
    // Act
    bool result = di->isBluetoothDevice("event9");
    // Assert
    EXPECT_FALSE(result);
    EXPECT_TRUE(di->m_BluetoothIsConnected);
    EXPECT_EQ(di->m_Name, QString(""));
}
