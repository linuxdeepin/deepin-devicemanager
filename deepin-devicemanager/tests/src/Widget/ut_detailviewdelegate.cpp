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
#include "../src/Widget/DetailViewDelegate.h"
#include "../src/Widget/DetailTreeView.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QCommonStyle>
#include <QStyle>
#include <QWidget>
#include <DStyle>

#include <gtest/gtest.h>
#include "../stub.h"

class DetailViewDelegate_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_treeView = new DetailTreeView;
        m_dViewDelegate = new DetailViewDelegate(m_treeView);
    }
    void TearDown()
    {
        delete m_treeView;
    }
    DetailViewDelegate *m_dViewDelegate;
    DetailTreeView *m_treeView;
};

bool ut_isValid()
{
    return true;
}

int ut_detailview_pixelMetric()
{
    return 10;
}

int ut_y()
{
    return 10;
}

int ut_column()
{
    return 0;
}

TEST_F(DetailViewDelegate_UT, ut_paint)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), ut_isValid);
    stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget *widget) const)ADDR(DStyle, pixelMetric), ut_detailview_pixelMetric);
    stub.set(ADDR(QPoint, y), ut_y);
    stub.set(ADDR(QModelIndex, column), ut_column);
    m_dViewDelegate->paint(&painter, option, index);
    delete item;
}

TEST_F(DetailViewDelegate_UT, ut_createEditor)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    m_dViewDelegate->createEditor(nullptr, m_item, index);
    delete item;
}

TEST_F(DetailViewDelegate_UT, ut_sizeHint)
{
    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    m_dViewDelegate->sizeHint(m_item, index);
    delete item;
}
