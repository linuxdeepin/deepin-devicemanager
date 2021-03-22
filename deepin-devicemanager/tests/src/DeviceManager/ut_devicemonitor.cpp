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
#include "../src/DeviceManager/DeviceMonitor.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceMonitor_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceMonitor = new DeviceMonitor;
    }
    void TearDown()
    {
        delete m_deviceMonitor;
    }
    DeviceMonitor *m_deviceMonitor;
};

bool ut_exacMatch()
{
    return true;
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_parseMonitorSize)
{
    double inch = 32.00;
    QSize size = QSize(100, 100);
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_exacMatch);
    m_deviceMonitor->parseMonitorSize("test", inch, size);
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setInfoFromHwinfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceMonitor->setInfoFromHwinfo(mapinfo);
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setInfoFromSelfDefine)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceMonitor->setInfoFromSelfDefine(mapinfo);
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setInfoFromEdid)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceMonitor->setInfoFromEdid(mapinfo);
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setMainInfoFromXrandr)
{
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_exacMatch);
    m_deviceMonitor->setMainInfoFromXrandr(".*([0-9]{3,5})mm\\sx\\s([0-9]{3,5})mm");
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_initFilterKey)
{
    m_deviceMonitor->initFilterKey();
    m_deviceMonitor->loadBaseDeviceInfo();
    m_deviceMonitor->loadOtherDeviceInfo();
    m_deviceMonitor->loadTableHeader();
    m_deviceMonitor->loadTableData();
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_name)
{
    m_deviceMonitor->name();
    m_deviceMonitor->driver();
    m_deviceMonitor->getOverviewInfo();
}

int ut_gcd()
{
    return 2;
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_caculateScreenRatio)
{
    m_deviceMonitor->gcd(2, 2);
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_exacMatch);
    stub.set(ADDR(DeviceMonitor, gcd), ut_gcd);
    m_deviceMonitor->caculateScreenRatio();
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_findAspectRatio)
{
    int arw = 5;
    int arh = 5;
    ASSERT_TRUE(m_deviceMonitor->findAspectRatio(10, 5, arw, arh));
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_caculateScreenSize)
{
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_exacMatch);
    m_deviceMonitor->caculateScreenSize();
    m_deviceMonitor->caculateScreenSize("abc\n");
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setInfoFromXradr)
{
    m_deviceMonitor->setInfoFromXradr("connect", "/n");
}

TEST_F(DeviceMonitor_UT, DeviceMonitor_UT_setCurrentResolution)
{
    m_deviceMonitor->setCurrentResolution("connect", "/n");
}
