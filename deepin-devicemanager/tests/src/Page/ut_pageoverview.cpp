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
#include <QMenu>
#include <QAction>
#include <DSysInfo>

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

DSysInfo::UosEdition ut_uosEditionType()
{
    return DSysInfo::UosHome;
}

DSysInfo::UosEdition ut_uosEditionType1()
{
    return DSysInfo::UosCommunity;
}

DSysInfo::UosEdition ut_uosEditionType2()
{
    return DSysInfo::UosEducation;
}

DSysInfo::UosEdition ut_uosEditionType3()
{
    return DSysInfo::UosEnterprise;
}

DSysInfo::UosEdition ut_uosEditionType4()
{
    return DSysInfo::UosEnterpriseC;
}

DSysInfo::UosEdition ut_uosEditionType5()
{
    return DSysInfo::UosEuler;
}

DSysInfo::UosEdition ut_uosEditionType6()
{
    return DSysInfo::UosEditionCount;
}

TEST_F(PageOverview_UT, PageOverview_UT_setLabel)
{
    Stub stub;
    m_pageOverview->setLabel("/desktop", "/#");
    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType);
    m_pageOverview->setLabel("/ternimal", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType1);
    //    m_pageOverview->setLabel("/Tablet", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType2);
    //    m_pageOverview->setLabel("/laptop", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType3);
    //    m_pageOverview->setLabel("/server", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType4);
    //    m_pageOverview->setLabel("/Tablet", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType5);
    //    m_pageOverview->setLabel("/laptop", "/#");
    //    stub.set(ADDR(DSysInfo, uosEditionType), ut_uosEditionType6);
    //    m_pageOverview->setLabel("/server", "/#");
}

TEST_F(PageOverview_UT, PageOverview_UT_slotActionRefresh)
{
    m_pageOverview->slotActionCopy();
    m_pageOverview->isOverview();
}

TEST_F(PageOverview_UT, PageOverview_UT_slotShowMenu)
{
    QPoint point(0, 0);
    //    Stub stub;
    //    stub.set((QAction(QMenu::*)(const QPoint &,QAction *))ADDR(QMenu,exec),ut_exec);
    //    m_pageOverview->slotShowMenu(point);
}
