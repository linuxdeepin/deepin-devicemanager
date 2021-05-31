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
#include "../src/DeviceManager/DeviceStorage.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceStorage_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceStorage = new DeviceStorage;
    }
    void TearDown()
    {
        delete m_deviceStorage;
    }
    DeviceStorage *m_deviceStorage;
};

bool ut_DeviceStorage_exacMatch()
{
    return true;
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_setHwinfoInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "123");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_SerialNumber = "Test123";
    m_deviceStorage->m_KeyToLshw = "nvme0";
    m_deviceStorage->m_Model = "ST";
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_DeviceStorage_exacMatch);
    ASSERT_TRUE(m_deviceStorage->setHwinfoInfo(mapinfo));
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_setKLUHwinfoInfo)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("SysFS BusID", "123");
    mapinfo.insert("Driver", "usb-storage");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_SerialNumber = "Test123";
    Stub stub;
    stub.set(ADDR(QRegExp, exactMatch), ut_DeviceStorage_exacMatch);
    ASSERT_TRUE(m_deviceStorage->setKLUHwinfoInfo(mapinfo));
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_addInfoFromlshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1:2@34");
    mapinfo.insert("Driver", "usb-storage");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_KeyToLshw = "34";
    ASSERT_TRUE(m_deviceStorage->addInfoFromlshw(mapinfo));
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_addNVMEInfoFromlshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1:2@34");
    mapinfo.insert("Driver", "usb-storage");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_KeyToLshw = "34";
    m_deviceStorage->addNVMEInfoFromlshw(mapinfo);
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_setMediaType)
{
    m_deviceStorage->m_DeviceFile = "/test";
    ASSERT_TRUE(m_deviceStorage->setMediaType("/test", "0"));
    ASSERT_TRUE(m_deviceStorage->setMediaType("/test", "1"));
    ASSERT_TRUE(m_deviceStorage->setMediaType("/test", "2R"));
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_setKLUMediaType)
{
    m_deviceStorage->m_DeviceFile = "/test";
    ASSERT_TRUE(m_deviceStorage->setKLUMediaType("/test", "0"));
    ASSERT_TRUE(m_deviceStorage->setKLUMediaType("/test", "1"));
    ASSERT_TRUE(m_deviceStorage->setKLUMediaType("/test", "2R"));
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_isValid)
{
    m_deviceStorage->m_Size = "32";
    ASSERT_TRUE(m_deviceStorage->isValid());
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_setDiskSerialID)
{
    m_deviceStorage->m_SerialNumber = "32";
    m_deviceStorage->setDiskSerialID("by-id,test");
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_compareSize)
{
    ASSERT_EQ(m_deviceStorage->compareSize("16", "32"), "32");
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_name)
{
    m_deviceStorage->name();
    m_deviceStorage->driver();
    m_deviceStorage->initFilterKey();
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_subTitle)
{
    m_deviceStorage->subTitle();
    m_deviceStorage->getOverviewInfo();
    m_deviceStorage->keyFromStorage();
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_loadTableHeader)
{
    m_deviceStorage->loadTableHeader();
    m_deviceStorage->loadTableData();
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_loadBaseDeviceInfo)
{
    m_deviceStorage->loadBaseDeviceInfo();
    m_deviceStorage->loadOtherDeviceInfo();
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_getInfoFromLshw)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("bus info", "1:2@34");
    mapinfo.insert("Driver", "usb-storage");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_KeyToLshw = "34";
    m_deviceStorage->getInfoFromLshw(mapinfo);
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_getInfoFromsmartctl)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device Model", "1:2@34");
    mapinfo.insert("User Capacity", "1");
    mapinfo.insert("Model Number", "test");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_KeyToLshw = "34";
    m_deviceStorage->getInfoFromsmartctl(mapinfo);
}

TEST_F(DeviceStorage_UT, DeviceStorage_UT_addInfoFromSmartctl)
{
    QMap<QString, QString> mapinfo;
    mapinfo.insert("Device Model", "1:2@34");
    mapinfo.insert("User Capacity", "1");
    mapinfo.insert("Model Number", "test");
    m_deviceStorage->m_Size = "16";
    m_deviceStorage->m_KeyToLshw = "34";
    m_deviceStorage->m_DeviceFile = "name";
    m_deviceStorage->addInfoFromSmartctl("name", mapinfo);
}
