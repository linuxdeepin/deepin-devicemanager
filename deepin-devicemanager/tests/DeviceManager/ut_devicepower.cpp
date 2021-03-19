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
#include "src/DeviceManager/DevicePower.h"
#include "src/DeviceManager/DeviceBios.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DevicePower_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_DevicePower = new DevicePower;
    }
    void TearDown()
    {
        delete m_DevicePower;
    }
    DevicePower *m_DevicePower;
};

TEST_F(DevicePower_UT, DevicePower_UT_setInfoFromUpower)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device", "/");
    mapinfo.insert("temperature", "12");
    m_DevicePower->setInfoFromUpower(mapinfo);
}

TEST_F(DevicePower_UT, DevicePower_UT_setDaemonInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "st:");
    m_DevicePower->setDaemonInfo(mapinfo);
}

TEST_F(DevicePower_UT, DevicePower_UT_name)
{
    m_DevicePower->name();
    m_DevicePower->driver();
    m_DevicePower->initFilterKey();
}

TEST_F(DevicePower_UT, DevicePower_UT_loadBaseDeviceInfo)
{
    m_DevicePower->loadBaseDeviceInfo();
    m_DevicePower->loadOtherDeviceInfo();
    m_DevicePower->loadTableData();
    m_DevicePower->getOverviewInfo();
}
