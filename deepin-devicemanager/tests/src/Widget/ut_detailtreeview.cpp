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
#include "../src/Widget/DetailTreeView.h"
#include "../src/Page/PageTableWidget.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DetailTreeView_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_dTreeView = new DetailTreeView;
    }
    void TearDown()
    {
        delete m_dTreeView;
    }
    DetailTreeView *m_dTreeView;
};

class BtnWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_bWidget = new BtnWidget;
    }
    void TearDown()
    {
        delete m_bWidget;
    }
    BtnWidget *m_bWidget;
};

TEST_F(BtnWidget_UT, ut_enterEvent)
{
    QEnterEvent event(QPointF(0, 0), QPointF(10, 10), QPointF(20, 20));
    QCoreApplication::sendEvent(m_bWidget, &event);
}

TEST_F(BtnWidget_UT, ut_leaveEvent)
{
    QEvent event(QEvent::Leave);
    QCoreApplication::sendEvent(m_bWidget, &event);
}

bool ut_isBaseBoard()
{
    return true;
}

TEST_F(DetailTreeView_UT, ut_setItem)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->setItem(0, 0, item);
    m_dTreeView->clear();
    delete item;
}

TEST_F(DetailTreeView_UT, ut_setCommanLinkButton)
{
    m_dTreeView->setCommanLinkButton(0);
}

TEST_F(DetailTreeView_UT, ut_hasExpendInfo)
{
    ASSERT_FALSE(m_dTreeView->hasExpendInfo());
}

TEST_F(DetailTreeView_UT, ut_toString)
{
    m_dTreeView->toString();
    ASSERT_TRUE(m_dTreeView->isCurDeviceEnable());
}

TEST_F(DetailTreeView_UT, ut_setCurDeviceState)
{
    m_dTreeView->setCurDeviceState(false);
}

TEST_F(DetailTreeView_UT, ut_expandCommandLinkClicked)
{
    m_dTreeView->m_IsExpand = true;
    m_dTreeView->mp_CommandBtn = new DCommandLinkButton("");
    m_dTreeView->expandCommandLinkClicked();
    m_dTreeView->m_IsExpand = false;
    m_dTreeView->expandCommandLinkClicked();
    delete m_dTreeView->mp_CommandBtn;
}

int ut_setTableHeight()
{
    return 10;
}

bool ut_hasExpendInfo()
{
    return true;
}

TEST_F(DetailTreeView_UT, ut_paintEvent)
{
    QPaintEvent paint(QRect(m_dTreeView->rect()));
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), ut_setTableHeight);
    stub.set(ADDR(DetailTreeView, hasExpendInfo), ut_hasExpendInfo);

    m_dTreeView->m_IsExpand = false;
    m_dTreeView->paintEvent(&paint);
    m_dTreeView->m_IsExpand = true;
    m_dTreeView->paintEvent(&paint);
}

TEST_F(DetailTreeView_UT, ut_leaveEvent)
{
    QEvent event(QEvent::Leave);
    QCoreApplication::sendEvent(m_dTreeView, &event);
}

TEST_F(DetailTreeView_UT, ut_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_dTreeView->resizeEvent(&resizeevent);
}

TEST_F(DetailTreeView_UT, UT_mouseMoveEvent)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(0, 0), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_dTreeView->mouseMoveEvent(&moveEvent);
}

TEST_F(DetailTreeView_UT, ut_slotTimeOut)
{
    m_dTreeView->slotTimeOut();
}

TEST_F(DetailTreeView_UT, ut_slotItemEnterd)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->slotItemEnterd(item);
    m_dTreeView->slotEnterBtnWidget();
    m_dTreeView->slotLeaveBtnWidget();
    delete item;
}

TEST_F(DetailTreeView_UT, ut_showTips)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->showTips(item);
    delete item;
}
