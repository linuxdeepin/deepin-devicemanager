// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MipsGenerator.h"
#include "DeviceManager.h"
#include "ut_Head.h"

#include <gtest/gtest.h>

class UT_MipsGenerator : public UT_HEAD
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

TEST_F(UT_MipsGenerator, ctor)
{
    MipsGenerator *gen = new MipsGenerator();
    EXPECT_NE(gen, nullptr);
    delete gen;
}

// cmdInfo 为空时，各 size()>0 分支不进入，但仍走完方法并 addComputerDevice
TEST_F(UT_MipsGenerator, generatorComputerDevice_emptyCmdInfo)
{
    MipsGenerator gen;
    gen.generatorComputerDevice();
    EXPECT_EQ(DeviceManager::instance()->m_ListDeviceComputer.size(), 1);
}
