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
#include "../src/Page/PageOverview.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class PageOverview_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageOverview = new PageOverview;
    }
    void TearDown()
    {
        delete m_pageOverview;
    }
    PageOverview *m_pageOverview = nullptr;
};

TEST_F(PageOverview_UT, PageOverview_UT_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageOverview->updateInfo(bInfo);
    delete device;
}

TEST_F(PageOverview_UT, PageOverview_UT_updateInfo2)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_pageOverview->updateInfo(mapinfo);
    m_pageOverview->clearWidgets();
    m_pageOverview->setLabel("");
}

TEST_F(PageOverview_UT, PageOverview_UT_setLabel)
{
    m_pageOverview->setLabel("/desktop", "/#");
    m_pageOverview->setLabel("/ternimal", "/#");
    m_pageOverview->setLabel("/Tablet", "/#");
    m_pageOverview->setLabel("/laptop", "/#");
    m_pageOverview->setLabel("/server", "/#");
}

TEST_F(PageOverview_UT, PageOverview_UT_slotActionRefresh)
{
    m_pageOverview->slotActionRefresh();
    m_pageOverview->slotActionExport();
    m_pageOverview->slotActionCopy();
    m_pageOverview->isOverview();
}
