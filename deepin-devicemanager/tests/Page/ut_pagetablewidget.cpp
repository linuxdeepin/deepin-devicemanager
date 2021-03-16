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
#include "../src/Page/PageTableWidget.h"
#include "../src/Widget/DetailTreeView.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class PageTableWidget_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_pageTableWidget;
    }
    PageTableWidget *m_pageTableWidget = nullptr;
};

TEST_F(PageTableWidget_UT, PageTableWidget_UT_paintEvent)
{
    m_pageTableWidget = new PageTableWidget;
    QPaintEvent paint(QRect(m_pageTableWidget->rect()));
    m_pageTableWidget->paintEvent(&paint);
}

TEST_F(PageTableWidget_UT, PageTableWidget_UT_setCurDeviceState)
{
    m_pageTableWidget = new PageTableWidget;
    m_pageTableWidget->setCurDeviceState(false);
}

TEST_F(PageTableWidget_UT, PageTableWidget_UT_expandTable)
{
    m_pageTableWidget = new PageTableWidget;
    m_pageTableWidget->mp_Table->mp_CommandBtn = new DCommandLinkButton("");
    m_pageTableWidget->mp_Table->m_IsExpand = true;
    m_pageTableWidget->expandTable();
}
