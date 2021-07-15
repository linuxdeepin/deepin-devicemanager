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
#include <DApplication>

#include <gtest/gtest.h>
#include "../stub.h"
DStyle *DetailViewDelegate_UT_style;
class DetailViewDelegate_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_treeView = new DetailTreeView;
        m_dViewDelegate = new DetailViewDelegate(m_treeView);
        m_treeView->setItemDelegate(m_dViewDelegate);
        DetailViewDelegate_UT_style = new DStyle;
    }
    void TearDown()
    {
        delete m_treeView;
        delete  DetailViewDelegate_UT_style;
        DetailViewDelegate_UT_style = nullptr;
    }
    DetailViewDelegate *m_dViewDelegate;
    DetailTreeView *m_treeView;
};

bool ut_isValid()
{
    return false;
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

DStyle *DetailViewDelegate_UT_style_fun()
{
    return DetailViewDelegate_UT_style;
}

TEST_F(DetailViewDelegate_UT, ut_paint)
{
    QStyleOptionViewItem option;
    QPainter painter(m_treeView);
    m_treeView->setColumnCount(1);
    m_treeView->insertRow(0);
    QTableWidgetItem *item = new QTableWidgetItem("xxx");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->indexAt(QPoint(0, 0));

    Stub stub;

    //stub.set((int (DStyle::*)(DStyle::PixelMetric, const QStyleOption *, const QWidget * widget) const)ADDR(DStyle, pixelMetric), ut_detailview_pixelMetric);
    stub.set(ADDR(DApplication, style), DetailViewDelegate_UT_style_fun);
    stub.set(ADDR(QPoint, y), ut_y);
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
