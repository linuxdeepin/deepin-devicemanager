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
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_PageTableWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageTableWidget = new PageTableWidget;
    }
    void TearDown()
    {
        delete m_pageTableWidget;
    }
    PageTableWidget *m_pageTableWidget = nullptr;
};

int ut_setTableHeight01()
{
    return 10;
}

TEST_F(UT_PageTableWidget, UT_PageTableWidget_setCurDeviceState)
{
    m_pageTableWidget->setCurDeviceState(false);
    EXPECT_FALSE(m_pageTableWidget->mp_Table->m_IsEnable);
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), ut_setTableHeight01);
    m_pageTableWidget->setCurDeviceState(true);
    EXPECT_TRUE(m_pageTableWidget->mp_Table->m_IsEnable);
}

TEST_F(UT_PageTableWidget, UT_PageTableWidget_expandTable)
{
    m_pageTableWidget->mp_Table->mp_CommandBtn = new DCommandLinkButton("");
    m_pageTableWidget->mp_Table->m_IsExpand = true;
    m_pageTableWidget->expandTable();
    EXPECT_STREQ("More", m_pageTableWidget->mp_Table->mp_CommandBtn->text().toStdString().c_str());
    delete m_pageTableWidget->mp_Table->mp_CommandBtn;
}
