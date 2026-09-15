// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 覆盖率缺口补全：MacroDefinition.h 中 DriverInfo 结构的 backupFileName/modelName/
// driverName/vendorName 四个内联 getter 此前未被调用，函数覆盖率为 0。
// 本文件通过构造 DriverInfo 并回写成员，验证 getter 取值正确。

#include "MacroDefinition.h"

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

TEST(UT_DriverInfo_Getters, BackupFileName_SetValue_ReturnsSameValue)
{
    DriverInfo info;
    info.m_BackupFileName = "nvidia-525-backup.deb";
    EXPECT_EQ(info.backupFileName(), "nvidia-525-backup.deb");
}

TEST(UT_DriverInfo_Getters, ModelName_SetValue_ReturnsSameValue)
{
    DriverInfo info;
    info.m_ModelName = "GeForce RTX 3060";
    EXPECT_EQ(info.modelName(), "GeForce RTX 3060");
}

TEST(UT_DriverInfo_Getters, DriverName_SetValue_ReturnsSameValue)
{
    DriverInfo info;
    info.m_DriverName = "nvidia-driver-525";
    EXPECT_EQ(info.driverName(), "nvidia-driver-525");
}

TEST(UT_DriverInfo_Getters, VendorName_SetValue_ReturnsSameValue)
{
    DriverInfo info;
    info.m_VendorName = "NVIDIA Corporation";
    EXPECT_EQ(info.vendorName(), "NVIDIA Corporation");
}

// 默认构造后各 getter 应返回空串，验证默认值语义
TEST(UT_DriverInfo_Getters, DefaultCtor_GettersReturnEmptyString)
{
    DriverInfo info;
    EXPECT_TRUE(info.backupFileName().isEmpty());
    EXPECT_TRUE(info.modelName().isEmpty());
    EXPECT_TRUE(info.driverName().isEmpty());
    EXPECT_TRUE(info.vendorName().isEmpty());
}
