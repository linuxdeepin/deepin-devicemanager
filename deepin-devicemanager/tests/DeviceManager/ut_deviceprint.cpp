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
#include "src/DeviceManager/DevicePrint.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DevicePrint_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_devicePrint;
    }
    DevicePrint *m_devicePrint;
};

TEST_F(DevicePrint_UT, DevicePrint_UT_setInfoFromUpower)
{
    m_devicePrint = new DevicePrint;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("device-uri", "http://");
    mapinfo.insert("printer-info", "Print info");
    m_devicePrint->setInfo(mapinfo);
}

TEST_F(DevicePrint_UT, DevicePrint_UT_name)
{
    m_devicePrint = new DevicePrint;
    m_devicePrint->name();
    m_devicePrint->driver();
    m_devicePrint->initFilterKey();
}

TEST_F(DevicePrint_UT, DevicePrint_UT_subTitle)
{
    m_devicePrint = new DevicePrint;
    m_devicePrint->subTitle();
}

TEST_F(DevicePrint_UT, DevicePrint_UT_loadBaseDeviceInfo)
{
    m_devicePrint = new DevicePrint;
    m_devicePrint->loadBaseDeviceInfo();
    m_devicePrint->loadOtherDeviceInfo();
    m_devicePrint->loadTableData();
    m_devicePrint->getOverviewInfo();
}
