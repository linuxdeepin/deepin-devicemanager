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
#include "DetailTreeView.h"
#include "PageTableWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DetailTreeView : public UT_HEAD
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

class UT_BtnWidget : public UT_HEAD
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

TEST_F(UT_BtnWidget, UT_BtnWidget_enterEvent)
{
    QEnterEvent event(QPointF(0, 0), QPointF(9, 9), QPointF(18, 18));
    EXPECT_TRUE(m_bWidget->event(&event));
}

TEST_F(UT_BtnWidget, UT_BtnWidget_leaveEvent)
{
    QEvent event(QEvent::Leave);
    EXPECT_TRUE(m_bWidget->event(&event));
}

bool ut_isBaseBoard()
{
    return false;
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_setItem)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->insertRow(0);
    m_dTreeView->insertColumn(0);
    m_dTreeView->setItem(0, 0, item);
    EXPECT_TRUE(m_dTreeView->itemAt(0,0) != nullptr);

    delete item;
    m_dTreeView->clear();
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_setCommanLinkButton)
{
    m_dTreeView->setCommanLinkButton(0);
    EXPECT_TRUE(m_dTreeView->mp_CommandBtn);
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_hasExpendInfo)
{
    ASSERT_FALSE(m_dTreeView->hasExpendInfo());
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_toString)
{
    QTableWidgetItem *item = new QTableWidgetItem("item");
    m_dTreeView->insertRow(0);
    m_dTreeView->insertColumn(0);
    m_dTreeView->setItem(0, 0, item);
    EXPECT_STREQ("item : ",m_dTreeView->toString().toStdString().c_str());

    delete item;
    m_dTreeView->clear();
}

bool ut_hasExpendInfo()
{
    return true;
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_setCurDeviceState)
{
//    m_dTreeView->m_IsExpand = true;
//    Stub stub;
//    stub.set(ADDR(DetailTreeView, hasExpendInfo), ut_hasExpendInfo);
//    m_dTreeView->setCurDeviceState(false);
//    EXPECT_FALSE(m_dTreeView->m_IsEnable);
//    m_dTreeView->setCurDeviceState(true);
//    EXPECT_TRUE(m_dTreeView->m_IsEnable);
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_expandCommandLinkClicked)
{
    m_dTreeView->m_IsExpand = true;
    m_dTreeView->mp_CommandBtn = new DCommandLinkButton("");
    m_dTreeView->expandCommandLinkClicked();
    EXPECT_STREQ("More",m_dTreeView->mp_CommandBtn->text().toStdString().c_str());
    m_dTreeView->m_IsExpand = false;
    m_dTreeView->expandCommandLinkClicked();
    EXPECT_STREQ("Collapse",m_dTreeView->mp_CommandBtn->text().toStdString().c_str());
    delete m_dTreeView->mp_CommandBtn;
}

int ut_setTableHeight()
{
    return 10;
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_paintEvent)
{
    QPaintEvent paint(QRect(m_dTreeView->rect()));
    Stub stub;
    stub.set(ADDR(DetailTreeView, setTableHeight), ut_setTableHeight);
    stub.set(ADDR(DetailTreeView, hasExpendInfo), ut_hasExpendInfo);

    m_dTreeView->m_IsExpand = false;
    m_dTreeView->paintEvent(&paint);
    EXPECT_FALSE(m_dTreeView->grab().isNull());
    m_dTreeView->m_IsExpand = true;
    m_dTreeView->paintEvent(&paint);
    EXPECT_FALSE(m_dTreeView->grab().isNull());
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_leaveEvent)
{
    QEvent event(QEvent::Leave);
    EXPECT_TRUE(m_dTreeView->event(&event));
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_dTreeView->resizeEvent(&resizeevent);
    EXPECT_FALSE(m_dTreeView->mp_CurItem);
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_mouseMoveEvent)
{
    QMouseEvent moveEvent(QEvent::MouseMove, QPoint(1, 1), QPoint(10, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_dTreeView->mouseMoveEvent(&moveEvent);
    EXPECT_EQ(m_dTreeView->mp_Point.x(),1);
    EXPECT_EQ(m_dTreeView->mp_Point.y(),1);
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_slotTimeOut)
{
    m_dTreeView->slotTimeOut();
    EXPECT_FALSE(m_dTreeView->mp_CurItem);
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_slotItemEnterd)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->slotItemEnterd(item);
    EXPECT_EQ(item,m_dTreeView->mp_CurItem);
    m_dTreeView->slotEnterBtnWidget();
    EXPECT_EQ(nullptr,m_dTreeView->mp_CurItem);
    m_dTreeView->slotLeaveBtnWidget();
    EXPECT_EQ(nullptr,m_dTreeView->mp_CurItem);
    delete item;
}

TEST_F(UT_DetailTreeView, UT_DetailTreeView_showTips)
{
    QTableWidgetItem *item = new QTableWidgetItem;
    m_dTreeView->showTips(item);
    EXPECT_TRUE(m_dTreeView->mp_ToolTips);
    delete item;
}
