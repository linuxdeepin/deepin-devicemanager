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
#include "src/DeviceManager/DeviceCdrom.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceCdrom_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceCdrom = new DeviceCdrom;
    }
    void TearDown()
    {
        delete m_deviceCdrom;
    }
    DeviceCdrom *m_deviceCdrom;
};

TEST_F(DeviceCdrom_UT, DeviceCdrom_UT_setInfoFromLshw)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "1@n");
    mapInfo.insert("Vendor", "/1.0");
    mapInfo.insert("bus info", "ab@");
    ASSERT_TRUE(m_deviceCdrom->setInfoFromLshw(mapInfo));
}

TEST_F(DeviceCdrom_UT, DeviceCdrom_UT_setInfoFromHwinfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "1@n");
    mapInfo.insert("Vendor", "/1.0");
    mapInfo.insert("bus info", "ab@");
    m_deviceCdrom->setInfoFromHwinfo(mapInfo);
}

TEST_F(DeviceCdrom_UT, DeviceCdrom_UT_name)
{
    m_deviceCdrom->name();
    m_deviceCdrom->driver();
}

TEST_F(DeviceCdrom_UT, DeviceCdrom_UT_subTitle)
{
    m_deviceCdrom->subTitle();
    m_deviceCdrom->getOverviewInfo();
    //    m_deviceCdrom->setEnable(true);
    //    m_deviceCdrom->enable();
}

TEST_F(DeviceCdrom_UT, DeviceCdrom_UT_initFilterKey)
{
    m_deviceCdrom->initFilterKey();
    m_deviceCdrom->loadBaseDeviceInfo();
    m_deviceCdrom->loadOtherDeviceInfo();
    m_deviceCdrom->loadTableHeader();
    m_deviceCdrom->loadTableData();
}
