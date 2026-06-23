// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 PageDriverManager 的 getDownloadInfo(纯算术格式化) + testDevices(构造测试 DriverInfo)。

#include <gtest/gtest.h>

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "PageDriverManager.h"

static void ut_pdmm_installNext()
{
    return;
}
static void ut_pdmm_backupNext()
{
    return;
}

class UT_PageDriverManagerMisc : public UT_HEAD
{
public:
    void SetUp() override
    {
        stub.set(ADDR(PageDriverManager, installNextDriver), ut_pdmm_installNext);
        stub.set(ADDR(PageDriverManager, backupNextDriver), ut_pdmm_backupNext);
        m = new PageDriverManager;
    }
    void TearDown() override
    {
    }
    PageDriverManager *m = nullptr;
    Stub stub;
};

TEST_F(UT_PageDriverManagerMisc, getDownloadInfo)
{
    QString speed, size;
    EXPECT_NO_FATAL_FAILURE(m->getDownloadInfo(50, 1000000, speed, size));
    EXPECT_NO_FATAL_FAILURE(m->getDownloadInfo(100, 0, speed, size));
}

TEST_F(UT_PageDriverManagerMisc, testDevices)
{
    EXPECT_NO_FATAL_FAILURE(m->testDevices());
}
