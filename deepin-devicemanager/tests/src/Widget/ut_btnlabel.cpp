// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 BtnLabel/TipsLabel/TitleLabel 的构造 + setDesc。

#include "BtnLabel.h"
#include "ut_Head.h"

#include <QString>

#include <gtest/gtest.h>

class UT_BtnLabel : public UT_HEAD
{
public:
    void SetUp() override
    {
        b = new BtnLabel;
    }
    void TearDown() override
    {
        delete b;
    }
    BtnLabel *b = nullptr;
};

TEST_F(UT_BtnLabel, setDesc)
{
    EXPECT_NO_FATAL_FAILURE(b->setDesc("description text"));
    EXPECT_NO_FATAL_FAILURE(b->setDesc(""));
}

TEST_F(UT_BtnLabel, ctor_tips_title)
{
    TipsLabel *t = new TipsLabel;
    TitleLabel *ttl = new TitleLabel;
    EXPECT_NE(t, nullptr);
    EXPECT_NE(ttl, nullptr);
    delete t;
    delete ttl;
}
