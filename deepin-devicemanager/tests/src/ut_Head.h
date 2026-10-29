// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
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
 * 单测不得触碰真实系统服务。隔离由双层兑底(均在 test_main.cpp, 不依赖本函数):
 * 1. 连接层防火墙: 拦截 QDBusConnection::call/asyncCall/send, 真实出网调用
 *    即时返回 ServiceUnknown 错误应答;
 * 2. DBUS_SYSTEM_BUS_ADDRESS 死路径: gdbus 子进程回退连不上真实总线。
 * 二者合围后, org.deepin.DeviceInfo.getInfo 等鉴权方法永不触发 polkit 弹窗。
 *
 * 历史教训(勿回退为打桩方案):
 * - 桩 DBusInterface::getInfo(static Stub 永不恢复)会永久拦截
 *   UT_DBusInterface.GetInfo_ValidReply 等直接测 getInfo 的用例(顺序污染);
 * - 桩 QDBusConnection::systemBus() 返回空连接, 会令 dtkcore DConfig 内部
 *   对 interface() 的解引用崩溃(空连接 interface() 为 nullptr)。
 * 故本函数保持为空实现, 仅保留作为套件 SetUp 的隔离声明锚点。
 */
inline void ut_disable_system_dbus()
{
    // 隔离已由进程级双层兑底承担, 无需逐套件打桩
}
#endif
