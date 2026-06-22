// Copyright (C) 2019-2026 ~ 2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"
#include "DeviceManager.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "HWGenerator.h"

// 供 cmdInfo 桩返回的通用数据,覆盖各 generator 读取的常见 key
static QList<QMap<QString, QString> > ut_hw_lstMap;
static const QList<QMap<QString, QString> > &ut_hw_cmdInfo()
{
    return ut_hw_lstMap;
}

class UT_HWGenerator : public UT_HEAD
{
public:
    void SetUp() override
    {
        DeviceManager::instance()->clear();
        m_gen = new HWGenerator;

        QMap<QString, QString> m;
        m.insert("Manufacturer", "Samsung");
        m.insert("product", "TestProduct");
        m.insert("vendor", "TestVendor");
        m.insert("description", "TestDesc");
        m.insert("size", "8GiB");
        m.insert("Capacity", "931 GB");
        m.insert("serial", "0000000");
        m.insert("logical name", "eth0");
        m.insert("SysFS BusID", "0:0:0:0");
        m.insert("Hardware Class", "bluetooth");
        m.insert("OS", "Linux version test (gcc version 8.3.0) #1 SMP");
        m.insert("HOME_URL", "http://test");
        m.insert("type", "type");
        m.insert("Model", "camera");
        m.insert("revision", "0x01");
        m.insert("Vendor", "0x1234");
        m.insert("Product", "0x5678");
        // 放入多个元素,使 size()>0 / size()>1 等分支都能进入
        for (int i = 0; i < 3; ++i) {
            ut_hw_lstMap.append(m);
        }
    }
    void TearDown() override
    {
        delete m_gen;
        ut_hw_lstMap.clear();
        DeviceManager::instance()->clear();
    }
    HWGenerator *m_gen = nullptr;
};

// 构造函数：调用 Common::boardVendorType()，对象创建成功
TEST_F(UT_HWGenerator, UT_HWGenerator_ctor)
{
    EXPECT_NE(nullptr, m_gen);
}

// 以下各 generator 方法桩 cmdInfo 注入通用数据后调用,覆盖解析+add 逻辑
TEST_F(UT_HWGenerator, UT_HWGenerator_generatorComputerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorComputerDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorCpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorCpuDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorAudioDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorAudioDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorBluetoothDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorBluetoothDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorGpuDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorGpuDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorNetworkDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorNetworkDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorDiskDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorDiskDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorMonitorDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorMonitorDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorPowerDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorPowerDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorKeyboardDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorKeyboardDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorMemoryDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorMemoryDevice());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_generatorCameraDevice)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->generatorCameraDevice());
}

// ---------- protected 子方法(获取音频/蓝牙/内存信息的各来源) ----------
TEST_F(UT_HWGenerator, UT_HWGenerator_getAudioInfoFromCatAudio)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->getAudioInfoFromCatAudio());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_getBluetoothInfoFromHciconfig)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->getBluetoothInfoFromHciconfig());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_getBlueToothInfoFromHwinfo)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->getBlueToothInfoFromHwinfo());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_getBluetoothInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->getBluetoothInfoFromLshw());
}

TEST_F(UT_HWGenerator, UT_HWGenerator_getMemoryInfoFromLshw)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_hw_cmdInfo);
    EXPECT_NO_FATAL_FAILURE(m_gen->getMemoryInfoFromLshw());
}
