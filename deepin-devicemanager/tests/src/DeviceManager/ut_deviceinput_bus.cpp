// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceInput::getDetailBusInfo(busId 查表返回 InputDeviceBusInfo)。

#include "ut_Head.h"

#define private public
#define protected public
#include "DeviceInput.h"

#include <QString>

#include <gtest/gtest.h>

class UT_DeviceInputBus : public UT_HEAD
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

TEST_F(UT_DeviceInputBus, getDetailBusInfo_various)
{
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo("0003"));   // USB
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo("0011"));   // PS/2
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo("0005"));   // Bluetooth
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo("0018"));   // I2C
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo("0099"));   // unknown
    EXPECT_NO_FATAL_FAILURE(di->getDetailBusInfo(""));       // empty
}
