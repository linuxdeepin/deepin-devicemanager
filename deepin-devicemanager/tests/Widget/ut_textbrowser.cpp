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
#include "../src/Widget/TextBrowser.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../src/DeviceManager/DeviceInput.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QMap>
#include <QDomDocument>

#include <gtest/gtest.h>
class TextBrowser_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete tBrowser;
    }
    TextBrowser *tBrowser;
};

TEST_F(TextBrowser_UT, ut_showDeviceInfo)
{
    tBrowser = new TextBrowser;
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    tBrowser->showDeviceInfo(bInfo.at(0));
    delete device;
}

TEST_F(TextBrowser_UT, ut_updateInfo)
{
    tBrowser = new TextBrowser;
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    tBrowser->mp_Info = bInfo.at(0);
    tBrowser->updateInfo();
    delete device;
}

TEST_F(TextBrowser_UT, ut_setDeviceEnabled)
{
    tBrowser = new TextBrowser;
    tBrowser->mp_Info = nullptr;
    EXPECT_EQ(tBrowser->setDeviceEnabled(true), EDS_Cancle);
    tBrowser->updateShowOtherInfo();
    tBrowser->fillClipboard();
}

TEST_F(TextBrowser_UT, ut_PaintEvent)
{
    tBrowser = new TextBrowser;
    QPaintEvent paint(QRect(tBrowser->rect()));
    tBrowser->paintEvent(&paint);
}

TEST_F(TextBrowser_UT, ut_keyPressEvent)
{
    tBrowser = new TextBrowser;
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_C, Qt::ShiftModifier);
    QCoreApplication::sendEvent(tBrowser, &keyPressEvent);
}

TEST_F(TextBrowser_UT, ut_slotActionRefresh)
{
    tBrowser = new TextBrowser;
    tBrowser->slotActionRefresh();
    tBrowser->slotActionExport();
}

TEST_F(TextBrowser_UT, ut_slotActionCopy)
{
    tBrowser = new TextBrowser;
    tBrowser->slotActionCopy();
}

TEST_F(TextBrowser_UT, ut_domTitleInfo)
{
    tBrowser = new TextBrowser;
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QDomDocument doc("h3");
    tBrowser->domTitleInfo(doc, bInfo.at(0));
    delete device;
}

TEST_F(TextBrowser_UT, ut_focusInEvent)
{
    tBrowser = new TextBrowser;
    QFocusEvent focus(QFocusEvent::FocusIn);
    QCoreApplication::sendEvent(tBrowser, &focus);
    QFocusEvent focusd(QFocusEvent::FocusOut);
    QCoreApplication::sendEvent(tBrowser, &focusd);
}
