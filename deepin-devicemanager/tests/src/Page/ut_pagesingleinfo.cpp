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
#include "PageInfo.h"
#include "PageSingleInfo.h"
#include "PageTableWidget.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "ut_Head.h"
#include "stub.h"

#include <DApplication>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QClipboard>
#include <QMenu>

#include <gtest/gtest.h>

class UT_PageSingleInfo : public UT_HEAD
{
public:
    void SetUp()
    {
        m_PageSingleInfo = new PageSingleInfo;
    }
    void TearDown()
    {
        delete m_PageSingleInfo;
    }
    PageSingleInfo *m_PageSingleInfo = nullptr;
};

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_PageSingleInfo->updateInfo(bInfo);
    DeviceInput *d = dynamic_cast<DeviceInput *>(m_PageSingleInfo->mp_Device);
    EXPECT_STREQ(d->m_Name.toStdString().c_str(),"name");
    EXPECT_STREQ(d->m_Vendor.toStdString().c_str(),"vendor");
    EXPECT_STREQ(d->m_Model.toStdString().c_str(),"model");
    delete device;
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_setLabel)
{
    m_PageSingleInfo->setLabel("test");
    EXPECT_STREQ("test",m_PageSingleInfo->mp_Label->text().toStdString().c_str());
    m_PageSingleInfo->clearWidgets();
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionRefresh)
{
    QString conStr=m_PageSingleInfo->mp_Content->toString();
    m_PageSingleInfo->slotActionCopy();
    QClipboard *clipboard = DApplication::clipboard();
    QString clipStr = clipboard->text();
    EXPECT_STREQ(conStr.toStdString().c_str(),clipStr.toStdString().c_str());

    m_PageSingleInfo->slotShowMenu(QPoint(0, 0));
    EXPECT_EQ(m_PageSingleInfo->mp_Menu->actions().size(),1);
}

TEST_F(UT_PageSingleInfo, UT_PageSingleInfo_slotActionEnable)
{
    DeviceInput *device = new DeviceInput;
    device->m_Name = "name";
    device->m_Vendor = "vendor";
    device->m_Model = "model";
    m_PageSingleInfo->mp_Device = device;
    m_PageSingleInfo->slotActionEnable();
    EXPECT_TRUE(m_PageSingleInfo->mp_Device->enable());
    delete device;
}
