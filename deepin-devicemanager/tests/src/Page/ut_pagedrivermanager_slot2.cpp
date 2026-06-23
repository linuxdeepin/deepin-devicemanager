// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 PageDriverManager 剩余 slot(下载/安装/还原/扫描/备份进度与完成回调)。

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"
#include "MacroDefinition.h"

#define private public
#include "PageDriverManager.h"

static void ut_pds2_install()
{
    return;
}
static void ut_pds2_backup()
{
    return;
}

class UT_PageDriverManagerSlot2 : public UT_HEAD
{
public:
    void SetUp() override
    {
        stub.set(ADDR(PageDriverManager, installNextDriver), ut_pds2_install);
        stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pds2_backup);
        m = new PageDriverManager;
    }
    void TearDown() override
    {
    }
    PageDriverManager *m = nullptr;
    Stub stub;
};

TEST_F(UT_PageDriverManagerSlot2, slotDownloadProgressChanged)
{
    QStringList msg;
    msg << "50%"
        << "1MB/s";
    EXPECT_NO_FATAL_FAILURE(m->slotDownloadProgressChanged(msg));
}

TEST_F(UT_PageDriverManagerSlot2, slotDownloadFinished)
{
    EXPECT_NO_FATAL_FAILURE(m->slotDownloadFinished());
}

TEST_F(UT_PageDriverManagerSlot2, slotInstallProgressChanged)
{
    EXPECT_NO_FATAL_FAILURE(m->slotInstallProgressChanged(75));
}

TEST_F(UT_PageDriverManagerSlot2, slotInstallProgressFinished)
{
    EXPECT_NO_FATAL_FAILURE(m->slotInstallProgressFinished(true, 0));
    EXPECT_NO_FATAL_FAILURE(m->slotInstallProgressFinished(false, 1));
}

TEST_F(UT_PageDriverManagerSlot2, slotRestoreProgress)
{
    EXPECT_NO_FATAL_FAILURE(m->slotRestoreProgress(50, "restoring driver"));
}

TEST_F(UT_PageDriverManagerSlot2, slotBackupFinished)
{
    EXPECT_NO_FATAL_FAILURE(m->slotBackupFinished(true));
    EXPECT_NO_FATAL_FAILURE(m->slotBackupFinished(false));
}

TEST_F(UT_PageDriverManagerSlot2, slotScanFinished)
{
    EXPECT_NO_FATAL_FAILURE(m->slotScanFinished(SR_SUCESS));
    EXPECT_NO_FATAL_FAILURE(m->slotScanFinished(SR_Failed));
}
