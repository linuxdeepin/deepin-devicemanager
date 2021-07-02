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
#include <DApplication>

#include <gtest/gtest.h>
#include "../stub.h"

DStyle *logViewItemDelegateSytle = nullptr;
class LogViewItemDelegate_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_Model = new QStandardItemModel;
        m_view = new LogTreeView;
        m_view->setModel(m_Model);
        m_Model->appendRow(new QStandardItem());
        m_logDelegate = new LogViewItemDelegate(m_view);
        logViewItemDelegateSytle = new DStyle;
    }
    void TearDown()
    {
        delete m_logDelegate;
        delete m_view;
        delete m_Model;
        delete logViewItemDelegateSytle;
    }
    LogViewItemDelegate *m_logDelegate;
    LogTreeView *m_view;
    QStandardItemModel *m_Model;
};

int ut_itemdelegate_pixelMetric()
{
    return 10;
}

bool ut_itemdelegate_isValid()
{
    return true;
}

DStyle *ut_style()
{
    return logViewItemDelegateSytle;
}

TEST_F(LogViewItemDelegate_UT, ut_paint)
{
    QPainter painter(m_view);
    QStyleOptionViewItem option;
    QModelIndex index = m_view->model()->index(0, 0);

    Stub stub;
    //stub.set(ADDR(QModelIndex, isValid), ut_itemdelegate_isValid);
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_itemdelegate_pixelMetric);
    stub.set(ADDR(DApplication, style), ut_style); // DApplication::style()
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
