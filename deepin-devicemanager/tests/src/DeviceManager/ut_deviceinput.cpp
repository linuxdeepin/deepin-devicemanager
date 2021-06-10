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
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceBios.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class DeviceInput_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_deviceInput = new DeviceInput;
    }
    void TearDown()
    {
        delete m_deviceInput;
    }
    DeviceInput *m_deviceInput;
};

TEST_F(DeviceInput_UT, DeviceInput_UT_setInfoFromHwinfo)
{
    QMap<QString, QString> map;
    map.insert("Hotplug", "abc");
    map.insert("Model", "Bluetooth");
    map.insert("Device", "Bluetooth");
    map.insert("Device File", "event3");
    m_deviceInput->m_KeyToLshw = "abcd";
    m_deviceInput->setInfoFromlshw(map);
    m_deviceInput->setInfoFromHwinfo(map);
}

TEST_F(DeviceInput_UT, DeviceInput_UT_setKLUInfoFromHwinfo)
{
    QMap<QString, QString> map;
    map.insert("Hotplug", "abc");
    map.insert("Model", "Bluetooth");
    map.insert("Device", "Bluetooth");
    map.insert("Device File", "event3");
    map.insert("SysFS BusID", "/-/://");
    m_deviceInput->setKLUInfoFromHwinfo(map);
}

bool ut_isValueValid()
{
    return true;
}

TEST_F(DeviceInput_UT, DeviceInput_UT_setInfoFromInput)
{
    m_deviceInput->setInfoFromInput();
    Stub stub;
    stub.set(ADDR(DeviceInput, isValueValid), ut_isValueValid);
    m_deviceInput->setInfoFromBluetoothctl();
}

TEST_F(DeviceInput_UT, DeviceInput_UT_name)
{
    m_deviceInput->name();
    m_deviceInput->driver();
    m_deviceInput->subTitle();
}

TEST_F(DeviceInput_UT, DeviceInput_UT_initFilterKey)
{
    m_deviceInput->initFilterKey();
    m_deviceInput->getOverviewInfo();
    m_deviceInput->loadBaseDeviceInfo();
    m_deviceInput->loadOtherDeviceInfo();
    m_deviceInput->loadTableHeader();
    m_deviceInput->loadTableData();
}
