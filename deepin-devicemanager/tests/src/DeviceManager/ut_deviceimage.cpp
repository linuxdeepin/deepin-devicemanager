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
#include "DeviceImage.h"

#include "stub.h"
#include "ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class UT_DeviceImage : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceImage = new DeviceImage;
    }
    void TearDown()
    {
        delete m_deviceImage;
    }
    DeviceImage *m_deviceImage;
};

void ut_image_setlshwmap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("bus info", "usb@1:8");
    mapinfo.insert("product", "product");
    mapinfo.insert("vendor", "vendor");
    mapinfo.insert("version", "version");
    mapinfo.insert("capabilities", "capabilities");
    mapinfo.insert("driver", "driver");
    mapinfo.insert("maxpower", "maxpower");
    mapinfo.insert("speed", "speed");
}

void ut_image_sethwinfomap(QMap<QString, QString> &mapinfo)
{
    mapinfo.insert("Device", "Device");
    mapinfo.insert("Vendor", "Vendor");
    mapinfo.insert("Model", "Model");
    mapinfo.insert("Revision", "Revision");
    mapinfo.insert("SysFS BusID", "1-8:1.0");
    mapinfo.insert("Driver", "Driver");
    mapinfo.insert("Driver Modules", "Driver Modules");
    mapinfo.insert("Speed", "Speed");
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromLshw_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_setlshwmap(mapinfo);
    m_deviceImage->m_KeyToLshw = "usb@1:8";

    m_deviceImage->setInfoFromLshw(mapinfo);
    EXPECT_STREQ("product", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("vendor", m_deviceImage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("version", m_deviceImage->m_Version.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceImage->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("capabilities", m_deviceImage->m_Capabilities.toStdString().c_str());
    EXPECT_STREQ("uvcvideo", m_deviceImage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("maxpower", m_deviceImage->m_MaximumPower.toStdString().c_str());
    EXPECT_STREQ("speed", m_deviceImage->m_Speed.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromLshw_002)
{
    QMap<QString, QString> mapinfo;
    ut_image_setlshwmap(mapinfo);
    m_deviceImage->m_KeyToLshw = "usb@1:9";

    m_deviceImage->setInfoFromLshw(mapinfo);
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setInfoFromHwinfo)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);

    m_deviceImage->setInfoFromHwinfo(mapinfo);
    EXPECT_STREQ("Device", m_deviceImage->m_Name.toStdString().c_str());
    EXPECT_STREQ("Vendor", m_deviceImage->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("Model", m_deviceImage->m_Model.toStdString().c_str());
    EXPECT_STREQ("Revision", m_deviceImage->m_Version.toStdString().c_str());
    EXPECT_STREQ("1-8:1.0", m_deviceImage->m_BusInfo.toStdString().c_str());
    EXPECT_STREQ("Driver Modules", m_deviceImage->m_Driver.toStdString().c_str());
    EXPECT_STREQ("Speed", m_deviceImage->m_Speed.toStdString().c_str());
    EXPECT_STREQ("usb@1:8", m_deviceImage->m_KeyToLshw.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_name)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString name = m_deviceImage->name();
    EXPECT_STREQ("Device", name.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_driver)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString driver = m_deviceImage->driver();
    EXPECT_STREQ("Driver Modules", driver.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_subTitle)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString title = m_deviceImage->subTitle();
    EXPECT_STREQ("Device", title.toStdString().c_str());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_getOverviewInfo)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    QString overview = m_deviceImage->getOverviewInfo();
    EXPECT_STREQ("Device (Model)", overview.toStdString().c_str());
}

EnableDeviceStatus ut_image_enableDeviceByDriver()
{
    return EnableDeviceStatus::EDS_Faild;
}

bool ut_image_isenablebydriver_true()
{
    return true;
}

bool ut_image_isenablebydriver_false()
{
    return false;
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setEnable_001)
{
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_deviceImage->setEnable(true));
}

TEST_F(UT_DeviceImage, UT_DeviceImage_setEnable_002)
{
    EXPECT_EQ(EnableDeviceStatus::EDS_Faild, m_deviceImage->setEnable(false));
}

TEST_F(UT_DeviceImage, UT_DeviceImage_enable)
{
    EXPECT_TRUE(m_deviceImage->enable());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_initFilterKey)
{
    m_deviceImage->initFilterKey();
    EXPECT_EQ(0, m_deviceImage->m_FilterKey.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadBaseDeviceInfo)
{
    m_deviceImage->loadBaseDeviceInfo();
    EXPECT_EQ(0, m_deviceImage->m_LstBaseInfo.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadOtherDeviceInfo)
{
    m_deviceImage->loadOtherDeviceInfo();
    EXPECT_EQ(1, m_deviceImage->m_LstOtherInfo.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadTableData_001)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    m_deviceImage->loadTableData();
    EXPECT_EQ(3, m_deviceImage->m_TableData.size());
}

TEST_F(UT_DeviceImage, UT_DeviceImage_loadTableData_002)
{
    QMap<QString, QString> mapinfo;
    ut_image_sethwinfomap(mapinfo);
    m_deviceImage->setInfoFromHwinfo(mapinfo);

    m_deviceImage->loadTableData();
    EXPECT_EQ(3, m_deviceImage->m_TableData.size());
}
