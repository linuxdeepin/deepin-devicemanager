// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CustomGenerator.h"
#include "DeviceManager.h"
#include "commontools.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

class UT_CustomGenerator : public UT_HEAD
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

TEST_F(UT_CustomGenerator, ctor)
{
    CustomGenerator *gen = new CustomGenerator();
    EXPECT_NE(gen, nullptr);
    delete gen;
}

// 桩 preGenerateGpuInfo 返回空 -> 提前 return(避免依赖真实 glxinfo 环境)
static QString ut_preGenerateGpuInfo_empty()
{
    return QString();
}

TEST_F(UT_CustomGenerator, generatorGpuDevice_empty)
{
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_preGenerateGpuInfo_empty);

    CustomGenerator gen;
    gen.generatorGpuDevice();
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
}

// 桩 preGenerateGpuInfo 返回构造数据，覆盖解析 + addGpuDevice 逻辑
static const char *ut_gpuinfo =
    "Vendor: Intel\nDevice: HD Graphics\n\n"
    "Vendor: NVIDIA\nDevice: GTX 1060";
static QString ut_preGenerateGpuInfo_data()
{
    return QString::fromUtf8(ut_gpuinfo);
}

TEST_F(UT_CustomGenerator, generatorGpuDevice_parse)
{
    Stub stub;
    stub.set(ADDR(CommonTools, preGenerateGpuInfo), ut_preGenerateGpuInfo_data);

    CustomGenerator gen;
    gen.generatorGpuDevice();
    EXPECT_GT(DeviceManager::instance()->m_ListDeviceGPU.size(), 0);
}

// generatorMonitorDevice: /sys/class/drm 不存在或无 edid 时安全完成
TEST_F(UT_CustomGenerator, generatorMonitorDevice)
{
    CustomGenerator gen;
    gen.generatorMonitorDevice();
    SUCCEED();
}
