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
#include "../src/Page/PageSingleInfo.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class PageSingleInfo_UT : public UT_HEAD
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

TEST_F(PageSingleInfo_UT, PageSingleInfo_UT_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_PageSingleInfo->updateInfo(bInfo);
    delete device;
}

TEST_F(PageSingleInfo_UT, PageSingleInfo_UT_setLabel)
{
    m_PageSingleInfo->setLabel("/");
    m_PageSingleInfo->clearWidgets();
}

TEST_F(PageSingleInfo_UT, PageSingleInfo_UT_slotActionRefresh)
{
    m_PageSingleInfo->slotActionCopy();
    m_PageSingleInfo->slotShowMenu(QPoint(0, 0));
}

TEST_F(PageSingleInfo_UT, PageSingleInfo_UT_slotActionEnable)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    m_PageSingleInfo->mp_Device = device;
    m_PageSingleInfo->slotActionEnable();
    delete device;
}
