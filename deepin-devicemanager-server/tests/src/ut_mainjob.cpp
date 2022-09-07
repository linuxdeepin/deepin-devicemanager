// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MainJob.h"
#include "ut_Head.h"
#include <gtest/gtest.h>
#include "stub.h"

#include <QProcess>

class MainJob_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_mainJob = new MainJob;
    }
    void TearDown()
    {
        delete m_mainJob;
    }
    MainJob *m_mainJob = nullptr;
};

void ut_process_start()
{
    return;
}

bool ut_initDBus()
{
    return true;
}

TEST_F(MainJob_UT, MainJob_UT_working)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, QIODevice::OpenMode))ADDR(QProcess, start), ut_process_start);
    //    stub.set(ADDR(MainJob,initDBus),ut_initDBus);
    m_mainJob->working();
}

TEST_F(MainJob_UT, MainJob_UT_executeClientInstruction)
{
    m_mainJob->m_FirstUpdate = true;
    m_mainJob->executeClientInstruction("UPDATE_UI");
}

TEST_F(MainJob_UT, MainJob_UT_isZhaoXin)
{
    //    m_mainJob->isZhaoXin();
}

TEST_F(MainJob_UT, MainJob_UT_slotUsbChanged)
{
    m_mainJob->slotUsbChanged();
}

TEST_F(MainJob_UT, MainJob_UT_onFirstUpdate)
{
    m_mainJob->onFirstUpdate();
}

TEST_F(MainJob_UT, MainJob_UT_initDBus)
{
    m_mainJob->initDBus();
}
