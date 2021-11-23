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
#include "PageListView.h"
#include "DeviceListView.h"

#include "DeviceInput.h"
#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "stub.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class PageListView_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageListView = new PageListView;
    }
    void TearDown()
    {
        delete m_pageListView;
    }
    PageListView *m_pageListView = nullptr;
};

TEST_F(PageListView_UT, PageListView_UT_updateListItems)
{
    QList<QPair<QString, QString>> list;
    list.append(QPair<QString, QString>("/", "/"));
    m_pageListView->updateListItems(list);
    EXPECT_EQ(0,m_pageListView->mp_ListView->mp_ItemModel->rowCount());
}

TEST_F(PageListView_UT, PageListView_UT_currentIndex)
{
    ASSERT_EQ(m_pageListView->currentIndex(), "");
    ASSERT_NE(m_pageListView->currentType(), "");
}

TEST_F(PageListView_UT, PageListView_UT_paintEvent)
{
    QPaintEvent paint(QRect(m_pageListView->rect()));
    m_pageListView->paintEvent(&paint);
}

QString ut_getConcatenateStrings()
{
    return "/";
}

TEST_F(PageListView_UT, PageListView_UT_slotListViewItemClicked)
{
    Stub stub;
    stub.set(ADDR(DeviceListView, getConcatenateStrings), ut_getConcatenateStrings);
    QModelIndex index;
    m_pageListView->slotListViewItemClicked(index);
    EXPECT_EQ("/",m_pageListView->m_CurType);
}
