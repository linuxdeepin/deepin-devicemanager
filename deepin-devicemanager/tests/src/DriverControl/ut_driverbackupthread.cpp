// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "DriverBackupThread.h"

class UT_DriverBackupThread : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_thread = new DriverBackupThread;
    }
    void TearDown() override
    {
        delete m_thread;
    }
    DriverBackupThread *m_thread = nullptr;
};

// 构造函数：初始状态
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_ctor)
{
    EXPECT_EQ(true, m_thread->m_isStop);
    EXPECT_EQ(nullptr, m_thread->mp_driverInfo);
    EXPECT_EQ(DriverBackupThread::Waiting, m_thread->m_status);
}

// setBackupDriverInfo：设置后状态翻转
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_setBackupDriverInfo)
{
    DriverInfo info;
    m_thread->setBackupDriverInfo(&info);
    EXPECT_FALSE(m_thread->m_isStop);
    EXPECT_EQ(&info, m_thread->mp_driverInfo);
    EXPECT_EQ(DriverBackupThread::Waiting, m_thread->m_status);
}

// undoBackup：标记停止
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_undoBackup)
{
    DriverInfo info;
    m_thread->setBackupDriverInfo(&info);
    EXPECT_FALSE(m_thread->m_isStop);
    m_thread->undoBackup();
    EXPECT_TRUE(m_thread->m_isStop);
}

// setStatus：设置各状态
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_setStatus)
{
    m_thread->setStatus(DriverBackupThread::Success);
    EXPECT_EQ(DriverBackupThread::Success, m_thread->m_status);
    m_thread->setStatus(DriverBackupThread::Failed);
    EXPECT_EQ(DriverBackupThread::Failed, m_thread->m_status);
    m_thread->setStatus(DriverBackupThread::Waiting);
    EXPECT_EQ(DriverBackupThread::Waiting, m_thread->m_status);
}

// run：默认状态(m_isStop=true)走 else 分支，安全返回
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_run_default)
{
    // 不设置 driverInfo，直接调用 run()，命中 else 分支
    m_thread->run();
    SUCCEED();
}

// run：设置了 driverInfo 但 packages 与 debVersion 都为空 => emit finished(false) 后返回
TEST_F(UT_DriverBackupThread, UT_DriverBackupThread_run_emptyPackage)
{
    DriverInfo info;
    // packages() 与 debVersion() 默认为空 => 命中空包名分支
    m_thread->setBackupDriverInfo(&info);
    m_thread->run();
    // 执行到此说明未崩溃
    SUCCEED();
}
