// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QDBusInterface>
#include <QDBusConnection>

#define private public
#define protected public
#include "DBusWakeupInterface.h"

class UT_DBusWakeupInterface : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_iface = DBusWakeupInterface::getInstance();
        // 保存原始接口指针，强制注入无效接口使后续用例确定且不依赖真实 DBus 服务
        m_origInputIface = m_iface->mp_InputIface;
        m_origIface = m_iface->mp_Iface;
        m_iface->mp_InputIface = nullptr;  // 触发文件回退分支
        m_iface->mp_Iface = new QDBusInterface("org.nonexistent.NoService", "/x",
                                               "org.nonexistent.NoService",
                                               QDBusConnection::systemBus());
    }
    void TearDown() override
    {
        // 恢复原始指针，避免污染单例
        m_iface->mp_InputIface = m_origInputIface;
        m_iface->mp_Iface = m_origIface;
    }
    DBusWakeupInterface *m_iface = nullptr;
    QDBusInterface *m_origInputIface = nullptr;
    QDBusInterface *m_origIface = nullptr;
};

// 单例
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_getInstance)
{
    DBusWakeupInterface *a = DBusWakeupInterface::getInstance();
    DBusWakeupInterface *b = DBusWakeupInterface::getInstance();
    EXPECT_EQ(a, b);
    EXPECT_EQ(a, m_iface);
}

// setWakeupMachine：mp_InputIface=null 跳过输入分支，mp_Iface 无效返回 false
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_setWakeupMachine)
{
    bool ret = m_iface->setWakeupMachine("unique1", "/sys/devices/platform/foo", true, "USB Mouse", "mouse");
    EXPECT_FALSE(ret);
}

// canInputWakeupMachine：输入接口无效 => 文件回退，存在文件返回 true
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_canInputWakeupMachine_exists)
{
    EXPECT_TRUE(m_iface->canInputWakeupMachine("/etc/hostname"));
}

TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_canInputWakeupMachine_missing)
{
    EXPECT_FALSE(m_iface->canInputWakeupMachine("/tmp/nonexistent-wakeup-file-12345"));
}

// isInputWakeupMachine：输入接口无效 => 文件回退，不存在返回 false
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_isInputWakeupMachine_missing)
{
    EXPECT_FALSE(m_iface->isInputWakeupMachine("/tmp/nonexistent-wakeup-file-12345", "Mouse", "mouse", "USB"));
}

// isNetworkWakeup：mp_Iface 无效返回 -1
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_isNetworkWakeup)
{
    EXPECT_EQ(-1, m_iface->isNetworkWakeup("eth0"));
}

// setNetworkWakeup：mp_Iface 无效返回 false
TEST_F(UT_DBusWakeupInterface, UT_DBusWakeupInterface_setNetworkWakeup)
{
    EXPECT_FALSE(m_iface->setNetworkWakeup("eth0", true));
}
