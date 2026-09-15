// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 DeviceBaseInfo 基类中此前未覆盖的 setEnable（基类默认实现恒返回 EDS_Success）。
// 通过限定名调用基类版本，避免命中子类 override。

#include "DevicePrint.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

TEST(UT_DeviceBaseInfoExtra, SetEnable_BaseImpl_ReturnsSuccess)
{
    DevicePrint *d = new DevicePrint;
    // 限定调用基类 DeviceBaseInfo::setEnable，覆盖基类默认实现
    EnableDeviceStatus ret = d->DeviceBaseInfo::setEnable(true);
    EXPECT_EQ(EDS_Success, ret);
    delete d;
}
