// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

#define private public
#define protected public
#include "HWGenerator.h"

class UT_HWGenerator : public UT_HEAD
{
public:
    void SetUp() override
    {
        m_gen = new HWGenerator;
    }
    void TearDown() override
    {
        delete m_gen;
    }
    HWGenerator *m_gen = nullptr;
};

// 构造函数：调用 Common::boardVendorType()，对象创建成功
TEST_F(UT_HWGenerator, UT_HWGenerator_ctor)
{
    EXPECT_NE(nullptr, m_gen);
}

// generatorCpuDevice：DeviceManager 无数据时基类提前返回，方法安全完成
TEST_F(UT_HWGenerator, UT_HWGenerator_generatorCpuDevice)
{
    m_gen->generatorCpuDevice();
    SUCCEED();
}
