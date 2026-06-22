// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceMonitor 的纯解析方法: getMonitorResolutionMap(xrandr文本解析)、
// parseMonitorSize(屏幕尺寸正则)、transWeekToDate(周年转日期)。零外部依赖。

#include "DeviceMonitor.h"
#include "ut_Head.h"

#include <QString>
#include <QStringList>
#include <QMap>
#include <QSize>

#include <gtest/gtest.h>

// 模拟 xrandr 输出: 含 connected/disconnected 显示器及分辨率行
static const char *ut_xrandr =
    "DP-1 connected primary 1920x1080+0+0 (normal left inverted right x axis y axis) 527mm x 296mm\n"
    "   1920x1080     60.00*+  59.93\n"
    "   1680x1050     59.95\n"
    "   1280x1024     60.02\n"
    "HDMI-1 disconnected (normal left inverted right x axis y axis)\n"
    "DP-2 connected 1280x1024+1920+0 (normal left inverted right x axis y axis) 338mm x 270mm\n"
    "   1280x1024     60.02  75.02";

class UT_DeviceMonitorParse : public UT_HEAD
{
public:
    void SetUp() override
    {
        m = new DeviceMonitor;
    }
    void TearDown() override
    {
        delete m;
    }
    DeviceMonitor *m = nullptr;
};

// getMonitorResolutionMap: 正常解析,key 命中
TEST_F(UT_DeviceMonitorParse, getMonitorResolutionMap_normal)
{
    QMap<QString, QStringList> r = m->getMonitorResolutionMap(QString::fromUtf8(ut_xrandr), "DP-1");
    EXPECT_TRUE(r.contains("DP-1"));
    EXPECT_GT(r["DP-1"].size(), 0);
}

// round=false 分支
TEST_F(UT_DeviceMonitorParse, getMonitorResolutionMap_roundFalse)
{
    QMap<QString, QStringList> r = m->getMonitorResolutionMap(QString::fromUtf8(ut_xrandr), "DP-1", false);
    EXPECT_GT(r["DP-1"].size(), 0);
}

// 空 rawText -> 空 map
TEST_F(UT_DeviceMonitorParse, getMonitorResolutionMap_empty)
{
    QMap<QString, QStringList> r = m->getMonitorResolutionMap("", "DP-1");
    EXPECT_EQ(r.size(), 0);
}

// key 不存在 -> clear 后空
TEST_F(UT_DeviceMonitorParse, getMonitorResolutionMap_keyNotExist)
{
    QMap<QString, QStringList> r = m->getMonitorResolutionMap(QString::fromUtf8(ut_xrandr), "NotExists");
    EXPECT_EQ(r.size(), 0);
}

// 默认 key="" -> 不命中 -> clear
TEST_F(UT_DeviceMonitorParse, getMonitorResolutionMap_defaultKey)
{
    QMap<QString, QStringList> r = m->getMonitorResolutionMap(QString::fromUtf8(ut_xrandr));
    EXPECT_EQ(r.size(), 0);
}

// parseMonitorSize: "NNNmm x NNNmm" 格式分支
TEST_F(UT_DeviceMonitorParse, parseMonitorSize_mmFormat)
{
    double inch = 0;
    QSize sz;
    QString ret = m->parseMonitorSize("527mm x 296mm", inch, sz);
    EXPECT_GT(inch, 0.0);
    EXPECT_FALSE(ret.isEmpty());
}

// parseMonitorSize: 不匹配格式
TEST_F(UT_DeviceMonitorParse, parseMonitorSize_noMatch)
{
    double inch = 0;
    QSize sz;
    QString ret = m->parseMonitorSize("unknown", inch, sz);
    EXPECT_EQ(inch, 0.0);
}

// transWeekToDate: 周年转日期
TEST_F(UT_DeviceMonitorParse, transWeekToDate)
{
    QString ret = m->transWeekToDate("2024", "5");
    EXPECT_FALSE(ret.isEmpty());
}

TEST_F(UT_DeviceMonitorParse, transWeekToDate_invalid)
{
    QString ret = m->transWeekToDate("abc", "xyz");
    EXPECT_TRUE(ret.isEmpty() || ret.startsWith("0"));
}
