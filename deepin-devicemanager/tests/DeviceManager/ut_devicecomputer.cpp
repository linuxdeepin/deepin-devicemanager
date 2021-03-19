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
#include "src/DeviceManager/DeviceComputer.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceComputer_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceComputer = new DeviceComputer;
    }
    void TearDown()
    {
        delete m_deviceComputer;
    }
    DeviceComputer *m_deviceComputer;
};

TEST_F(DeviceComputer_UT, DeviceComputer_UT_name)
{
    m_deviceComputer->name();
    m_deviceComputer->driver();
}

TEST_F(DeviceComputer_UT, DeviceComputer_UT_setHomeUrl)
{
    m_deviceComputer->setHomeUrl("http://");
    m_deviceComputer->setOsDescription("test");
    m_deviceComputer->setOS("UOS");
    m_deviceComputer->setVendor("Intel");
    m_deviceComputer->setName("Intel");
    m_deviceComputer->setType(" ");
    m_deviceComputer->setVendor("System manufacturer", "Intel");
    m_deviceComputer->setName("System Product Name", "Intel", "font", "1.0");
}

TEST_F(DeviceComputer_UT, DeviceComputer_UT_initFilterKey)
{
    m_deviceComputer->initFilterKey();
    m_deviceComputer->getOverviewInfo();
    m_deviceComputer->loadBaseDeviceInfo();
    m_deviceComputer->loadOtherDeviceInfo();
    m_deviceComputer->loadTableHeader();
    m_deviceComputer->loadTableData();
}
