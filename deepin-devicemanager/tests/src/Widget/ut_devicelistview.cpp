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
#include "DeviceListView.h"
#include "ut_Head.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceListView : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceListView = new DeviceListView;
    }
    void TearDown()
    {
        delete m_deviceListView;
    }
    DeviceListView *m_deviceListView;
};

class UT_DeviceListviewDelegate : public UT_HEAD
{
public:
    void SetUp()
    {
        m_listView = new DeviceListView;
        m_deviceListViewDelegate = new DeviceListviewDelegate(m_listView);
    }
    void TearDown()
    {
        delete m_deviceListViewDelegate;
        delete m_listView;
    }
    DeviceListviewDelegate *m_deviceListViewDelegate;
    DeviceListView *m_listView;
};

TEST_F(UT_DeviceListView, UT_DeviceListviewDelegate_addItem)
{
    m_deviceListView->addItem("item", "icon##icon");
    EXPECT_STREQ(m_deviceListView->mp_ItemModel->item(0)->text().toStdString().c_str() , "item");
    m_deviceListView->setCurItem("item");
    QModelIndex index = m_deviceListView->currentIndex();
    EXPECT_EQ(0,index.row());
    EXPECT_EQ(0,index.column());
    m_deviceListView->setCurItemEnable(true);
    EXPECT_TRUE(m_deviceListView->mp_ItemModel->item(0)->isEnabled());
}

TEST_F(UT_DeviceListView, UT_DeviceListView_getConcatenateStrings)
{
    QModelIndex index;
    EXPECT_TRUE(m_deviceListView->getConcatenateStrings(index).isEmpty());
}

TEST_F(UT_DeviceListView, UT_DeviceListView_paintEvent)
{
    QPaintEvent paint(QRect(m_deviceListView->rect()));
    m_deviceListView->paintEvent(&paint);
    EXPECT_FALSE(m_deviceListView->grab().isNull());
}

TEST_F(UT_DeviceListView, UT_DeviceListView_mousePressEvent)
{
    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_deviceListView->mousePressEvent(&mousePressEvent);
}

TEST_F(UT_DeviceListView, UT_DeviceListView_mouseMoveEvent)
{
    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_deviceListView->mouseMoveEvent(&mouseMoveEvent);
    EXPECT_EQ(m_deviceListView->cursor().pos().x(),10);
    EXPECT_EQ(m_deviceListView->cursor().pos().y(),10);
}

TEST_F(UT_DeviceListView, UT_DeviceListView_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    EXPECT_TRUE(m_deviceListView->event(&keyPressEvent));
    QKeyEvent keyPressEvent1(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    EXPECT_TRUE(m_deviceListView->event(&keyPressEvent1));
}

TEST_F(UT_DeviceListviewDelegate, UT_DeviceListView_paintSeparator)
{
    QPainter painter(m_listView);
    QStyleOptionViewItem option;
    QModelIndex index = m_listView->model()->index(0, 0);
    m_deviceListViewDelegate->paintSeparator(&painter, option);
    EXPECT_FALSE(m_listView->grab().isNull());
}
