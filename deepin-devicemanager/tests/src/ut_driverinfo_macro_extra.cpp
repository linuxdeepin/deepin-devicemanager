// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 MacroDefinition.h 中 DriverInfo 结构体的 inline getter
// （vendorName / modelName / driverName / backupFileName）。

#include "MacroDefinition.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

TEST(UT_DriverInfoGetters, VendorName_ReturnsMember)
{
    DriverInfo di;
    di.m_VendorName = "HP";
    EXPECT_STREQ("HP", di.vendorName().toStdString().c_str());
}

TEST(UT_DriverInfoGetters, ModelName_ReturnsMember)
{
    DriverInfo di;
    di.m_ModelName = "LaserJet";
    EXPECT_STREQ("LaserJet", di.modelName().toStdString().c_str());
}

TEST(UT_DriverInfoGetters, DriverName_ReturnsMember)
{
    DriverInfo di;
    di.m_DriverName = "hplip";
    EXPECT_STREQ("hplip", di.driverName().toStdString().c_str());
}

TEST(UT_DriverInfoGetters, BackupFileName_ReturnsMember)
{
    DriverInfo di;
    di.m_BackupFileName = "hplip_3.deb";
    EXPECT_STREQ("hplip_3.deb", di.backupFileName().toStdString().c_str());
}
