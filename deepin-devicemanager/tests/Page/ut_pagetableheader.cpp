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
#include "../src/Page/PageInfo.h"
#include "../src/Page/PageTableHeader.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class PageTableHeader_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_pageTableHeader;
    }
    PageTableHeader *m_pageTableHeader = nullptr;
};

TEST_F(PageTableHeader_UT, PageTableHeader_UT_updateCurItemEnable)
{
    m_pageTableHeader = new PageTableHeader;
    m_pageTableHeader->updateCurItemEnable(0, true);
}

TEST_F(PageTableHeader_UT, PageTableHeader_UT_paintEvent)
{
    m_pageTableHeader = new PageTableHeader;
    QPaintEvent paint(QRect(m_pageTableHeader->rect()));
    m_pageTableHeader->paintEvent(&paint);
}

TEST_F(PageTableHeader_UT, PageTableHeader_UT_slotRefreshInfo)
{
    m_pageTableHeader = new PageTableHeader;
    m_pageTableHeader->slotRefreshInfo();
    m_pageTableHeader->slotExportInfo();
    m_pageTableHeader->slotEnableDevice(0, true);
}
