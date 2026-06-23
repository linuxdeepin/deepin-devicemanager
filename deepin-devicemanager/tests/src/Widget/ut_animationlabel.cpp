// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 AnimationLabel 的纯逻辑方法(setImageList/setcurrentIcon/currentIcon)。

#include "AnimationLabel.h"
#include "ut_Head.h"

#include <QStringList>

#include <gtest/gtest.h>

class UT_AnimationLabel : public UT_HEAD
{
public:
    void SetUp() override
    {
        l = new AnimationLabel;
    }
    void TearDown() override
    {
        delete l;
    }
    AnimationLabel *l = nullptr;
};

TEST_F(UT_AnimationLabel, setImageList_setcurrentIcon_normal)
{
    l->setImageList(QStringList() << "a"
                                  << "b"
                                  << "c");
    l->setcurrentIcon(1);
    EXPECT_EQ(l->currentIcon(), 1);
    l->setcurrentIcon(2);
    EXPECT_EQ(l->currentIcon(), 2);
}

TEST_F(UT_AnimationLabel, setcurrentIcon_outOfRange)
{
    l->setImageList(QStringList() << "a"
                                  << "b");
    l->setcurrentIcon(0);
    EXPECT_EQ(l->currentIcon(), 0);
    // 越界:不修改当前 index
    l->setcurrentIcon(-1);
    EXPECT_EQ(l->currentIcon(), 0);
    l->setcurrentIcon(99);
    EXPECT_EQ(l->currentIcon(), 0);
}
