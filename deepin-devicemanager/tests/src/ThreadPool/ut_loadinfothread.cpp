/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../src/Page/DeviceWidget.h"
#include "../src/ThreadPool/LoadInfoThread.h"
#include "../src/ThreadPool/ThreadExecXrandr.h"
#include "../src/ThreadPool/GenerateDevicePool.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class LoadInfoThread_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_loadInfoThread = new LoadInfoThread;
    }
    void TearDown()
    {
        delete m_loadInfoThread;
    }
    LoadInfoThread *m_loadInfoThread;
};

class ThreadExecXrandr_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_threadExecXrandr = new ThreadExecXrandr(true);
    }
    void TearDown()
    {
        delete m_threadExecXrandr;
    }
    ThreadExecXrandr *m_threadExecXrandr;
};

TEST_F(LoadInfoThread_UT, LoadInfoThread_UT_run)
{
    m_loadInfoThread->m_FinishedReadFilePool = true;
}

TEST_F(LoadInfoThread_UT, LoadInfoThread_UT_slotFinishedReadFilePool)
{
    m_loadInfoThread->slotFinishedReadFilePool("/");
    m_loadInfoThread->setFramework("/");
}

TEST_F(ThreadExecXrandr_UT, ThreadExecXrandr_UT_getGpuInfoFromXrandr)
{
    m_threadExecXrandr->getGpuInfoFromXrandr();
}
