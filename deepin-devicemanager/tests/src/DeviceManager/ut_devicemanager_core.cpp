// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DeviceManager 核心链: convert*/createDevice(纯映射)、addCmdInfo/cmdInfo 往返、findBy*。
// 这些均为纯逻辑,无外部依赖。

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

class UT_DeviceManagerCore : public UT_HEAD
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
};

// convertDeviceTomlClassName: 纯映射,覆盖各分支
TEST_F(UT_DeviceManagerCore, convertDeviceTomlClassName)
{
    auto dm = DeviceManager::instance();
    EXPECT_EQ(dm->convertDeviceTomlClassName(DT_Null), QString(""));
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

// createDevice: new 对应子类
TEST_F(UT_DeviceManagerCore, createDevice)
{
    auto dm = DeviceManager::instance();
    DeviceBaseInfo *d = nullptr;

    d = dm->createDevice(DT_Storage);  EXPECT_NE(dynamic_cast<DeviceStorage *>(d), nullptr); delete d;
    d = dm->createDevice(DT_Cpu);      EXPECT_NE(dynamic_cast<DeviceCpu *>(d), nullptr);     delete d;
    d = dm->createDevice(DT_Memory);   EXPECT_NE(dynamic_cast<DeviceMemory *>(d), nullptr);  delete d;
    d = dm->createDevice(DT_Gpu);      EXPECT_NE(dynamic_cast<DeviceGpu *>(d), nullptr);     delete d;
    d = dm->createDevice(DT_Null);     EXPECT_EQ(d, nullptr);
}

// convertDeviceList / convertDeviceListAddr
TEST_F(UT_DeviceManagerCore, convertDeviceList)
{
    auto dm = DeviceManager::instance();
    EXPECT_EQ(dm->convertDeviceList(DT_Storage).size(), 0);
    EXPECT_NE(dm->convertDeviceListAddr(DT_Storage), nullptr);
    EXPECT_NE(dm->convertDeviceListAddr(DT_Cpu), dm->convertDeviceListAddr(DT_Storage));
}

// addCmdInfo / cmdInfo 往返
TEST_F(UT_DeviceManagerCore, addCmdInfo_cmdInfo)
{
    auto dm = DeviceManager::instance();
    dm->clear();

    QMap<QString, QList<QMap<QString, QString> > > cmd;
    QMap<QString, QString> e;
    e["K"] = "V";
    cmd["mykey"].append(e);
    dm->addCmdInfo(cmd);

    EXPECT_EQ(dm->cmdInfo("mykey").size(), 1);
    EXPECT_EQ(dm->cmdInfo("mykey").at(0).value("K"), QString("V"));
    EXPECT_EQ(dm->cmdInfo("notexist").size(), 0);
}

// findByModalias: 空参数/null device 提前 return,正常调用不崩
TEST_F(UT_DeviceManagerCore, findByModalias)
{
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    EXPECT_FALSE(dm->findByModalias(DT_Storage, &dev, ""));          // 空 modalias
    EXPECT_FALSE(dm->findByModalias(DT_Storage, nullptr, "pci:xx")); // null device

    dev.m_Modalias = "pci:v00001234d00005678";
    EXPECT_NO_FATAL_FAILURE(dm->findByModalias(DT_Storage, &dev, "pci:v00001234d00005678"));
    // 非标准 modalias + VIDAndPID 匹配分支
    dev.m_VID = "0x1234"; dev.m_PID = "0x5678";
    EXPECT_NO_FATAL_FAILURE(dm->findByModalias(DT_Storage, &dev, "custom12345678modalias"));
}

// findByVIDPID: 空 vid/pid / null device 提前 return,正常匹配
TEST_F(UT_DeviceManagerCore, findByVIDPID)
{
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, &dev, "", "0x5678"));    // 空 vid
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, &dev, "0x1234", ""));    // 空 pid
    EXPECT_FALSE(dm->findByVIDPID(DT_Storage, nullptr, "0x1234", "0x5678")); // null device

    dev.m_VID = "0x1234"; dev.m_PID = "0x5678";
    EXPECT_NO_FATAL_FAILURE(dm->findByVIDPID(DT_Storage, &dev, "0x1234", "0x5678"));
    EXPECT_NO_FATAL_FAILURE(dm->findByVIDPID(DT_Storage, &dev, "1234", "5678"));
}

// findByVendorName: 空 name / null device 提前 return; 正常调用不崩
TEST_F(UT_DeviceManagerCore, findByVendorName)
{
    auto dm = DeviceManager::instance();
    DeviceStorage dev;
    EXPECT_FALSE(dm->findByVendorName(DT_Storage, &dev, "vendor", "")); // 空 name
    EXPECT_FALSE(dm->findByVendorName(DT_Storage, nullptr, "v", "n"));  // null device
    EXPECT_NO_FATAL_FAILURE(dm->findByVendorName(DT_Storage, &dev, "Intel", "SSD"));
}
