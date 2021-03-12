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
#include "../src/Page/PageBoardInfo.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../src/Page/PageDetail.h"
#include "../src/Widget/TextBrowser.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class PageDetail_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_pageDetail;
    }
    PageDetail *m_pageDetail;
};

class DetailButton_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_detailButton;
    }
    DetailButton *m_detailButton;
};

class DetailSeperator_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_detailSeperator;
    }
    DetailSeperator *m_detailSeperator;
};

class ScrollAreaWidget_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_scrollWidget;
    }
    ScrollAreaWidget *m_scrollWidget;
};

TEST_F(DetailButton_UT, DetailButton_UT_updateText)
{
    m_detailButton = new DetailButton("More");
    m_detailButton->updateText();
}

TEST_F(DetailButton_UT, DetailButton_UT_paintEvent)
{
    m_detailButton = new DetailButton("More");
    QPaintEvent paint(QRect(m_detailButton->rect()));
    m_detailButton->paintEvent(&paint);
}

TEST_F(DetailSeperator_UT, DetailSeperator_UT_paintEvent)
{
    m_detailSeperator = new DetailSeperator;
    QPaintEvent paint(QRect(m_detailSeperator->rect()));
    m_detailSeperator->paintEvent(&paint);
}

TEST_F(ScrollAreaWidget_UT, ScrollAreaWidget_UT_paintEvent)
{
    m_scrollWidget = new ScrollAreaWidget;
    QPaintEvent paint(QRect(m_scrollWidget->rect()));
    m_scrollWidget->paintEvent(&paint);
}

TEST_F(PageDetail_UT, PageDetail_UT_showDeviceInfo)
{
    m_pageDetail = new PageDetail;
    DeviceBios *device = new DeviceBios;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setBiosInfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageDetail->showDeviceInfo(bInfo);
}

TEST_F(PageDetail_UT, PageDetail_UT_showInfoOfNum)
{
    m_pageDetail = new PageDetail;
    m_pageDetail->showInfoOfNum(0);
    m_pageDetail->enableDevice(0, true);
}

TEST_F(PageDetail_UT, PageDetail_UT_paintEvent)
{
    m_pageDetail = new PageDetail;
    QPaintEvent paint(QRect(m_pageDetail->rect()));
    m_pageDetail->paintEvent(&paint);
}

TEST_F(PageDetail_UT, PageDetail_UT_resizeEvent)
{
    m_pageDetail = new PageDetail;
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_pageDetail->resizeEvent(&resizeevent);
}

TEST_F(PageDetail_UT, PageDetail_UT_addWidgets)
{
    m_pageDetail = new PageDetail;
    TextBrowser *m_tBrowser = new TextBrowser;
    m_pageDetail->addWidgets(m_tBrowser, true);
    m_pageDetail->slotExportInfo();
    m_pageDetail->slotCopyAllInfo();
    m_pageDetail->slotRefreshInfo();
    delete m_tBrowser;
}
