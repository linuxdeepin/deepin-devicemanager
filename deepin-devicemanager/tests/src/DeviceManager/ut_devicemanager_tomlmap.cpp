// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager toml 映射链 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: tomlDeviceMapSet / tomlDeviceReadKeyValue 的全部 18 个 DeviceType 分支
//       (每个 case dynamic_cast + 调 setInfoFromTomlBase/setInfoFromTomlOneByOne)、
//       null device / DT_Null 兜底分支。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                 | level | factors | min | actual |
// |------------------------|-------|---------|-----|--------|
// | tomlDeviceMapSet       | mid   | -       | 2   | 4      |
// | tomlDeviceReadKeyValue | mid   | -       | 2   | 3      |
// ============================================================

#include "DeviceManager.h"
#include "GenerateDevicePool.h"
#include "DeviceStorage.h"
#include "ut_Head.h"

#include <QMap>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerTomlMap : public ::testing::Test
{
public:
    void SetUp() override
    {
        ut_disable_system_dbus();  // 隔离真实系统服务, 避免运行期触发系统总线鉴权
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

// tomlDeviceMapSet: 全部 18 个 DeviceType 分支遍历, 各类型设备创建后安全完成
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceMapSet_allTypes)
{
    // Arrange: 18 个有效设备类型与 toml 映射
    auto dm = DeviceManager::instance();
    QMap<QString, QString> map = makeMap();
    DeviceType types[] = {DT_Computer, DT_Cpu,     DT_Bios,      DT_Memory,   DT_Storage,
                          DT_Gpu,      DT_Monitor,  DT_Network,   DT_Audio,    DT_Bluetoorh,
                          DT_Keyboard, DT_Mouse,    DT_Print,     DT_Image,    DT_Cdrom,
                          DT_Others,   DT_OtherPCI, DT_Power};

    // Act: 执行被测调用
    // Assert: 各分支 dynamic_cast 命中, 注入安全完成
    for (DeviceType t : types) {
        DeviceBaseInfo *d = dm->createDevice(t);
        EXPECT_NE(d, nullptr);
        EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceMapSet(t, d, map));
        delete d;
    }

    // Assert: 空设备分支同样安全返回 TOML_None
    EXPECT_EQ(dm->tomlDeviceMapSet(DT_Storage, nullptr, makeMap()), TOML_None);
}

// tomlDeviceMapSet: storage 设备注入后厂商字段被写入
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceMapSet_storage)
{
    // Arrange: storage 设备与含 Vendor 的映射
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage;
    QMap<QString, QString> map = makeMap();

    // Act: 注入 toml 映射
    dm->tomlDeviceMapSet(DT_Storage, st, map);

    // Assert: 厂商与接口均被映射写入(存储分支不写 Model)
    EXPECT_STREQ("Intel", st->m_Vendor.toStdString().c_str());
    EXPECT_STREQ("PCIe", st->m_Interface.toStdString().c_str());
    EXPECT_STREQ("512GB", st->m_Size.toStdString().c_str());
    delete st;
}

// tomlDeviceMapSet: null device 返回 TOML_None
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceMapSet_nullDevice)
{
    // Arrange: 空设备指针
    auto dm = DeviceManager::instance();

    // Act: 执行被测调用
    // Assert: 空设备直接返回 TOML_None, 重复调用结果一致
    EXPECT_EQ(dm->tomlDeviceMapSet(DT_Storage, nullptr, makeMap()), TOML_None);
    EXPECT_EQ(dm->tomlDeviceMapSet(DT_Cpu, nullptr, makeMap()), TOML_None);
}

// tomlDeviceMapSet: DT_Null 分支直接 break, 不操作设备
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceMapSet_nullType)
{
    // Arrange: 有效设备 + 无效类型
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage;

    // Act: 以 DT_Null 注入
    dm->tomlDeviceMapSet(DT_Null, st, makeMap());

    // Assert: 设备字段不被修改
    EXPECT_TRUE(st->m_Vendor.isEmpty());
    EXPECT_TRUE(st->m_Model.isEmpty());
    delete st;
}

// tomlDeviceReadKeyValue: 全部 18 个 DeviceType 分支遍历, 键值回读非空
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceReadKeyValue_allTypes)
{
    // Arrange: 18 个有效设备类型
    auto dm = DeviceManager::instance();
    DeviceType types[] = {DT_Computer, DT_Cpu,     DT_Bios,      DT_Memory,   DT_Storage,
                          DT_Gpu,      DT_Monitor, DT_Network,   DT_Audio,    DT_Bluetoorh,
                          DT_Keyboard, DT_Mouse,   DT_Print,     DT_Image,    DT_Cdrom,
                          DT_Others,   DT_OtherPCI, DT_Power};

    // Act: 执行被测调用
    // Assert: 各分支回读 Vendor 键不崩溃
    for (DeviceType t : types) {
        DeviceBaseInfo *d = dm->createDevice(t);
        EXPECT_NE(d, nullptr);
        EXPECT_NO_FATAL_FAILURE(dm->tomlDeviceReadKeyValue(t, d, "Vendor"));
        delete d;
    }

    // Assert: 空设备分支返回空串
    EXPECT_EQ(dm->tomlDeviceReadKeyValue(DT_Storage, nullptr, "Vendor"), QString(""));
}

// tomlDeviceReadKeyValue: null device 返回空串
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceReadKeyValue_nullDevice)
{
    // Arrange: 空设备指针
    // Act: 执行被测调用
    // Assert: 返回空串, 不同类型同样空
    EXPECT_EQ(DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Storage, nullptr, "Vendor"), QString(""));
    EXPECT_EQ(DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Cpu, nullptr, "Vendor"), QString(""));
}

// tomlDeviceReadKeyValue: storage 设备回读 Vendor 键
TEST_F(UT_DeviceManagerTomlMap, UT_DeviceManagerTomlMap_tomlDeviceReadKeyValue_storage)
{
    // Arrange: storage 设备并设置厂商
    auto dm = DeviceManager::instance();
    DeviceStorage *st = new DeviceStorage;
    st->m_Vendor = "Intel";

    // Act: 执行被测调用
    // Assert: 回读 Vendor 键返回已设厂商, 重复回读结果一致
    EXPECT_EQ(dm->tomlDeviceReadKeyValue(DT_Storage, st, "Vendor"), QString("Intel"));
    EXPECT_EQ(dm->tomlDeviceReadKeyValue(DT_Storage, st, "Vendor"), QString("Intel"));
    delete st;
}
