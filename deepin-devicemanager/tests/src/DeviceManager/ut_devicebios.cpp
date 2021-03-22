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
#include "../src/DeviceManager/DeviceBios.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceBios_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceBios = new DeviceBios;
    }
    void TearDown()
    {
        delete m_deviceBios;
    }
    DeviceBios *m_deviceBios;
};

TEST_F(DeviceBios_UT, DeviceBios_UT_setBiosInfo)
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Vendor", "1@n");
    mapInfo.insert("Version", "1.0");
    ASSERT_TRUE(m_deviceBios->setBiosInfo(mapInfo));
    ASSERT_TRUE(m_deviceBios->setBiosLanguageInfo(mapInfo));
    ASSERT_TRUE(m_deviceBios->setBaseBoardInfo(mapInfo));
    ASSERT_TRUE(m_deviceBios->setSystemInfo(mapInfo));
    ASSERT_TRUE(m_deviceBios->setChassisInfo(mapInfo));
    ASSERT_TRUE(m_deviceBios->setMemoryInfo(mapInfo));
}

TEST_F(DeviceBios_UT, DeviceBios_UT_name)
{
    m_deviceBios->name();
    m_deviceBios->vendor();
    m_deviceBios->driver();
    m_deviceBios->version();
    m_deviceBios->isBoard();
}

TEST_F(DeviceBios_UT, DeviceBios_UT_subTitle)
{
    m_deviceBios->subTitle();
    m_deviceBios->getOverviewInfo();
}

TEST_F(DeviceBios_UT, DeviceBios_UT_initFilterKey)
{
    m_deviceBios->initFilterKey();
    m_deviceBios->loadBaseDeviceInfo();
    m_deviceBios->loadOtherDeviceInfo();
    m_deviceBios->loadTableHeader();
    m_deviceBios->loadTableData();
}
