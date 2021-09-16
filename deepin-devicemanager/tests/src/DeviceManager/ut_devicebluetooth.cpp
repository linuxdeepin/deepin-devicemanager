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
#include "../src/DeviceManager/DeviceBluetooth.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceBluetooth_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceBluetooth = new DeviceBluetooth;
    }
    void TearDown()
    {
        delete m_deviceBluetooth;
    }
    DeviceBluetooth *m_deviceBluetooth;
};

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_setInfoFromHciconfig)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "1@n");
    mapInfo.insert("Vendor", "/1.0");
    mapInfo.insert("bus info", "abc");
    m_deviceBluetooth->setInfoFromHciconfig(mapInfo);
    m_deviceBluetooth->m_Vendor = "/1.0";
    ASSERT_TRUE(m_deviceBluetooth->setInfoFromHwinfo(mapInfo));
}

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_setInfoFromLshw)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "1@n");
    mapInfo.insert("Vendor", "/1.0");
    mapInfo.insert("bus info", "abc");
    m_deviceBluetooth->m_UniqueKey = "abc";
    ASSERT_TRUE(m_deviceBluetooth->setInfoFromLshw(mapInfo));
}

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_name)
{
    m_deviceBluetooth->name();
    m_deviceBluetooth->driver();
}

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_subTitle)
{
    m_deviceBluetooth->subTitle();
    m_deviceBluetooth->getOverviewInfo();
    //    m_deviceBluetooth->enable();
    //    m_deviceBluetooth->setEnable(true);
}

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_initFilterKey)
{
    m_deviceBluetooth->initFilterKey();
    m_deviceBluetooth->loadBaseDeviceInfo();
    m_deviceBluetooth->loadOtherDeviceInfo();
    m_deviceBluetooth->loadTableHeader();
    m_deviceBluetooth->loadTableData();
}

TEST_F(DeviceBluetooth_UT, DeviceBluetooth_UT_parseKeyToLshw)
{
    m_deviceBluetooth->parseKeyToLshw("http://abc");
}
