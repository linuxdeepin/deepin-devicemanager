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
#include "../src/Page/PageMultiInfo.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class PageInfo_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
    }
    PageInfo *m_pageInfo = nullptr;
};

TEST_F(PageInfo_UT, PageInfo_UT_getDeviceInfoNum)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    m_pageInfo->getDeviceInfoNum();
    QMap<QString, QString> map;
    map.insert("/", "/");
    m_pageInfo->updateInfo(map);
    m_pageInfo->setLabel("", "");
    m_pageInfo->clearContent();
    delete p;
}

TEST_F(PageInfo_UT, PageInfo_UT_paintEvent)
{
    PageMultiInfo *p = new PageMultiInfo;
    m_pageInfo = dynamic_cast<PageInfo *>(p);
    QPaintEvent paint(QRect(m_pageInfo->rect()));
    m_pageInfo->paintEvent(&paint);
    delete p;
}
