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
#include "../src/Page/PageMultiInfo.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class PageMultiInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageMultiInfo = new PageMultiInfo;
    }
    void TearDown()
    {
        delete m_pageMultiInfo;
    }
    PageMultiInfo *m_pageMultiInfo = nullptr;
};

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageMultiInfo->updateInfo(bInfo);
    delete device;
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_setLabel)
{
    m_pageMultiInfo->setLabel("/");
    m_pageMultiInfo->clearWidgets();
}

TEST_F(PageMultiInfo_UT, PageMultiInfo_UT_slotItemClicked)
{
    m_pageMultiInfo->slotItemClicked(0);
    m_pageMultiInfo->slotRefreshInfo();
    m_pageMultiInfo->slotExportInfo();
    m_pageMultiInfo->slotEnableDevice(0, true);
}
