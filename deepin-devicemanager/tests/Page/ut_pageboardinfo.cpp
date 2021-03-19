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
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../src/Page/PageInfoWidget.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>
#include <QDebug>

class PageBoardInfo_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_pageBoardInfo = new PageBoardInfo;
    }
    void TearDown()
    {
        delete m_pageBoardInfo;
    }
    PageBoardInfo *m_pageBoardInfo;
};

TEST_F(PageBoardInfo_UT, ut_updateInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    m_pageBoardInfo->updateInfo(bInfo);
    delete device;
}

TEST_F(PageBoardInfo_UT, ut_loadDeviceInfo)
{
    DeviceBios *device = new DeviceBios;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setBiosInfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QList<QPair<QString, QString>> lst;
    lst.append(QPair<QString, QString>("/", "/"));
    m_pageBoardInfo->loadDeviceInfo(bInfo, lst);
    delete device;
}

TEST_F(PageBoardInfo_UT, ut_getOtherInfoPair)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QList<QPair<QString, QString>> lst;
    lst.append(QPair<QString, QString>("/", "/"));
    m_pageBoardInfo->getOtherInfoPair(bInfo, lst);
    delete device;
}

TEST_F(PageBoardInfo_UT, ut_getValueInfo)
{
    DeviceInput *device = new DeviceInput;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    device->setInfoFromHwinfo(mapinfo);
    QList<DeviceBaseInfo *> bInfo;
    bInfo.append(device);
    QPair<QString, QString> pair = QPair<QString, QString>("/", "/");
    m_pageBoardInfo->getValueInfo(bInfo.at(0), pair);
    delete device;
}
