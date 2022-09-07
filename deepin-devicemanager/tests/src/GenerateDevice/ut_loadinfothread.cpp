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
#include "DeviceWidget.h"
#include "LoadInfoThread.h"
#include "ThreadExecXrandr.h"
#include "GenerateDevicePool.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_LoadInfoThread : public UT_HEAD
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

class UT_ThreadExecXrandr : public UT_HEAD
{
public:
    void SetUp()
    {
        m_threadExecXrandr = new ThreadExecXrandr(true, true);
    }
    void TearDown()
    {
        delete m_threadExecXrandr;
    }
    ThreadExecXrandr *m_threadExecXrandr;
};

//TEST_F(UT_LoadInfoThread,UT_LoadInfoThread_start){
//    m_loadInfoThread->run();
//    EXPECT_FALSE(m_loadInfoThread->m_Running);
//}

//TEST_F(UT_ThreadExecXrandr,UT_ThreadExecXrandr_start){
//    m_threadExecXrandr->run();
//    EXPECT_TRUE(m_threadExecXrandr->m_Gpu);
//}


