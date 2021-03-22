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
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class LogViewItemDelegate_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_view = new LogTreeView;
        m_logDelegate = new LogViewItemDelegate(m_view);
    }
    void TearDown()
    {
        delete m_logDelegate;
        delete m_view;
    }
    LogViewItemDelegate *m_logDelegate;
    LogTreeView *m_view;
};

TEST_F(LogViewItemDelegate_UT, ut_paint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->paint(&painter, option, index);
}

TEST_F(LogViewItemDelegate_UT, ut_createEditor)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->createEditor(nullptr, option, index);
}

TEST_F(LogViewItemDelegate_UT, ut_sizeHint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);
    m_logDelegate->sizeHint(option, index);
}
