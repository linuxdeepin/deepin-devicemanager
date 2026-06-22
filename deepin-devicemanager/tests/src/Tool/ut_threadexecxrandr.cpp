// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 ThreadExecXrandr 的 xrandr 输出解析方法 loadXrandrInfo/loadXrandrVerboseInfo。
// 桩 runCmd 注入构造的 xrandr 输出,避免真实执行 xrandr 命令。
// 注: -fno-access-control 下可访问/调用 private 成员。

#include "ThreadExecXrandr.h"
#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QList>
#include <QString>

#include <gtest/gtest.h>

// 构造普通 xrandr 输出(含 Screen 分辨率 + HDMI/VGA/DP 接口分支)
static QString ut_xrandr_normal =
    "Screen 0: minimum 320 x 200, current 1920 x 1080, maximum 16384 x 16384\n"
    "HDMI-1 connected primary 1920x1080+0+0\n"
    "DP-1 connected 1280x1024+1920+0\n"
    "VGA-1 disconnected (normal)\n"
    "eDP-1 connected\n"
    "DVI-1 connected\n";

// 构造 verbose 输出(含 port + edid + current rate)
static QString ut_xrandr_verbose =
    "Screen 0: minimum 320 x 200\n"
    "DP-1 connected primary 1920x1080+0+0 (normal left) 527mm x 296mm\n"
    "\t\t00fffffffffe0012345678901234567890\n"
    "   1920x1080 (0x48) 60.00Hz*current +0+0\n";

// 桩 runCmd: 按 cmd 内容返回不同构造数据。
// 注: 被桩的是成员方法,桩函数首参须为 this 指针对齐(否则 info 参数会错位到 this)。
static QString ut_runCmd_mode = QStringLiteral("normal");
static void ut_runCmd(ThreadExecXrandr * /*self*/, QString &info, const QString &cmd)
{
    (void)cmd;
    info = (ut_runCmd_mode == "verbose") ? ut_xrandr_verbose : ut_xrandr_normal;
}

class UT_ThreadExecXrandr : public UT_HEAD
{
public:
    void SetUp() override
    {
        t = new ThreadExecXrandr(false, false);
    }
    void TearDown() override
    {
        delete t;
    }
    ThreadExecXrandr *t = nullptr;
};

// loadXrandrInfo: 解析 Screen 分辨率 + 各接口分支
TEST_F(UT_ThreadExecXrandr, loadXrandrInfo)
{
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_mode = "normal";

    QList<QMap<QString, QString> > lst;
    t->loadXrandrInfo(lst, "xrandr");
    EXPECT_GT(lst.size(), 0);
    // Screen 行触发一个空 map 并解析分辨率
    bool hasRes = false;
    for (const auto &m : lst) {
        if (m.contains("curResolution")) hasRes = true;
    }
    EXPECT_TRUE(hasRes);
}

// loadXrandrVerboseInfo: 解析 port + edid + rate
TEST_F(UT_ThreadExecXrandr, loadXrandrVerboseInfo)
{
    Stub stub;
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_runCmd);
    ut_runCmd_mode = "verbose";

    QList<QMap<QString, QString> > lst;
    EXPECT_NO_FATAL_FAILURE(t->loadXrandrVerboseInfo(lst, "xrandr --verbose"));
}

// getMonitorNumber: 初始为 0
TEST_F(UT_ThreadExecXrandr, getMonitorNumber)
{
    EXPECT_EQ(t->getMonitorNumber(), 0);
}
