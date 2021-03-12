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
#include "../src/Widget/logviewitemdelegate.h"
#include "../src/Widget/logtreeview.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class LogViewItemDelegate_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_logDelegate;
    }
    LogViewItemDelegate *m_logDelegate;
};

TEST_F(LogViewItemDelegate_UT, ut_paint)
{
    LogTreeView *m_view = new LogTreeView;
    m_logDelegate = new LogViewItemDelegate(m_view);
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->paint(&painter, option, index);
}

TEST_F(LogViewItemDelegate_UT, ut_createEditor)
{
    LogTreeView *m_view = new LogTreeView;
    m_logDelegate = new LogViewItemDelegate(m_view);
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->createEditor(nullptr, option, index);
}

TEST_F(LogViewItemDelegate_UT, ut_sizeHint)
{
    LogTreeView *m_view = new LogTreeView;
    m_logDelegate = new LogViewItemDelegate(m_view);
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->sizeHint(option, index);
}
