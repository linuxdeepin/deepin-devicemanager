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
#include "../src/DeviceManager/DeviceNetwork.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceNetwork_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceNetwork = new DeviceNetwork;
    }
    void TearDown()
    {
        delete m_deviceNetwork;
    }
    DeviceNetwork *m_deviceNetwork;
};

TEST_F(DeviceNetwork_UT, DeviceNetwork_UT_setInfoFromHwinfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceNetwork->setInfoFromHwinfo(mapinfo);
}

TEST_F(DeviceNetwork_UT, DeviceNetwork_UT_setInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceNetwork->setInfoFromLshw(mapinfo);
}

TEST_F(DeviceNetwork_UT, DeviceNetwork_UT_loadBaseDeviceInfo)
{
    m_deviceNetwork->loadBaseDeviceInfo();
    m_deviceNetwork->loadOtherDeviceInfo();
    m_deviceNetwork->loadTableHeader();
    m_deviceNetwork->loadTableData();
}

TEST_F(DeviceNetwork_UT, DeviceNetwork_UT_name)
{
    m_deviceNetwork->name();
    m_deviceNetwork->driver();
    m_deviceNetwork->getOverviewInfo();
}
