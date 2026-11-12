// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DriverScanner 的构造 + setDriverList(run 是线程,跳过)。

#include "DriverScanner.h"
#include "MacroDefinition.h"
#include "ut_Head.h"

#include <QList>

#include <gtest/gtest.h>

class UT_DriverScanner : public UT_HEAD
{
public:
    void SetUp() override
    {
        s = new DriverScanner;
    }
    void TearDown() override
    {
        delete s;
    }
    DriverScanner *s = nullptr;
};

TEST_F(UT_DriverScanner, ctor)
{
    EXPECT_NE(s, nullptr);
}

TEST_F(UT_DriverScanner, setDriverList_empty)
{
    QList<DriverInfo *> lst;
    EXPECT_NO_FATAL_FAILURE(s->setDriverList(lst));
}
