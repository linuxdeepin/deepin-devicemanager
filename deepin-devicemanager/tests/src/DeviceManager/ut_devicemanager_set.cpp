// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager 未测的 set*From 数据加载方法(纯 map 解析,创建设备+setInfoFromXxx+add)。
// 这些是 ut_devicemanager.cpp 未覆盖的真未触达大块。

#include "DeviceManager.h"
#include "DeviceAudio.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerSet : public UT_HEAD
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
    // 通用 map,含各 set 方法可能读取的常见 key
    static QMap<QString, QString> makeMap()
    {
        QMap<QString, QString> m;
        m["Vendor"] = "Intel";
        m["Vendor_ID"] = "0x8086";
        m["Product_ID"] = "0x1234";
        m["Modalias"] = "pci:v00001234d00005678";
        m["Model"] = "TestModel";
        m["Sound Interface"] = "PCI";
        m["Serial ID"] = "ABC123";
        m["Driver"] = "snd_hda_intel";
        m["size"] = "8GiB";
        m["Capacity"] = "1TB";
        m["description"] = "CPU";
        m["logical name"] = "eth0";
        m["cpu_id"] = "0";
        m["L1 cache"] = "32KB";
        m["core"] = "8";
        m["thread"] = "16";
        m["HZ"] = "2400";
        m["product"] = "TestProduct";
        m["SysFS BusID"] = "0:0:0:0";
        return m;
    }
};

TEST_F(UT_DeviceManagerSet, setMemoryInfoFromDmidecode)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->setMemoryInfoFromDmidecode(makeMap()));
}

TEST_F(UT_DeviceManagerSet, setMonitorInfoFromDbus)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->setMonitorInfoFromDbus(makeMap()));
}

// 注: setAudioInfoFromToml / setAudioInfoFrom_sysFS 在头文件声明但无实现(条件编译),不测

TEST_F(UT_DeviceManagerSet, setCameraInfoFromLshw)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->setCameraInfoFromLshw(makeMap()));
}

TEST_F(UT_DeviceManagerSet, setCpuRefreshInfoFromlscpu)
{
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->setCpuRefreshInfoFromlscpu(makeMap()));
}

TEST_F(UT_DeviceManagerSet, setAudioDeviceEnable)
{
    DeviceAudio *audio = new DeviceAudio;
    EXPECT_NO_FATAL_FAILURE(DeviceManager::instance()->setAudioDeviceEnable(audio, true));
    delete audio;
}
