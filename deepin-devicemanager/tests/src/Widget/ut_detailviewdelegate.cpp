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

#include <gtest/gtest.h>
#include "../stub.h"

class DetailViewDelegate_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dViewDelegate = new DetailViewDelegate(nullptr);
    }
    void TearDown()
    {
        delete m_dViewDelegate;
    }
    DetailViewDelegate *m_dViewDelegate;
};

TEST_F(DetailViewDelegate_UT, ut_paint)
{
    DetailTreeView *m_treeView = new DetailTreeView;

    QStyleOptionViewItem option;
    QPainter painter(m_treeView);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    m_dViewDelegate->paint(&painter, option, index);
    delete item;
    delete m_treeView;
}

TEST_F(DetailViewDelegate_UT, ut_createEditor)
{
    DetailTreeView *m_treeView = new DetailTreeView;

    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    m_dViewDelegate->createEditor(nullptr, m_item, index);
    delete item;
    delete m_treeView;
}

TEST_F(DetailViewDelegate_UT, ut_sizeHint)
{
    DetailTreeView *m_treeView = new DetailTreeView;

    QStyleOptionViewItem m_item;

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText("/");
    m_treeView->setItem(0, 0, item);
    QModelIndex index = m_treeView->model()->index(0, 0);

    m_dViewDelegate->sizeHint(m_item, index);
    delete item;
    delete m_treeView;
}
