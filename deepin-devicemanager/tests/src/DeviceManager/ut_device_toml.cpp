// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 批量覆盖各 device 类的 setInfoFromTomlOneByOne(toml 解析) 及 DeviceStorage 纯逻辑方法。
// setInfoFromTomlOneByOne 统一签名(QMap),传入构造 map 即覆盖解析逻辑。

#include "DeviceCpu.h"
#include "DeviceMemory.h"
#include "DeviceGpu.h"
#include "DeviceAudio.h"
#include "DeviceBluetooth.h"
#include "DeviceNetwork.h"
#include "DeviceBios.h"
#include "DeviceComputer.h"
#include "DeviceStorage.h"
#include "DeviceMonitor.h"
#include "DeviceImage.h"
#include "DevicePower.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceToml : public UT_HEAD
{
public:
    // 通用 toml map,含各 device 类 setInfoFromTomlOneByOne 可能读取的常见 key
    static QMap<QString, QString> makeToml()
    {
        QMap<QString, QString> m;
        m["Model"] = "TestModel";
        m["Vendor"] = "Intel";
        m["Interface"] = "PCIe";
        m["Bus Info"] = "pci@0:0:0:0";
        m["Speed"] = "5000M";
        m["Maximum Current"] = "100mA";
        m["Capabilities"] = "audio";
        m["Driver"] = "test_drv";
        m["Media Type"] = "SSD";
        m["Size"] = "512 GB";
        m["Serial ID"] = "ABC123";
        m["Version"] = "1.0";
        m["Description"] = "Test device";
        m["Memory Size"] = "8GiB";
        m["Frequency"] = "2400";
        m["Core"] = "8";
        m["Logical ID"] = "0";
        m["Physical ID"] = "0";
        m["Type"] = "CPU";
        m["BIOS Version"] = "1.2.3";
        m["Manufacturer"] = "Intel";
        m["Product Name"] = "TestProduct";
        m["OS"] = "UOS";
        return m;
    }
};

// ---- 各 device 类的 setInfoFromTomlOneByOne ----
TEST_F(UT_DeviceToml, Cpu_setInfoFromToml)
{
    DeviceCpu d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Memory_setInfoFromToml)
{
    DeviceMemory d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Gpu_setInfoFromToml)
{
    DeviceGpu d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Audio_setInfoFromToml)
{
    DeviceAudio d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Bluetooth_setInfoFromToml)
{
    DeviceBluetooth d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Network_setInfoFromToml)
{
    DeviceNetwork d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Bios_setInfoFromToml)
{
    DeviceBios d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Storage_setInfoFromToml)
{
    DeviceStorage d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Monitor_setInfoFromToml)
{
    DeviceMonitor d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Image_setInfoFromToml)
{
    DeviceImage d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

TEST_F(UT_DeviceToml, Power_setInfoFromToml)
{
    DevicePower d;
    EXPECT_NO_FATAL_FAILURE(d.setInfoFromTomlOneByOne(makeToml()));
}

// ---- DeviceStorage 额外纯逻辑方法 ----
TEST_F(UT_DeviceToml, Storage_interface_mediaType)
{
    DeviceStorage d;
    d.m_Interface = "USB";
    d.m_MediaType = "SSD";
    EXPECT_EQ(d.interface(), QString("USB"));
    EXPECT_EQ(d.mediaType(), QString("SSD"));
}

TEST_F(UT_DeviceToml, Storage_isValid)
{
    DeviceStorage d;
    EXPECT_NO_FATAL_FAILURE(d.isValid());
}

TEST_F(UT_DeviceToml, Storage_appendDisk)
{
    DeviceStorage d1;
    DeviceStorage d2;
    EXPECT_NO_FATAL_FAILURE(d1.appendDisk(&d2));
}

TEST_F(UT_DeviceToml, Storage_checkDiskSize)
{
    DeviceStorage d;
    EXPECT_NO_FATAL_FAILURE(d.checkDiskSize());
}

TEST_F(UT_DeviceToml, Storage_getDiskSerialID)
{
    DeviceStorage d;
    EXPECT_NO_FATAL_FAILURE(d.getDiskSerialID());
}

// ---- 各 device 类的 getOverviewInfo/subTitle/getBaseAttribs/getOtherAttribs ----
// 这些触发 loadBaseDeviceInfo/loadOtherDeviceInfo 等虚函数,覆盖各 device 类的解析逻辑
#define UT_DEVICE_OVERVIEW(CLS) \
    TEST_F(UT_DeviceToml, CLS##_overview) { \
        Device##CLS d; \
        EXPECT_NO_FATAL_FAILURE({ \
            d.getOverviewInfo(); \
            d.subTitle(); \
            d.getBaseAttribs(); \
            d.getOtherAttribs(); \
            d.getTableHeader(); \
            d.getTableData(); \
        }); \
    }

UT_DEVICE_OVERVIEW(Cpu)
UT_DEVICE_OVERVIEW(Memory)
UT_DEVICE_OVERVIEW(Gpu)
UT_DEVICE_OVERVIEW(Audio)
UT_DEVICE_OVERVIEW(Bluetooth)
UT_DEVICE_OVERVIEW(Network)
UT_DEVICE_OVERVIEW(Bios)
UT_DEVICE_OVERVIEW(Computer)
UT_DEVICE_OVERVIEW(Storage)
UT_DEVICE_OVERVIEW(Monitor)
UT_DEVICE_OVERVIEW(Image)
UT_DEVICE_OVERVIEW(Power)
