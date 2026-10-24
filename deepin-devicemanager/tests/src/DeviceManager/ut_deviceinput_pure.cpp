// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ===================== 用例计数声明（总用例数: 23） =====================
// 设备: DeviceInput（输入设备, 纯逻辑与唤醒能力部分）
//
// | method | level | factors | min | actual |
// |---|---|---|---|---|
// | getDetailBusInfo | high | 等价类/边界 | 3 | 4 |
// | canWakeupMachine | high | 等价类/分支 | 3 | 3 |
// | isWakeupMachine | high | 分支 | 2 | 3 |
// | setEnable | high | 等价类/分支 | 3 | 5 |
// | wakeupPath | mid | 等价类/边界 | 2 | 3 |
// | eventStrFromDeviceFiles | mid | 等价类/边界 | 2 | 3 |
// | getInterface | low | 冒烟 | 1 | 1 |
// | bluetoothIsConnected | low | 冒烟 | 1 | 1 |
//
// 环境限制: isWakeupMachine 的 PS/2 分支依赖 DBusWakeupInterface
// (org.deepin.dde.Cooperation1 服务), 无头环境不可覆盖, 已在
// ut_deviceinput.cpp 既有用例中以桩覆盖。

#include "DeviceInput.h"
#include "stub.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QTemporaryDir>

#include <gtest/gtest.h>

// ===================== 桩辅助 =====================
// wakeupPath 桩: 返回由用例控制的可控文件路径, 避免依赖真实 /sys 唤醒节点
static QString ut_input_wakeupFilePath;
static QString ut_input_stubWakeupPath(DeviceInput * /*self*/)
{
    return ut_input_wakeupFilePath;
}

// ===================== 测试夹具 =====================
class UT_DeviceInputPure : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ut_input_wakeupFilePath.clear();
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

// ===================== getInterface / bluetoothIsConnected =====================
// 用例: getInterface 返回成员变量值
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_GetInterface_ReturnsMemberValue)
{
    // Arrange
    di->m_Interface = "USB";
    // Act
    QString usbInterface = di->getInterface();
    // Assert
    EXPECT_EQ(usbInterface, QString("USB"));
    di->m_Interface = "Bluetooth";
    EXPECT_EQ(di->getInterface(), QString("Bluetooth"));
}

// 用例: bluetoothIsConnected 返回成员连接状态
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_BluetoothIsConnected_ReturnsMemberValue)
{
    // Arrange
    di->m_BluetoothIsConnected = true;
    // Act
    bool connected = di->bluetoothIsConnected();
    // Assert
    EXPECT_TRUE(connected);
    EXPECT_EQ(di->bluetoothIsConnected(), di->m_BluetoothIsConnected);
    di->m_BluetoothIsConnected = false;
    EXPECT_FALSE(di->bluetoothIsConnected());
}

// ===================== wakeupPath（B1-B3） =====================
// 用例: PS/2 设备返回 ACPI 唤醒文件路径（B2）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_WakeupPath_Ps2Name_ReturnsAcpiPath)
{
    // Arrange
    di->m_Name = "PS/2 Keyboard";
    di->m_SysPath = "/devices/platform/i8042/serio1/input/input5";
    // Act
    QString path = di->wakeupPath();
    // Assert
    EXPECT_EQ(path, QString("/proc/acpi/wakeup"));
    EXPECT_TRUE(path.contains("acpi"));
}

// 用例: USB 设备按 syspath 拼接 power/wakeup 路径（B3）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_WakeupPath_UsbDevice_BuildsPowerWakeupPath)
{
    // Arrange
    di->m_Name = "USB Mouse";
    di->m_SysPath = "/devices/pci0000:00/usb1/1-1/input/input1";
    // Act
    QString path = di->wakeupPath();
    // Assert
    EXPECT_EQ(path, QString("/sys/devices/pci0000:00/usb1/1-1/input/power/wakeup"));
    EXPECT_TRUE(path.endsWith("power/wakeup"));
}

// 用例: syspath 无有效父目录时返回空（B1 边界值）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_WakeupPath_ShortSysPath_ReturnsEmpty)
{
    // Arrange
    di->m_Name = "USB Mouse";
    di->m_SysPath = "/x";
    // Act
    QString path = di->wakeupPath();
    // Assert
    EXPECT_EQ(path, QString(""));
    EXPECT_TRUE(path.isEmpty());
}

// ===================== canWakeupMachine（B1-B3） =====================
// 用例: WakeupID 为空直接返回 false（B1 边界值）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_CanWakeupMachine_EmptyWakeupID_ReturnsFalse)
{
    // Arrange
    di->m_WakeupID.clear();
    // Act
    bool canWakeup = di->canWakeupMachine();
    // Assert
    EXPECT_FALSE(canWakeup);
    EXPECT_EQ(canWakeup, false);
}

// 用例: 唤醒文件不存在时返回 false（B2, wakeupPath 桩指向不存在路径）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_CanWakeupMachine_FileMissing_ReturnsFalse)
{
    // Arrange
    di->m_WakeupID = "PNP0C0E:00";
    QString missingPath;
    {
        QTemporaryDir tmpDir;
        missingPath = tmpDir.path() + "/wakeup";
    }
    ASSERT_FALSE(QFile::exists(missingPath));
    ut_input_wakeupFilePath = missingPath;
    m_stub.set(ADDR(DeviceInput, wakeupPath), ut_input_stubWakeupPath);
    // Act
    bool canWakeup = di->canWakeupMachine();
    // Assert
    EXPECT_FALSE(canWakeup);
    EXPECT_EQ(QFile::exists(missingPath), false);
}

// 用例: 唤醒文件存在且可读时返回 true（B3, 桩指向临时文件）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_CanWakeupMachine_FileReadable_ReturnsTrue)
{
    // Arrange
    di->m_WakeupID = "PNP0C0E:00";
    QTemporaryDir tmpDir;
    QString filePath = tmpDir.path() + "/wakeup";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("enabled\n");
    file.close();
    ut_input_wakeupFilePath = filePath;
    m_stub.set(ADDR(DeviceInput, wakeupPath), ut_input_stubWakeupPath);
    // Act
    bool canWakeup = di->canWakeupMachine();
    // Assert
    EXPECT_TRUE(canWakeup);
    EXPECT_EQ(di->canWakeupMachine(), true);
}

// ===================== isWakeupMachine（非 PS/2 分支） =====================
// 用例: 唤醒文件不存在返回 false
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_IsWakeupMachine_FileMissing_ReturnsFalse)
{
    // Arrange
    di->m_Name = "USB Mouse";
    di->m_Interface = "USB";
    QString missingPath;
    {
        QTemporaryDir tmpDir;
        missingPath = tmpDir.path() + "/wakeup";
    }
    ASSERT_FALSE(QFile::exists(missingPath));
    ut_input_wakeupFilePath = missingPath;
    m_stub.set(ADDR(DeviceInput, wakeupPath), ut_input_stubWakeupPath);
    // Act
    bool wakeup = di->isWakeupMachine();
    // Assert
    EXPECT_FALSE(wakeup);
    EXPECT_EQ(QFile::exists(missingPath), false);
}

// 用例: 唤醒文件内容不含 disabled 时返回 true
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_IsWakeupMachine_EnabledContent_ReturnsTrue)
{
    // Arrange
    di->m_Name = "USB Mouse";
    di->m_Interface = "USB";
    QTemporaryDir tmpDir;
    QString filePath = tmpDir.path() + "/wakeup";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("S4 *enabled: usb1\n");
    file.close();
    ut_input_wakeupFilePath = filePath;
    m_stub.set(ADDR(DeviceInput, wakeupPath), ut_input_stubWakeupPath);
    // Act
    bool wakeup = di->isWakeupMachine();
    // Assert
    EXPECT_TRUE(wakeup);
    EXPECT_EQ(di->m_Interface, QString("USB"));
}

// 用例: 唤醒文件内容含 disabled 时返回 false
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_IsWakeupMachine_DisabledContent_ReturnsFalse)
{
    // Arrange
    di->m_Name = "USB Mouse";
    di->m_Interface = "USB";
    QTemporaryDir tmpDir;
    QString filePath = tmpDir.path() + "/wakeup";
    QFile file(filePath);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("S4 *disabled: usb1\n");
    file.close();
    ut_input_wakeupFilePath = filePath;
    m_stub.set(ADDR(DeviceInput, wakeupPath), ut_input_stubWakeupPath);
    // Act
    bool wakeup = di->isWakeupMachine();
    // Assert
    EXPECT_FALSE(wakeup);
    EXPECT_NE(di->isWakeupMachine(), true);
}

// ===================== setEnable（状态分支） =====================
// 用例: USB 键盘 VID 为空返回 EDS_Faild
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_SetEnable_KeyboardUsbEmptyVid_ReturnsFailed)
{
    // Arrange
    di->m_HardwareClass = "keyboard";
    di->m_Interface = "USB";
    di->m_VID.clear();
    di->m_PID.clear();
    // Act
    EnableDeviceStatus status = di->setEnable(true);
    // Assert
    EXPECT_EQ(status, EDS_Faild);
    EXPECT_NE(status, EDS_Success);
    EXPECT_EQ(di->m_VID, QString(""));
}

// 用例: 非 USB 键盘返回 EDS_Faild
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_SetEnable_KeyboardNonUsb_ReturnsFailed)
{
    // Arrange
    di->m_HardwareClass = "keyboard";
    di->m_Interface = "PS/2";
    di->m_VID = "1a2b";
    di->m_PID = "3c4d";
    // Act
    EnableDeviceStatus status = di->setEnable(true);
    // Assert
    EXPECT_EQ(status, EDS_Faild);
    EXPECT_EQ(status, EnableDeviceStatus::EDS_Faild);
}

// 用例: 触摸板设备走 TouchPad DBus 设置后返回 EDS_Success
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_SetEnable_Touchpad_ReturnsSuccess)
{
    // Arrange
    di->m_Name = "Synaptics Touchpad";
    di->m_HardwareClass = "mouse";
    // Act
    EnableDeviceStatus status = di->setEnable(true);
    // Assert
    EXPECT_EQ(status, EDS_Success);
    EXPECT_TRUE(di->m_Enable);
}

// 用例: 无序列号的普通鼠标返回 EDS_NoSerial
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_SetEnable_NoSerial_ReturnsNoSerial)
{
    // Arrange
    di->m_Name = "Logitech USB Mouse";
    di->m_HardwareClass = "mouse";
    di->m_SerialID.clear();
    // Act
    EnableDeviceStatus status = di->setEnable(false);
    // Assert
    EXPECT_EQ(status, EDS_NoSerial);
    EXPECT_NE(status, EDS_Success);
}

// 用例: 有序列号但 UniqueID 为空返回 EDS_Faild
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_SetEnable_EmptyUniqueID_ReturnsFailed)
{
    // Arrange
    di->m_Name = "Logitech USB Mouse";
    di->m_HardwareClass = "mouse";
    di->m_SerialID = "SER123456";
    di->m_UniqueID.clear();
    di->m_SysPath = "/devices/pci0000:00/usb1";
    // Act
    EnableDeviceStatus status = di->setEnable(false);
    // Assert
    EXPECT_EQ(status, EDS_Faild);
    EXPECT_NE(status, EDS_NoSerial);
    EXPECT_EQ(di->m_UniqueID, QString(""));
}

// ===================== eventStrFromDeviceFiles（B1-B3） =====================
// 用例: 从设备文件串提取 event 编号
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_EventStrFromDeviceFiles_ExtractsEvent)
{
    // Arrange
    const QString deviceFiles = "usb-0000:00:1d.0-1/input0/event5";
    // Act
    QString eventStr = di->eventStrFromDeviceFiles(deviceFiles);
    // Assert
    EXPECT_EQ(eventStr, QString("event5"));
    EXPECT_TRUE(eventStr.startsWith("event"));
}

// 用例: 两位编号的 event 提取（边界值）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_EventStrFromDeviceFiles_TwoDigits_ExtractsEvent)
{
    // Arrange
    const QString deviceFiles = "/dev/input/event12";
    // Act
    QString eventStr = di->eventStrFromDeviceFiles(deviceFiles);
    // Assert
    EXPECT_EQ(eventStr, QString("event12"));
    EXPECT_NE(eventStr, QString("event1"));
}

// 用例: 无 event 关键字返回空（无效输入等价类）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_EventStrFromDeviceFiles_NoEvent_ReturnsEmpty)
{
    // Arrange
    const QString deviceFiles = "psmouse serio1";
    // Act
    QString eventStr = di->eventStrFromDeviceFiles(deviceFiles);
    // Assert
    EXPECT_EQ(eventStr, QString(""));
    EXPECT_TRUE(eventStr.isEmpty());
}

// ===================== getDetailBusInfo（B1-B4） =====================
// 用例: USB 总线 ID 查表返回 USB 信息
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_GetDetailBusInfo_UsbId_ReturnsUsbInfo)
{
    // Arrange
    const QString busId = "0003";
    // Act
    InputDeviceBusInfo info = di->getDetailBusInfo(busId);
    // Assert
    EXPECT_EQ(info.busId, QString("0003"));
    EXPECT_EQ(info.interfaceType, QString("USB"));
    EXPECT_EQ(info.typicalDevices, QString("USB鼠标/键盘/手柄"));
}

// 用例: PS/2 总线 ID 查表返回 PS/2 信息
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_GetDetailBusInfo_Ps2Id_ReturnsPs2Info)
{
    // Arrange
    const QString busId = "0011";
    // Act
    InputDeviceBusInfo info = di->getDetailBusInfo(busId);
    // Assert
    EXPECT_EQ(info.busId, QString("0011"));
    EXPECT_EQ(info.interfaceType, QString("PS/2"));
    EXPECT_EQ(info.description, QString("圆形 6-pin 接口，设备节点为 /dev/psaux"));
}

// 用例: 大写十六进制总线 ID 大小写不敏感匹配（边界值）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_GetDetailBusInfo_CaseInsensitive_Matches)
{
    // Arrange
    const QString busId = "001A";
    // Act
    InputDeviceBusInfo info = di->getDetailBusInfo(busId);
    // Assert
    EXPECT_EQ(info.busId, QString("001A"));
    EXPECT_EQ(info.interfaceType, QString("ILLUMINANCE"));
}

// 用例: 未知总线 ID 返回 Unknown 信息（无效输入等价类）
TEST_F(UT_DeviceInputPure, UT_DeviceInputPure_GetDetailBusInfo_UnknownId_ReturnsUnknown)
{
    // Arrange
    const QString busId = "0099";
    // Act
    InputDeviceBusInfo info = di->getDetailBusInfo(busId);
    // Assert
    EXPECT_EQ(info.busId, QString("0099"));
    EXPECT_EQ(info.interfaceType, QString("Unknown"));
    EXPECT_TRUE(info.description.contains("Unrecognized Bus ID"));
}
