// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>
#include <QRegularExpression>

#define private public
#define protected public
#include "commonfunction.h"

class UT_Common : public UT_HEAD
{
public:
    void SetUp() override
    {
        // 保存静态状态，避免用例间互相干扰
        m_savedSpecialComType = Common::specialComType;
    }
    void TearDown() override
    {
        // 恢复静态状态
        Common::specialComType = m_savedSpecialComType;
    }
    int m_savedSpecialComType = -1;
};

// ===================== formatTotalCache =====================
TEST_F(UT_Common, UT_Common_formatTotalCache_KiB)
{
    EXPECT_STREQ("120 KiB", Common::formatTotalCache("30 KB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_to_MiB)
{
    EXPECT_STREQ("2 MiB", Common::formatTotalCache("256 KB", 8).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_unit)
{
    // 无空格单位 "KiB" 命中 K 分支
    EXPECT_STREQ("2 KiB", Common::formatTotalCache("1KiB", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_MiB)
{
    // 无空格单位 "MiB" 命中 M 分支
    EXPECT_STREQ("4 MiB", Common::formatTotalCache("1MiB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_MiB_decimal)
{
    EXPECT_STREQ("3 MiB", Common::formatTotalCache("1.5MiB", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_GiB)
{
    EXPECT_STREQ("1 GiB", Common::formatTotalCache("1GiB", 1).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_TiB)
{
    // T 分支：1TiB * 1 = 1024 GiB
    EXPECT_STREQ("1024 GiB", Common::formatTotalCache("1TiB", 1).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_unknownUnit)
{
    // 带空格的 "1 MB" 单位解析为 " MB"(未知) 走 else 分支按原值当 KiB
    EXPECT_STREQ("4 KiB", Common::formatTotalCache("1 MB", 4).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_KiB_decimal)
{
    EXPECT_STREQ("4.5 KiB", Common::formatTotalCache("1.5 KiB", 3).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_noUnit_bytes)
{
    // 无单位视为字节: 512/1024=0.5 KiB/核 * 2 核 = 1 KiB
    EXPECT_STREQ("1 KiB", Common::formatTotalCache("512", 2).toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_empty)
{
    EXPECT_TRUE(Common::formatTotalCache("", 4).isEmpty());
}

TEST_F(UT_Common, UT_Common_formatTotalCache_invalidNumber)
{
    EXPECT_TRUE(Common::formatTotalCache("abc", 4).isEmpty());
}

// ===================== parseSharedCpuCount =====================
TEST_F(UT_Common, UT_Common_parseSharedCpuCount_mixed)
{
    // 0-3(4) + 8(1) + 10-12(3) = 8
    EXPECT_EQ(8, Common::parseSharedCpuCount("0-3,8,10-12"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleRange)
{
    EXPECT_EQ(4, Common::parseSharedCpuCount("0-3"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleNumbers)
{
    EXPECT_EQ(3, Common::parseSharedCpuCount("1,2,3"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_empty)
{
    EXPECT_EQ(0, Common::parseSharedCpuCount(""));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_singleValue)
{
    EXPECT_EQ(1, Common::parseSharedCpuCount("5"));
}

TEST_F(UT_Common, UT_Common_parseSharedCpuCount_withInvalid)
{
    // 0-3(4), abc 无效不计
    EXPECT_EQ(4, Common::parseSharedCpuCount("0-3,abc"));
}

// ===================== specialHString / specialVendorType =====================
TEST_F(UT_Common, UT_Common_specialHString)
{
    // ASCII {72,85,65,87,69,73} => "HUAWEI"
    EXPECT_STREQ("HUAWEI", Common::specialHString().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_specialVendorType_hw)
{
    Common::specialComType = Common::PGUW;
    // 刷新 boardVendorType 缓存，使其反映当前 specialComType
    Common::checkBoardVendorFlag();
    EXPECT_STREQ("HUAWEI", Common::specialVendorType().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_specialVendorType_normal)
{
    Common::specialComType = Common::NormalCom;
    Common::checkBoardVendorFlag();
    EXPECT_STREQ("normalmagical", Common::specialVendorType().toStdString().c_str());
}

// ===================== isHwPlatform =====================
TEST_F(UT_Common, UT_Common_isHwPlatform_unknow)
{
    Common::specialComType = Common::Unknow;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_normal)
{
    Common::specialComType = Common::NormalCom;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_custom)
{
    Common::specialComType = Common::kCustomType;
    EXPECT_FALSE(Common::isHwPlatform());
}

TEST_F(UT_Common, UT_Common_isHwPlatform_special)
{
    Common::specialComType = Common::PGUW;
    EXPECT_TRUE(Common::isHwPlatform());
}

// ===================== checkBoardVendorFlag (specialComType 分支) =====================
TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_normal)
{
    Common::specialComType = Common::NormalCom;
    EXPECT_STREQ("", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_pguw)
{
    Common::specialComType = Common::PGUW;
    EXPECT_STREQ("PGUW", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_klvv)
{
    Common::specialComType = Common::KLVV;
    EXPECT_STREQ("KLVV", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_klvu)
{
    Common::specialComType = Common::KLVU;
    EXPECT_STREQ("KLVU", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_pguv)
{
    Common::specialComType = Common::PGUV;
    EXPECT_STREQ("PGUV", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_type5)
{
    Common::specialComType = Common::kSpecialType5;
    EXPECT_STREQ("PGUX", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_custom)
{
    Common::specialComType = Common::kCustomType;
    EXPECT_STREQ("CustomType", Common::checkBoardVendorFlag().toStdString().c_str());
}

TEST_F(UT_Common, UT_Common_checkBoardVendorFlag_default)
{
    // 未列出的特殊类型走 default 分支 => PGUW
    Common::specialComType = Common::kSpecialType6;
    EXPECT_STREQ("PGUW", Common::checkBoardVendorFlag().toStdString().c_str());
}

// ===================== tomlFilesName get/set =====================
TEST_F(UT_Common, UT_Common_tomlFilesName)
{
    Common::tomlFilesNameSet("myToml.toml");
    EXPECT_STREQ("myToml.toml", Common::tomlFilesNameGet().toStdString().c_str());
    // 恢复默认值，避免影响其它用例
    Common::tomlFilesNameSet("tomlFilesName");
}

// ===================== getArch / getArchStore =====================
TEST_F(UT_Common, UT_Common_getArch)
{
    QString arch = Common::getArch();
    EXPECT_FALSE(arch.isEmpty());
}

TEST_F(UT_Common, UT_Common_getArchStore)
{
    QString store = Common::getArchStore();
    EXPECT_FALSE(store.isEmpty());
}

// ===================== executeClientCmd =====================
TEST_F(UT_Common, UT_Common_executeClientCmd_success)
{
    QByteArray out = Common::executeClientCmd("/bin/echo", QStringList() << "hello");
    EXPECT_STREQ("hello\n", out.data());
}

TEST_F(UT_Common, UT_Common_executeClientCmd_fail)
{
    // 不存在的命令 => 失败返回空
    QByteArray out = Common::executeClientCmd("nonexistent-cmd-12345", QStringList());
    EXPECT_TRUE(out.isEmpty());
}
