// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 ThreadExecXrandr 未测方法: getMonitorInfoFromXrandrVerbose/getGpuInfoFromXrandr。
// 桩 runCmd(成员方法,首参 this) + 桩 DeviceManager::setMonitorInfo/setGpuInfoFromXrandr 避免污染单例。

#include "ThreadExecXrandr.h"
#include "DeviceManager.h"
#include "ut_Head.h"
#include "stub.h"

#include <QMap>
#include <QString>
#include <QList>

#include <gtest/gtest.h>

// 桩 runCmd(成员方法首参 this)
static QString ut_txr_out = QStringLiteral("normal");
static void ut_txr_runCmd(ThreadExecXrandr * /*self*/, QString &info, const QString & /*cmd*/)
{
    info = ut_txr_out;
}
// 桩 DeviceManager 方法(成员首参 this)
static void ut_txr_setMonitor(DeviceManager * /*self*/, const QString & /*a*/, const QString & /*b*/, const QString & /*c*/, const QString & /*d*/)
{
    return;
}
static void ut_txr_setGpu(DeviceManager * /*self*/, const QMap<QString, QString> & /*m*/)
{
    return;
}
static void ut_txr_getResolution(ThreadExecXrandr * /*self*/, QMap<QString, QString> & /*m*/)
{
    return;
}

class UT_ThreadExecXrandrMore : public UT_HEAD
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

TEST_F(UT_ThreadExecXrandrMore, getMonitorInfoFromXrandrVerbose)
{
    Stub stub;
    ut_txr_out = QStringLiteral("DP-1 connected primary 1920x1080+0+0\n");
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_txr_runCmd);
    stub.set(ADDR(DeviceManager, setMonitorInfoFromXrandr), ut_txr_setMonitor);
    EXPECT_NO_FATAL_FAILURE(t->getMonitorInfoFromXrandrVerbose());
}

TEST_F(UT_ThreadExecXrandrMore, getGpuInfoFromXrandr)
{
    Stub stub;
    ut_txr_out = QStringLiteral("Screen 0: current 1920 x 1080\n");
    stub.set(ADDR(ThreadExecXrandr, runCmd), ut_txr_runCmd);
    stub.set(ADDR(ThreadExecXrandr, getResolutionFromDBus), ut_txr_getResolution);
    stub.set(ADDR(DeviceManager, setGpuInfoFromXrandr), ut_txr_setGpu);
    EXPECT_NO_FATAL_FAILURE(t->getGpuInfoFromXrandr());
}
