// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceInput 的纯逻辑/简单方法(getInterface/bluetoothIsConnected/wakeupPath/
// canWakeupMachine/isWakeupMachine/eventStrFromDeviceFiles/getPS2Syspath)。

#include "ut_Head.h"
#include "stub.h"
#include "DeviceManager.h"

#define private public
#define protected public
#include "DeviceInput.h"

#include <QString>

#include <gtest/gtest.h>

class UT_DeviceInputPure : public UT_HEAD
{
public:
    void SetUp() override
    {
        di = new DeviceInput;
    }
    void TearDown() override
    {
        delete di;
    }
    DeviceInput *di = nullptr;
};

TEST_F(UT_DeviceInputPure, getInterface)
{
    di->m_Interface = "USB";
    EXPECT_EQ(di->getInterface(), QString("USB"));
    di->m_Interface = "Bluetooth";
    EXPECT_EQ(di->getInterface(), QString("Bluetooth"));
}

TEST_F(UT_DeviceInputPure, bluetoothIsConnected)
{
    EXPECT_NO_FATAL_FAILURE(di->bluetoothIsConnected());
    di->m_BluetoothIsConnected = false;
    EXPECT_FALSE(di->bluetoothIsConnected());
}

TEST_F(UT_DeviceInputPure, wakeupPath)
{
    di->m_Name = "USB Mouse";
    di->m_SysPath = "/devices/pci0000:00/usb1/1-1/input/input1";
    EXPECT_NO_FATAL_FAILURE(di->wakeupPath());
}

TEST_F(UT_DeviceInputPure, canWakeupMachine)
{
    EXPECT_NO_FATAL_FAILURE(di->canWakeupMachine());
}

TEST_F(UT_DeviceInputPure, isWakeupMachine)
{
    EXPECT_NO_FATAL_FAILURE(di->isWakeupMachine());
}

TEST_F(UT_DeviceInputPure, eventStrFromDeviceFiles)
{
    EXPECT_NO_FATAL_FAILURE(di->eventStrFromDeviceFiles("/dev/input/event3"));
    EXPECT_NO_FATAL_FAILURE(di->eventStrFromDeviceFiles("/dev/input/mouse0"));
}

TEST_F(UT_DeviceInputPure, getPS2Syspath)
{
    EXPECT_NO_FATAL_FAILURE(di->getPS2Syspath("/dev/input/mouse0"));
    EXPECT_NO_FATAL_FAILURE(di->getPS2Syspath(""));
}

static bool ut_dip_isExist(DeviceManager *, const QString &)
{
    return true;
}

TEST_F(UT_DeviceInputPure, setInfoFromBluetoothctl)
{
    di->m_keysToPairedDevice = "AA:BB:CC:DD:EE:FF";
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromBluetoothctl());
    Stub stub;
    stub.set(ADDR(DeviceManager, isDeviceExistInPairedDevice), ut_dip_isExist);
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromBluetoothctl());
}
