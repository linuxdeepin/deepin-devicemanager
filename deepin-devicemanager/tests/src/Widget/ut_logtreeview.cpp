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

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QWidget>
#include <DStyle>

#include <gtest/gtest.h>
#include "../stub.h"

class LogTreeView_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_logTreeView = new LogTreeView;
    }
    void TearDown()
    {
        delete m_logTreeView;
    }
    LogTreeView *m_logTreeView;
};

TEST_F(LogTreeView_UT, UT_setHeaderLabels)
{
    m_logTreeView->setHeaderLabels(QStringList() << "/");
}

TEST_F(LogTreeView_UT, UT_currentRowEnable)
{
    ASSERT_FALSE(m_logTreeView->currentRowEnable());
    ASSERT_NE(m_logTreeView->currentRow(), 0);
}

TEST_F(LogTreeView_UT, UT_updateCurItemEnable)
{
    m_logTreeView->updateCurItemEnable(0, 1);
}

TEST_F(LogTreeView_UT, UT_paintEvent)
{
    QPaintEvent paint(QRect(m_logTreeView->rect()));
    m_logTreeView->paintEvent(&paint);
}

int ut_treeview_pixelMetric()
{
    return 10;
}

TEST_F(LogTreeView_UT, UT_drawRow)
{
    //    QStyleOptionViewItem option;
    //    QPainter painter(m_logTreeView);
    //    QModelIndex index;
    //    Stub stub;
    //    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *widget) const)ADDR(DStyle, pixelMetric), ut_treeview_pixelMetric);
    //    m_logTreeView->drawRow(&painter, option, index);
}

TEST_F(LogTreeView_UT, UT_setRowNum)
{
    m_logTreeView->setRowNum(0);
    m_logTreeView->clear();
    ASSERT_EQ(m_logTreeView->RowNum(), 0);
}

TEST_F(LogTreeView_UT, ut_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Up, Qt::ShiftModifier);
    QCoreApplication::sendEvent(m_logTreeView, &keyPressEvent);
}
