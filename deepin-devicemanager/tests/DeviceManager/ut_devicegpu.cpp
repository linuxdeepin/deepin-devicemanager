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
#include "src/DeviceManager/DeviceGpu.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceGpu_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceGpu = new DeviceGpu;
    }
    void TearDown()
    {
        delete m_deviceGpu;
    }
    DeviceGpu *m_deviceGpu;
};

TEST_F(DeviceGpu_UT, DeviceGpu_UT_initFilterKey)
{
    m_deviceGpu->initFilterKey();
    m_deviceGpu->getOverviewInfo();
    m_deviceGpu->loadBaseDeviceInfo();
    m_deviceGpu->loadOtherDeviceInfo();
    m_deviceGpu->loadTableHeader();
    m_deviceGpu->loadTableData();
}

TEST_F(DeviceGpu_UT, DeviceGpu_UT_name)
{
    m_deviceGpu->name();
    m_deviceGpu->driver();
    m_deviceGpu->subTitle();
}

TEST_F(DeviceGpu_UT, DeviceGpu_UT_setLshwInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Vendor", "1@n");
    m_deviceGpu->setLshwInfo(mapinfo);
    m_deviceGpu->setHwinfoInfo(mapinfo);
    m_deviceGpu->setXrandrInfo(mapinfo);
    m_deviceGpu->setGpuInfo(mapinfo);
    m_deviceGpu->setDmesgInfo("4G");
}
