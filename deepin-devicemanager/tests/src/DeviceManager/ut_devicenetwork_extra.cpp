// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 DeviceNetwork::vendor / isWireless / hwAddress / canDisable 访问器。
// 利用 -fno-access-control 直接设置受保护成员后调用 getter 校验返回值。

#include "DeviceNetwork.h"
#include "DeviceInfo.h"
#include "DBusEnableInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class UT_DeviceNetworkAccessors : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceNetwork = new DeviceNetwork;
    }
    void TearDown()
    {
        delete m_deviceNetwork;
    }
    DeviceNetwork *m_deviceNetwork = nullptr;
};

// const QString &vendor() const;
TEST_F(UT_DeviceNetworkAccessors, Vendor_MemberSet_ReturnsSameValue)
{
    m_deviceNetwork->m_Vendor = "Intel";
    EXPECT_STREQ("Intel", m_deviceNetwork->vendor().toStdString().c_str());
}

// bool isWireless();
TEST_F(UT_DeviceNetworkAccessors, IsWireless_MemberTrue_ReturnsTrue)
{
    m_deviceNetwork->m_IsWireless = true;
    EXPECT_TRUE(m_deviceNetwork->isWireless());
}

TEST_F(UT_DeviceNetworkAccessors, IsWireless_MemberFalse_ReturnsFalse)
{
    m_deviceNetwork->m_IsWireless = false;
    EXPECT_FALSE(m_deviceNetwork->isWireless());
}

// QString hwAddress();
TEST_F(UT_DeviceNetworkAccessors, HwAddress_MemberSet_ReturnsSameValue)
{
    m_deviceNetwork->m_MACAddress = "f4:b5:20:24:5e:4f";
    EXPECT_STREQ("f4:b5:20:24:5e:4f", m_deviceNetwork->hwAddress().toStdString().c_str());
}

// bool canDisable();
TEST_F(UT_DeviceNetworkAccessors, CanDisable_SysPathEmpty_ReturnsFalse)
{
    m_deviceNetwork->m_SysPath = "";
    EXPECT_FALSE(m_deviceNetwork->canDisable());
}

TEST_F(UT_DeviceNetworkAccessors, CanDisable_SysPathSet_ReturnsTrue)
{
    m_deviceNetwork->m_SysPath = "/sys/devices/pci0000:00/0000:00:1c.0";
    EXPECT_TRUE(m_deviceNetwork->canDisable());
}
