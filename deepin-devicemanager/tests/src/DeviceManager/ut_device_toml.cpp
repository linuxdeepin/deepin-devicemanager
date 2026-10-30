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

// ============================================================================
// 系统总线隔离（避免真机鉴权弹窗）：
// DevicePower::setInfoFromTomlOneByOne → getOtherMapInfo → Common::
// checkBoardVendorFlag() 进程内首次初始化时会：
//   1) 真实执行 dmidecode -s system-product-name（无 root 时输出为空）；
//   2) 空则走 DBusInterface::getInfo("dmidecode_spn") 向
//      org.deepin.DeviceInfo 系统总线发调用 —— 真机上会触发鉴权弹窗；
//   3) 失败再回退 /tmp/device-info/dmidecode_spn.txt（root 属主目录，
//      测试进程不可写）。
// 处理：本 fixture 用打桩注入真实采集数据（见 ut_stub_getinfo_captured），
// 全程离线且不污染其它套件（Stub 析构自动恢复原函数入口）。
// ============================================================================

// 真实环境采集数据（2025-10-29，本机 root 服务缓存）：
//   gdbus call --system --dest org.deepin.DeviceInfo
//     --object-path /org/deepin/DeviceInfo
//     --method org.deepin.DeviceInfo.getInfo dmidecode_spn → ('E500\n',)
// 语义：dmidecode -s system-product-name 的输出（服务端 threadpool.cpp 写入
// /tmp/device-info/dmidecode_spn.txt 的内容）。注：读取路径未触发 polkit 鉴权。
static bool ut_stub_getinfo_captured(void *obj, const QString &key, QString &info)
{
    Q_UNUSED(obj);
    if (key == QLatin1String("dmidecode_spn")) {
        info = QStringLiteral("E500\n");
        return true;
    }
    return false;  // 其余 key 与 ut_disable_system_dbus 同义：无数据，不碰总线
}

class UT_DeviceToml : public UT_HEAD
{
public:
    void SetUp() override
    {
        // 每个用例独立打桩/恢复：避免与 ut_disable_system_dbus 的进程级静态桩
        // 互相覆盖导致 ut_dbusinterface（直接测 getInfo 本身）的用例失效
        m_dbusStub.set(ADDR(DBusInterface, getInfo), ut_stub_getinfo_captured);
    }

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

private:
    Stub m_dbusStub;  // 作用域限于本 fixture，析构时自动恢复 getInfo 原入口
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
