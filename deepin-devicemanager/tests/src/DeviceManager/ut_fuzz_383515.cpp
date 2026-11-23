// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS BUG-383515 模糊测试问题回归用例
// 说明：ut_devicemonitor.cpp 中现存用例仍使用旧版 setInfoFromXradr() 三参数接口，
// 无法编译，故此处单独收敛与本次安全修复相关的用例。

#include "DeviceMonitor.h"

#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>

#include <gtest/gtest.h>

#include <limits>

DWIDGET_USE_NAMESPACE

class UT_DeviceMonitorFuzz : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceMonitor = new DeviceMonitor;
    }
    void TearDown()
    {
        delete m_deviceMonitor;
    }
    DeviceMonitor *m_deviceMonitor = nullptr;
};

// PMS BUG-383515 用例：fuzz_devicemonitor.cpp/devicemonitor_gcd_fuzz()
// gcd() 未处理 0 参数，a % b 触发除零崩溃
TEST_F(UT_DeviceMonitorFuzz, UT_DeviceMonitorFuzz_gcd_zero_383515)
{
    EXPECT_EQ(0, m_deviceMonitor->gcd(0, 0));
    EXPECT_EQ(100, m_deviceMonitor->gcd(0, 100));
    EXPECT_EQ(100, m_deviceMonitor->gcd(100, 0));
    EXPECT_EQ(-5, m_deviceMonitor->gcd(-5, 0));
    EXPECT_EQ(1920, m_deviceMonitor->gcd(0, 1920));

    // 正常输入结果不变
    EXPECT_EQ(120, m_deviceMonitor->gcd(1920, 1080));
    EXPECT_EQ(10, m_deviceMonitor->gcd(10, 10));
    EXPECT_EQ(1, m_deviceMonitor->gcd(10, 11));
}

// PMS BUG-383515 用例：负数参数会导致无限递归/取模溢出
TEST_F(UT_DeviceMonitorFuzz, UT_DeviceMonitorFuzz_gcd_negative_383515)
{
    EXPECT_EQ(4, m_deviceMonitor->gcd(-8, 12));
    EXPECT_EQ(4, m_deviceMonitor->gcd(8, -12));
    EXPECT_EQ(4, m_deviceMonitor->gcd(-8, -12));
}

// PMS BUG-383515 用例：INT_MIN 取反与 a % b 属于未定义行为
TEST_F(UT_DeviceMonitorFuzz, UT_DeviceMonitorFuzz_gcd_extremum_383515)
{
    const int intMin = std::numeric_limits<int>::min();

    EXPECT_EQ(1, m_deviceMonitor->gcd(intMin, -1));
    EXPECT_EQ(1, m_deviceMonitor->gcd(-1, intMin));
    EXPECT_EQ(std::numeric_limits<int>::max(), m_deviceMonitor->gcd(intMin, intMin));
    EXPECT_EQ(intMin, m_deviceMonitor->gcd(intMin, 0));
}

// PMS BUG-383515 用例：分辨率为 0x0 时 caculateScreenRatio() 以零参数调用 gcd()
TEST_F(UT_DeviceMonitorFuzz, UT_DeviceMonitorFuzz_caculateScreenRatio_zero_383515)
{
    m_deviceMonitor->m_AspectRatio = "";
    m_deviceMonitor->m_CurrentResolution = "0x0";
    m_deviceMonitor->caculateScreenRatio();
    EXPECT_TRUE(m_deviceMonitor->m_AspectRatio.isEmpty());

    m_deviceMonitor->m_CurrentResolution = "1920x0";
    m_deviceMonitor->caculateScreenRatio();
    EXPECT_TRUE(m_deviceMonitor->m_AspectRatio.isEmpty());

    m_deviceMonitor->m_CurrentResolution = "0x1080";
    m_deviceMonitor->caculateScreenRatio();
    EXPECT_TRUE(m_deviceMonitor->m_AspectRatio.isEmpty());

    // 正常分辨率结果不变
    m_deviceMonitor->m_CurrentResolution = "1920x1080";
    m_deviceMonitor->caculateScreenRatio();
    EXPECT_STREQ("16 : 9", m_deviceMonitor->m_AspectRatio.toStdString().c_str());
}
