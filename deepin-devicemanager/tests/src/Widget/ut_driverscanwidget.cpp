// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DriverScanWidget 的状态切换 UI 方法(setScanningUI/setScanFailedUI 等)。

#include "DriverScanWidget.h"
#include "ut_Head.h"

#include <QString>

#include <gtest/gtest.h>

class UT_DriverScanWidget : public UT_HEAD
{
public:
    void SetUp() override
    {
        w = new DriverScanWidget;
    }
    void TearDown() override
    {
        delete w;
    }
    DriverScanWidget *w = nullptr;
};

TEST_F(UT_DriverScanWidget, setScanningUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setScanningUI("scanning device", 50));
}

TEST_F(UT_DriverScanWidget, setScanFailedUI)
{
    EXPECT_NO_FATAL_FAILURE(w->setScanFailedUI());
}

TEST_F(UT_DriverScanWidget, setNetworkErr)
{
    EXPECT_NO_FATAL_FAILURE(w->setNetworkErr());
}

TEST_F(UT_DriverScanWidget, setProgressFinish)
{
    EXPECT_NO_FATAL_FAILURE(w->setProgressFinish());
}

TEST_F(UT_DriverScanWidget, refreshProgress)
{
    EXPECT_NO_FATAL_FAILURE(w->refreshProgress("info", 10));
    EXPECT_NO_FATAL_FAILURE(w->refreshProgress("more", 20));
}

TEST_F(UT_DriverScanWidget, slotReDetected)
{
    EXPECT_NO_FATAL_FAILURE(w->slotReDetected());
}
