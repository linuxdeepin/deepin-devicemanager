// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_HEAD_HH
#define UT_HEAD_HH
#include <gtest/gtest.h>
#include <QLoggingCategory>
#include "stub.h"
#include "GenerateDevice/DBusInterface.h"

class UT_HEAD : public testing::Test
{
public:
};

/*
 * 单测不得触碰真实系统服务(运行时会向 org.deepin.DeviceInfo 系统总线发调用,
 * 在真机上会触发鉴权弹窗)。拦截 DBusInterface::getInfo 使其直接返回失败,
 * 被测代码将回退到本地打桩/临时文件路径, 与真实设备环境解耦。
 */
inline bool ut_stub_dbi_getinfo_off()
{
    return false;
}

inline void ut_disable_system_dbus()
{
    // 幂等: 仅进程内首次调用时打桩, 避免重复 Stub::set 造成累积泄漏
    static bool ut_system_dbus_done = false;
    if (ut_system_dbus_done)
        return;
    ut_system_dbus_done = true;
    static Stub ut_system_dbus_stub;
    ut_system_dbus_stub.set(ADDR(DBusInterface, getInfo), ut_stub_dbi_getinfo_off);
}
#endif
