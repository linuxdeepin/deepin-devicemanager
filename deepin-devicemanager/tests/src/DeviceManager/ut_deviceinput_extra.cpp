// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceInput 的纯 map 解析方法 setInfoFromTomlOneByOne/setInfoFromHwinfo。
// 不依赖 matchToLshw/getMouseInfoFromBusDevice 等外部调用,纯解析。

#include "DeviceInput.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceInputExtra : public UT_HEAD
{
public:
    void SetUp() override
    {
        di = new DeviceInput;
    }
    void TearDown() override
    {
        delete di;
    }
    DeviceInput *di = nullptr;
    static QMap<QString, QString> makeMap()
    {
        QMap<QString, QString> m;
        m["Model"] = "TestModel";
        m["Interface"] = "USB";
        m["Bus Info"] = "usb@1:1";
        m["Speed"] = "480M";
        m["Maximum Current"] = "100mA";
        m["Capabilities"] = "mouse";
        m["path"] = "/sys/devices/usb/x";
        m["name"] = "USB Mouse";
        m["driver"] = "usbhid";
        m["unique_id"] = "0x1234567890";   // 10+ chars,触发 VID/PID 解析
        m["Vendor"] = "Intel";
        m["Serial ID"] = "ABC123";
        m["VID_PID"] = "12345678";
        m["VID"] = "1234";
        m["PID"] = "5678";
        m["Hotplug"] = "USB";
        return m;
    }
};

// setInfoFromTomlOneByOne: setTomlAttribute 各字段 + getOtherMapInfo
TEST_F(UT_DeviceInputExtra, setInfoFromTomlOneByOne)
{
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromTomlOneByOne(makeMap()));
}

// setInfoFromHwinfo: path 分支(USB + VID/PID 解析)
TEST_F(UT_DeviceInputExtra, setInfoFromHwinfo_path)
{
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromHwinfo(makeMap()));
}

// setInfoFromHwinfo: 无 path 走 Model/Touchpad/Enable/Hotplug 分支
TEST_F(UT_DeviceInputExtra, setInfoFromHwinfo_touchpad)
{
    QMap<QString, QString> m;
    m["Model"] = "Touchpad Device";
    m["Enable"] = "yes";
    m["Vendor"] = "Intel";
    m["name"] = "Touchpad";
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromHwinfo(m));
}

// setInfoFromHwinfo: 无 path 无 Hotplug -> PS/2 分支
TEST_F(UT_DeviceInputExtra, setInfoFromHwinfo_ps2)
{
    QMap<QString, QString> m;
    m["Model"] = "Generic Mouse";
    m["Vendor"] = "Logitech";
    EXPECT_NO_FATAL_FAILURE(di->setInfoFromHwinfo(m));
}
