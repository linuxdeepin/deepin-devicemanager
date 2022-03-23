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
#include "PageInfo.h"
#include "PageMultiInfo.h"
#include "DeviceInput.h"
#include "PageDetail.h"
#include "ut_Head.h"
#include "stub.h"

#include <DMessageManager>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QScrollBar>

#include <gtest/gtest.h>

class PageMultiInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageMultiInfo = new PageMultiInfo;
    }
    void TearDown()
    {
        delete m_pageMultiInfo;
    }
    PageMultiInfo *m_pageMultiInfo = nullptr;
};

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageMultiInfo->updateInfo(bInfo);
    EXPECT_EQ(0,m_pageMultiInfo->mp_Detail->mp_ScrollArea->verticalScrollBar()->value());
    delete device;
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_setLabel)
{
    m_pageMultiInfo->setLabel("/");
    EXPECT_EQ("/",m_pageMultiInfo->mp_Label->text());
    EXPECT_EQ(63,m_pageMultiInfo->mp_Label->font().weight());
    m_pageMultiInfo->clearWidgets();
    EXPECT_TRUE(m_pageMultiInfo->mp_Detail->m_ListTextBrowser.isEmpty());
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotItemClicked)
{
    m_pageMultiInfo->slotItemClicked(0);
    EXPECT_EQ(0,m_pageMultiInfo->mp_Detail->mp_ScrollArea->verticalScrollBar()->value());
    m_pageMultiInfo->slotEnableDevice(0, true);
    EXPECT_FALSE(m_pageMultiInfo->isVisible());
}
