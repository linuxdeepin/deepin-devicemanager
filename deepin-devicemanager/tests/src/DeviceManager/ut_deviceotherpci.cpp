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
#include "DeviceOtherPCI.h"
#include "DeviceBios.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceOtherPCI : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceOtherPCI = new DeviceOtherPCI;
    }
    void TearDown()
    {
        delete m_deviceOtherPCI;
    }
    DeviceOtherPCI *m_deviceOtherPCI;
};

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_name)
{
    QString name = m_deviceOtherPCI->name();
    EXPECT_STREQ("", name.toStdString().c_str());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_driver)
{
    QString driver = m_deviceOtherPCI->driver();
    EXPECT_STREQ("", driver.toStdString().c_str());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_subTitle)
{
    QString title = m_deviceOtherPCI->subTitle();
    EXPECT_STREQ("", title.toStdString().c_str());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_getOverviewInfo)
{
    QString overview = m_deviceOtherPCI->getOverviewInfo();
    EXPECT_STREQ("", overview.toStdString().c_str());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_initFilterKey)
{
    m_deviceOtherPCI->initFilterKey();
    EXPECT_EQ(0, m_deviceOtherPCI->m_FilterKey.size());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadBaseDeviceInfo)
{
    m_deviceOtherPCI->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceOtherPCI->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadOtherDeviceInfo)
{
    m_deviceOtherPCI->loadOtherDeviceInfo();
    EXPECT_EQ(0, m_deviceOtherPCI->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceOtherPCI, UT_DeviceOtherPCI_loadTableData)
{
    m_deviceOtherPCI->loadTableData();
    EXPECT_EQ(3, m_deviceOtherPCI->m_TableData.size());
}
