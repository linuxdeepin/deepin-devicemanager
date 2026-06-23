// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 PageDriverManager 未测的 slot 方法 + 状态 getter。
// SetUp 桩 installNextDriver/backupNextDriver(避免触发真实安装/备份线程),与现有 ut 范式一致。

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"

static void ut_pdm_installNextDriver()
{
    return;
}
static void ut_pdm_backupNextDriver()
{
    return;
}

class UT_PageDriverManagerSlot : public UT_HEAD
{
public:
    void SetUp() override
    {
        stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdm_installNextDriver);
        stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pdm_backupNextDriver);
        m = new PageDriverManager;
    }
    void TearDown() override
    {
    }
    PageDriverManager *m = nullptr;
    Stub stub;
};

TEST_F(UT_PageDriverManagerSlot, slotListViewWidgetItemClicked)
{
    EXPECT_NO_FATAL_FAILURE(m->slotListViewWidgetItemClicked("driverinstall"));
    EXPECT_NO_FATAL_FAILURE(m->slotListViewWidgetItemClicked("driverbackup"));
    EXPECT_NO_FATAL_FAILURE(m->slotListViewWidgetItemClicked("driversoftware"));
    EXPECT_NO_FATAL_FAILURE(m->slotListViewWidgetItemClicked("unknown"));
}

TEST_F(UT_PageDriverManagerSlot, slotInstallAllDrivers)
{
    EXPECT_NO_FATAL_FAILURE(m->slotInstallAllDrivers());
}

TEST_F(UT_PageDriverManagerSlot, slotBackupAllDrivers)
{
    EXPECT_NO_FATAL_FAILURE(m->slotBackupAllDrivers());
}

TEST_F(UT_PageDriverManagerSlot, slotUndoInstall)
{
    EXPECT_NO_FATAL_FAILURE(m->slotUndoInstall());
}

TEST_F(UT_PageDriverManagerSlot, slotUndoBackup)
{
    EXPECT_NO_FATAL_FAILURE(m->slotUndoBackup());
}

TEST_F(UT_PageDriverManagerSlot, slotBackupProgressChanged)
{
    EXPECT_NO_FATAL_FAILURE(m->slotBackupProgressChanged(50));
}

TEST_F(UT_PageDriverManagerSlot, state_getters)
{
    EXPECT_NO_FATAL_FAILURE(m->isFirstScan());
    EXPECT_NO_FATAL_FAILURE(m->isInstalling());
    EXPECT_NO_FATAL_FAILURE(m->isBackingup());
    EXPECT_NO_FATAL_FAILURE(m->isRestoring());
    EXPECT_NO_FATAL_FAILURE(m->isScanning());
}
