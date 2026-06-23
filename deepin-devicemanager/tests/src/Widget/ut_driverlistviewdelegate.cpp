// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DriverListViewDelegate::sizeHint(固定行高)。paint 依赖 DStyle,offscreen 下跳过。

#include "DriverListViewDelegate.h"
#include "ut_Head.h"

#include <QStyleOptionViewItem>
#include <QModelIndex>

#include <gtest/gtest.h>

class UT_DriverListViewDelegate : public UT_HEAD
{
public:
    void SetUp() override
    {
        d = new DriverListViewDelegate;
    }
    void TearDown() override
    {
        delete d;
    }
    DriverListViewDelegate *d = nullptr;
};

TEST_F(UT_DriverListViewDelegate, sizeHint)
{
    QStyleOptionViewItem opt;
    QModelIndex idx;
    auto sz = d->sizeHint(opt, idx);
    EXPECT_GT(sz.height(), 0);
}
