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
#include "TextBrowser.h"
#include "DeviceInfo.h"
#include "DeviceInput.h"
#include "stub.h"
#include "ut_Head.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QMap>
#include <QDomDocument>
#include <QMenu>
#include <QClipboard>
#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_TextBrowser : public UT_HEAD
{
public:
    void SetUp()
    {
        tBrowser = new TextBrowser;
    }
    void TearDown()
    {
        delete tBrowser;
    }
    TextBrowser *tBrowser;
};

TEST_F(UT_TextBrowser, UT_TextBrowser_showDeviceInfo)
{
//    DeviceInput *device = new DeviceInput;
//    device->m_Name = "mouse";
//    device->m_Interface = "USB";
//    QList<DeviceBaseInfo *> bInfo;
//    bInfo.append(device);
//    tBrowser->showDeviceInfo(bInfo.at(0));
//    EXPECT_STREQ("mouse\nName:\nmouse\nInterface:\nUSB\n",tBrowser->toPlainText().toStdString().c_str());
//    delete device;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_updateInfo)
{
//    DeviceInput *device = new DeviceInput;
//    device->m_Name = "keyboard";
//    device->m_Interface = "USB";
//    QList<DeviceBaseInfo *> bInfo;
//    bInfo.append(device);
//    tBrowser->mp_Info = bInfo.at(0);
//    tBrowser->m_ShowOtherInfo = true;
//    tBrowser->updateInfo();
//    EXPECT_STREQ("keyboard\nName:\nkeyboard\nInterface:\nUSB\n",tBrowser->toPlainText().toStdString().c_str());
//    delete device;
}

QString ut_selectedText()
{
    return "/";
}

TEST_F(UT_TextBrowser, UT_TextBrowser_setDeviceEnabled)
{
    tBrowser->updateShowOtherInfo();
    Stub stub;
    stub.set(ADDR(QTextCursor, selectedText), ut_selectedText);
    tBrowser->fillClipboard();
    tBrowser->mp_Info = nullptr;
    EXPECT_EQ(tBrowser->setDeviceEnabled(true), EDS_Cancle);
}

TEST_F(UT_TextBrowser, UT_TextBrowser_PaintEvent)
{
    QPaintEvent paint(QRect(tBrowser->rect()));
    tBrowser->paintEvent(&paint);
    EXPECT_FALSE(tBrowser->grab().isNull());
}

TEST_F(UT_TextBrowser, UT_TextBrowser_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_C, Qt::ShiftModifier);
    EXPECT_TRUE(tBrowser->event(&keyPressEvent));
}

TEST_F(UT_TextBrowser, UT_TextBrowser_slotActionRefresh)
{
    QSignalSpy spy1(tBrowser, SIGNAL(refreshInfo()));
    ASSERT_EQ(0, spy1.count());
    tBrowser->slotActionRefresh();
    ASSERT_EQ(1, spy1.count());

    QSignalSpy spy2(tBrowser, SIGNAL(exportInfo()));
    ASSERT_EQ(0, spy2.count());
    tBrowser->slotActionExport();
    ASSERT_EQ(1, spy2.count());
}

TEST_F(UT_TextBrowser, UT_TextBrowser_slotActionCopy)
{
    Stub stub;
    stub.set(ADDR(QTextCursor, selectedText), ut_selectedText);
    tBrowser->slotActionCopy();
    QClipboard *clipboard = DApplication::clipboard();
    EXPECT_STREQ("/", clipboard->text().toStdString().c_str());
}

bool ut_text_enable()
{
    return false;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_domTitleInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "keyboard";
    device->m_Interface = "USB";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QDomDocument doc("h3");
    tBrowser->domTitleInfo(doc, bInfo.at(0));
    EXPECT_FALSE(doc.isNull());
    delete device;
}

bool ut_rect_contains()
{
    return false;
}

TEST_F(UT_TextBrowser, UT_TextBrowser_focusInEvent)
{
    tBrowser->m_ShowOtherInfo = true;
    QFocusEvent focus(QFocusEvent::FocusIn);
    QCoreApplication::sendEvent(tBrowser, &focus);

    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contains);
    DeviceInput *device = new DeviceInput;
    device->m_Name = "keyboard";
    device->m_Interface = "USB";

    tBrowser->mp_Info = dynamic_cast<DeviceBaseInfo *>(device);
    QFocusEvent focusd(QFocusEvent::FocusOut);
    EXPECT_TRUE(tBrowser->event(&focusd));
    delete device;
}
