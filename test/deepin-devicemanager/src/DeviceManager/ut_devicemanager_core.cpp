// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================
// 测试对象: DeviceManager 核心链 (deepin-devicemanager/src/DeviceManager/DeviceManager.cpp)
// 覆盖: convert*/createDevice(纯映射)、addCmdInfo/cmdInfo 往返、findBy*(设备匹配)。
// 均为纯逻辑, 无外部依赖。
//
// 用例计数声明（self-check-structural 验证此块）：
// | method                      | level | factors | min | actual |
// |-----------------------------|-------|---------|-----|--------|
// | convertDeviceTomlClassName  | mid   | -       | 2   | 2      |
// | createDevice                | mid   | -       | 2   | 2      |
// | convertDeviceList           | low   | -       | 1   | 1      |
// | addCmdInfo/cmdInfo 往返     | low   | -       | 1   | 1      |
// | findByModalias              | mid   | -       | 2   | 2      |
// | findByVIDPID                | mid   | -       | 2   | 2      |
// | findByVendorName            | mid   | -       | 2   | 2      |
// ============================================================

#include "DeviceManager.h"
#include "GenerateDevicePool.h"   // DeviceType/DT_ 枚举
#include "DeviceStorage.h"
#include "DeviceCpu.h"
#include "DeviceMemory.h"
#include "DeviceGpu.h"
#include "ut_Head.h"

#include <QMap>
#include <QList>
#include <QString>

#include <gtest/gtest.h>

class UT_DeviceManagerCore : public ::testing::Test
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
};

// convertDeviceTomlClassName: 纯映射, 覆盖合法枚举分支与未知枚举兜底分支
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_convertDeviceTomlClassName_known)
{
    // Arrange: 合法 DeviceType 枚举集合
    auto dm = DeviceManager::instance();

    // Act: 执行被测调用
    // Assert: 每个枚举映射到对应 toml 类名
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Computer), QString("tomlComputer"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Cpu), QString("tomlCPU"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Bios), QString("tomlMotherboard"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Memory), QString("tomlMemory"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Gpu), QString("tomlDisplayGPU"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Audio), QString("tomlSoundAudio"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Storage), QString("tomlStorage"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_OtherPCI), QString("tomlOtherPCI"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Power), QString("tomlPower"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Bluetoorh), QString("tomlBluetooth"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Network), QString("tomlNetWork"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Mouse), QString("tomlMouse"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Keyboard), QString("tomlKeyboard"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Monitor), QString("tomlMonitor"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Cdrom), QString("tomlCDROM"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Print), QString("tomlPrinter"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Image), QString("tomlCamera"));
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Others), QString("tomlOtherDevices"));
}

// convertDeviceTomlClassName: 未知枚举兜底
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_convertDeviceTomlClassName_unknown)
{
    // Arrange: 未知设备类型
    auto dm = DeviceManager::instance();

    // Act: 执行被测调用
    // Assert: 未识别类型返回空串
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Null), QString(""));
    EXPECT_TRUE(dm->convertDeviceTomlClassName(DT_Null).isEmpty());
}

// createDevice: 正常类型 new 对应子类实例
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_createDevice_valid)
{
    // Arrange: 有效设备类型
    auto dm = DeviceManager::instance();
    DeviceBaseInfo *d = nullptr;

    // Act: 执行被测调用
    // Assert: 各类型创建对应子类实例
    d = dm->createDevice(DT_Storage);
    EXPECT_NE(dynamic_cast<DeviceStorage *>(d), nullptr);
    EXPECT_NE(d, nullptr);
    delete d;

    d = dm->createDevice(DT_Cpu);
    EXPECT_NE(dynamic_cast<DeviceCpu *>(d), nullptr);
    delete d;

    d = dm->createDevice(DT_Memory);
    EXPECT_NE(dynamic_cast<DeviceMemory *>(d), nullptr);
    delete d;

    d = dm->createDevice(DT_Gpu);
    EXPECT_NE(dynamic_cast<DeviceGpu *>(d), nullptr);
    delete d;
}

// createDevice: 未知类型返回空指针
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_createDevice_null)
{
    // Arrange: 未知设备类型
    auto dm = DeviceManager::instance();

    // Act
    DeviceBaseInfo *d = dm->createDevice(DT_Null);

    // Assert: 未识别类型不创建对象, 重复调用同样不创建
    EXPECT_EQ(d, nullptr);
    EXPECT_EQ(dm->createDevice(DT_Null), nullptr);
}

// convertDeviceList / convertDeviceListAddr: 空列表与地址表
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_convertDeviceList)
{
    // Arrange: 单例, 无设备
    auto dm = DeviceManager::instance();

    // Act: 获取存储设备列表与地址表
    QList<DeviceBaseInfo *> lst = dm->convertDeviceList(DT_Storage);
    QList<DeviceBaseInfo *> *addrStorage = dm->convertDeviceListAddr(DT_Storage);
    QList<DeviceBaseInfo *> *addrCpu = dm->convertDeviceListAddr(DT_Cpu);

    // Assert: 空列表; 地址表非空且不同类型指向不同列表
    EXPECT_EQ(lst.size(), 0);
    EXPECT_NE(addrStorage, nullptr);
    EXPECT_NE(addrCpu, addrStorage);
}

// addCmdInfo / cmdInfo 往返: 写入后可读出, 不存在键返回空
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_addCmdInfo_cmdInfo)
{
    // Arrange: 构造一条命令输出缓存
    auto dm = DeviceManager::instance();
    dm->clear();

    QMap<QString, QList<QMap<QString, QString> > > cmd;
    QMap<QString, QString> e;
    e["K"] = "V";
    cmd["mykey"].append(e);

    // Act: 写入并读出
    dm->addCmdInfo(cmd);

    // Assert: 往返一致, 不存在的键返回空列表
    EXPECT_EQ(dm->cmdInfo("mykey").size(), 1);
    EXPECT_EQ(dm->cmdInfo("mykey").at(0).value("K"), QString("V"));
    EXPECT_EQ(dm->cmdInfo("notexist").size(), 0);
    EXPECT_TRUE(dm->cmdInfo("notexist").isEmpty());
}

// findByModalias: 空 modalias / null device 提前返回 false; 相同 modalias 与自定义 VIDPID 均可命中
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByModalias_invalid)
{
    // Arrange: 存储设备与空/null 入参
    auto dm = DeviceManager::instance();
    DeviceStorage dev;

    // Act: 执行被测调用
    // Assert: 空 modalias 或 null device 直接返回 false
    EXPECT_FALSE(dm->findByModalias(DT_Storage, &dev, ""));
    EXPECT_FALSE(dm->findByModalias(DT_Storage, nullptr, "pci:xx"));
}

TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByModalias_match)
{
    // Arrange: 设备 modalias 与查询串一致; 再构造 VIDPID 自定义场景
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    dev.m_Modalias = "pci:v00001234d00005678";

    // Act: 执行被测调用
    // Assert: 标准 pci modalias 精确(忽略大小写)命中
    EXPECT_TRUE(dm->findByModalias(DT_Storage, &dev, "pci:v00001234d00005678"));
    EXPECT_TRUE(dm->findByModalias(DT_Storage, &dev, "PCI:V00001234D00005678"));

    // Act: 执行被测调用
    // Assert: 自定义 modalias 通过 VIDPID 缓存(m_VID_PID)包含关系命中
    dev.m_VID = "0x1234";
    dev.m_PID = "0x5678";
    dev.m_VID_PID = "0x12345678";
    EXPECT_TRUE(dm->findByModalias(DT_Storage, &dev, "custom12345678modalias"));
}

// findByVIDPID: 空 vid/pid / null device 提前返回; 正常 VIDPID 匹配命中
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByVIDPID_invalid)
{
    // Arrange: 存储设备
    auto dm = DeviceManager::instance();
    DeviceStorage dev;

    // Act: 执行被测调用
    // Assert: 空 vid/pid 或 null device 直接返回 false
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, &dev, "", "0x5678"));
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, &dev, "0x1234", ""));
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, nullptr, "0x1234", "0x5678"));
}

TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByVIDPID_match)
{
    // Arrange: 设备带 VID/PID(带 0x 前缀)
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    dev.m_VID = "0x1234";
    dev.m_PID = "0x5678";

    // Act: 执行被测调用
    // Assert: 带 0x 前缀与不带前缀两种查询均命中
    EXPECT_TRUE(dm->findByVIDPID(DT_Storage, &dev, "0x1234", "0x5678"));
    EXPECT_TRUE(dm->findByVIDPID(DT_Storage, &dev, "1234", "5678"));
}

// findByVendorName: 空 name / null device 提前返回; vendor+name 均相同才命中
TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByVendorName_invalid)
{
    // Arrange: 存储设备
    auto dm = DeviceManager::instance();
    DeviceStorage dev;

    // Act: 执行被测调用
    // Assert: 空 name 或 null device 直接返回 false
    EXPECT_FALSE(dm->findByVendorName(DT_Storage, &dev, "vendor", ""));
    EXPECT_FALSE(dm->findByVendorName(DT_Storage, nullptr, "v", "n"));
}

TEST_F(UT_DeviceManagerCore, UT_DeviceManagerCore_findByVendorName_match)
{
    // Arrange: 设备厂商与名称已设置
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    dev.m_Vendor = "Intel";
    dev.m_Name = "SSD";

    // Act: 执行被测调用
    // Assert: 厂商名称完全一致(忽略大小写)命中
    EXPECT_TRUE(dm->findByVendorName(DT_Storage, &dev, "Intel", "SSD"));
    EXPECT_TRUE(dm->findByVendorName(DT_Storage, &dev, "intel", "ssd"));
}
