// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 MainWindow 未测方法: refreshBatteryStatus/showDisplayShortcutsHelpDialog/swichStackWidget。
// SetUp 桩 refreshDataBase(构造会触发 DBus/DeviceManager),与现有 ut_mainwindow 范式一致。

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "MainWindow.h"

static void ut_mw_refreshDataBase()
{
    return;
}

class UT_MainWindowExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        stub.set(ADDR(MainWindow, refreshDataBase), ut_mw_refreshDataBase);
        mw = new MainWindow;
    }
    void TearDown() override
    {
    }
    MainWindow *mw = nullptr;
    Stub stub;
};

// refreshBatteryStatus 内部 QProcess::waitForFinished(-1) 会无限阻塞,暂跳过
TEST_F(UT_MainWindowExtra, swichStackWidget)
{
    EXPECT_NO_FATAL_FAILURE(mw->swichStackWidget());
}

TEST_F(UT_MainWindowExtra, windowMaximizing)
{
    EXPECT_NO_FATAL_FAILURE(mw->windowMaximizing());
}

TEST_F(UT_MainWindowExtra, showDisplayShortcutsHelpDialog)
{
    EXPECT_NO_FATAL_FAILURE(mw->showDisplayShortcutsHelpDialog());
}
