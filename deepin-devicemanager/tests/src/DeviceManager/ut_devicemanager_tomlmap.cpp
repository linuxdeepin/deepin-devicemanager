// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager::tomlDeviceMapSet 的全部 18 个 DeviceType 分支
// (每个 case dynamic_cast + 调 setInfoFromTomlBase/setInfoFromTomlOneByOne)。

#include "DeviceManager.h"
#include "GenerateDevicePool.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerTomlMap : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
    }
    void TearDown() override
    {
        DeviceManager::instance()->clear();
    }
    static QMap<QString, QString> makeMap()
    {
        QMap<QString, QString> m;
        m["Vendor"] = "Intel";
        m["Model"] = "TestModel";
        m["Vendor_ID"] = "0x8086";
        m["Product_ID"] = "0x1234";
        m["Modalias"] = "pci:v00001234d00005678";
        m["Driver"] = "test_drv";
        m["Serial ID"] = "ABC123";
        m["Size"] = "512GB";
        m["Type"] = "CPU";
        m["Interface"] = "PCIe";
        return m;
    }
};

// 全部 18 个 DeviceType 分支
TEST_F(UT_DeviceManagerTomlMap, tomlDeviceMapSet_allTypes)
{
    auto dm = DeviceManager::instance();
    QMap<QString, QString> map = makeMap();
    DeviceType types[] = {DT_Computer, DT_Cpu,     DT_Bios,      DT_Memory,   DT_Storage,
                          DT_Gpu,      DT_Monitor,  DT_Network,   DT_Audio,    DT_Bluetoorh,
                          DT_Keyboard, DT_Mouse,    DT_Print,     DT_Image,    DT_Cdrom,
                          DT_Others,   DT_OtherPCI, DT_Power};
    for (DeviceType t : types) {
        DeviceBaseInfo *d = dm->createDevice(t);
        EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceMapSet(t, d, map));
        delete d;
    }
}

// null device 分支
TEST_F(UT_DeviceManagerTomlMap, tomlDeviceMapSet_nullDevice)
{
    auto dm = DeviceManager::instance();
    EXPECT_EQ(dm->tomlDeviceMapSet(DT_Storage, nullptr, makeMap()), TOML_None);
}

// DT_Null 分支(break)
TEST_F(UT_DeviceManagerTomlMap, tomlDeviceMapSet_nullType)
{
    auto dm = DeviceManager::instance();
    DeviceBaseInfo *d = dm->createDevice(DT_Storage);
    EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceMapSet(DT_Null, d, makeMap()));
    delete d;
}

// tomlDeviceReadKeyValue: 全部 18 个 DeviceType 分支(各调 readDeviceInfoKeyValue)
TEST_F(UT_DeviceManagerTomlMap, tomlDeviceReadKeyValue_allTypes)
{
    auto dm = DeviceManager::instance();
    DeviceType types[] = {DT_Computer, DT_Cpu,     DT_Bios,      DT_Memory,   DT_Storage,
                          DT_Gpu,      DT_Monitor,  DT_Network,   DT_Audio,    DT_Bluetoorh,
                          DT_Keyboard, DT_Mouse,    DT_Print,     DT_Image,    DT_Cdrom,
                          DT_Others,   DT_OtherPCI, DT_Power};
    for (DeviceType t : types) {
        DeviceBaseInfo *d = dm->createDevice(t);
        EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceReadKeyValue(t, d, "Vendor"));
        delete d;
    }
}

TEST_F(UT_DeviceManagerTomlMap, tomlDeviceReadKeyValue_nullDevice)
{
    EXPECT_EQ(DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Storage, nullptr, "Vendor"), QString(""));
}

TEST_F(UT_DeviceManagerTomlMap, tomlDeviceReadKeyValue_nullType)
{
    auto dm = DeviceManager::instance();
    DeviceBaseInfo *d = dm->createDevice(DT_Storage);
    EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceReadKeyValue(DT_Null, d, "Vendor"));
    delete d;
}
