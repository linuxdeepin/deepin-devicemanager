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
#include "../src/Widget/DeviceListView.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class DeviceListView_UT : public UT_HEAD
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

class DeviceListviewDelegate_UT : public UT_HEAD
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

TEST_F(DeviceListView_UT, ut_addItem)
{
    m_deviceListView->addItem("/", "/###");
    m_deviceListView->setCurItem("/");
    m_deviceListView->setCurItemEnable(true);
    m_deviceListView->setCurItem("/");
}

TEST_F(DeviceListView_UT, ut_getConcatenateStrings)
{
    QModelIndex index;
    m_deviceListView->getConcatenateStrings(index);
}

TEST_F(DeviceListView_UT, ut_paintEvent)
{
    QPaintEvent paint(QRect(m_deviceListView->rect()));
    m_deviceListView->paintEvent(&paint);
}

TEST_F(DeviceListView_UT, UT_mousePressEvent)
{
    QMouseEvent mousePressEvent(QEvent::MouseButtonPress, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_deviceListView->mousePressEvent(&mousePressEvent);
}

TEST_F(DeviceListView_UT, UT_mouseMoveEvent)
{
    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_deviceListView->mouseMoveEvent(&mouseMoveEvent);
}

TEST_F(DeviceListView_UT, UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_deviceListView, &keyPressEvent);
    QKeyEvent keyPressEvent1(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QCoreApplication::sendEvent(m_deviceListView, &keyPressEvent1);
}

TEST_F(DeviceListviewDelegate_UT, ut_paintSeparator)
{
    QPainter painter(m_listView);
    QStyleOptionViewItem option;
    QModelIndex index = m_listView->model()->index(0, 0);
    m_deviceListViewDelegate->paintSeparator(&painter, option);
}
