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
#include "src/DeviceManager/DeviceMemory.h"
#include "src/DeviceManager/DeviceBios.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceMemory_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_deviceMemory;
    }
    DeviceMemory *m_deviceMemory;
};

TEST_F(DeviceMemory_UT, DeviceMemory_UT_setInfoFromLshw)
{
    m_deviceMemory = new DeviceMemory;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceMemory->setInfoFromLshw(mapinfo);
}

TEST_F(DeviceMemory_UT, DeviceMemory_UT_setInfoFromDmidecode)
{
    m_deviceMemory = new DeviceMemory;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("/", "/");
    m_deviceMemory->setInfoFromDmidecode(mapinfo);
}

TEST_F(DeviceMemory_UT, DeviceMemory_UT_initFilterKey)
{
    m_deviceMemory = new DeviceMemory;
    m_deviceMemory->initFilterKey();
    m_deviceMemory->loadBaseDeviceInfo();
    m_deviceMemory->loadOtherDeviceInfo();
    m_deviceMemory->loadTableHeader();
    m_deviceMemory->loadTableData();
}

TEST_F(DeviceMemory_UT, DeviceMemory_UT_name)
{
    m_deviceMemory = new DeviceMemory;
    m_deviceMemory->name();
    m_deviceMemory->driver();
    m_deviceMemory->getOverviewInfo();
}
