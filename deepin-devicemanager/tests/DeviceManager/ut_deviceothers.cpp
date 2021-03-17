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
#include "src/DeviceManager/DeviceOthers.h"
#include "src/DeviceManager/DeviceBios.h"
#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include <tests/stub.h>

class DeviceOthers_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_deviceOthers;
    }
    DeviceOthers *m_deviceOthers;
};

TEST_F(DeviceOthers_UT, DeviceOthers_UT_setInfoFromLshw)
{
    m_deviceOthers = new DeviceOthers;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "bus");
    m_deviceOthers->m_BusInfo = "bus";
    m_deviceOthers->setInfoFromLshw(mapinfo);
}

TEST_F(DeviceOthers_UT, DeviceOthers_UT_setInfoFromHwinfo)
{
    m_deviceOthers = new DeviceOthers;
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "st:");
    m_deviceOthers->setInfoFromHwinfo(mapinfo);
}

TEST_F(DeviceOthers_UT, DeviceOthers_UT_name)
{
    m_deviceOthers = new DeviceOthers;
    m_deviceOthers->name();
    m_deviceOthers->driver();
    m_deviceOthers->initFilterKey();
}

TEST_F(DeviceOthers_UT, DeviceOthers_UT_busInfo)
{
    m_deviceOthers = new DeviceOthers;
    m_deviceOthers->busInfo();
    m_deviceOthers->loadTableData();
    m_deviceOthers->getOverviewInfo();
}
