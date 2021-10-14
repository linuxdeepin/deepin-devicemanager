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
#include "PageInfo.h"
#include "WaitingWidget.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_WaitingWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_waitingWidget = new WaitingWidget;
    }
    void TearDown()
    {
        delete m_waitingWidget;
    }
    WaitingWidget *m_waitingWidget = nullptr;
};

TEST_F(UT_WaitingWidget, UT_WaitingWidget_start)
{
    m_waitingWidget->start();
    EXPECT_TRUE(m_waitingWidget->mp_Spinner->isPlaying());
    m_waitingWidget->stop();
    EXPECT_FALSE(m_waitingWidget->mp_Spinner->isPlaying());
}
