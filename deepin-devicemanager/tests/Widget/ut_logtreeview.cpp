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
#include "../src/Widget/logtreeview.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class LogTreeView_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_logTreeView;
    }
    LogTreeView *m_logTreeView;
};

TEST_F(LogTreeView_UT, UT_setHeaderLabels)
{
    m_logTreeView = new LogTreeView;
    m_logTreeView->setHeaderLabels(QStringList() << "/");
}

TEST_F(LogTreeView_UT, UT_currentRowEnable)
{
    m_logTreeView = new LogTreeView;
    ASSERT_FALSE(m_logTreeView->currentRowEnable());
    ASSERT_NE(m_logTreeView->currentRow(), 0);
}

TEST_F(LogTreeView_UT, UT_updateCurItemEnable)
{
    m_logTreeView = new LogTreeView;
    m_logTreeView->updateCurItemEnable(0, 1);
}

TEST_F(LogTreeView_UT, UT_paintEvent)
{
    m_logTreeView = new LogTreeView;
    QPaintEvent paint(QRect(m_logTreeView->rect()));
    m_logTreeView->paintEvent(&paint);
}

TEST_F(LogTreeView_UT, UT_setRowNum)
{
    m_logTreeView = new LogTreeView;
    m_logTreeView->setRowNum(0);
    m_logTreeView->clear();
    ASSERT_EQ(m_logTreeView->RowNum(), 0);
}
