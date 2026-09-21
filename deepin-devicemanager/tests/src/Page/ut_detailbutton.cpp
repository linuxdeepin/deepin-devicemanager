// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DetailButton / DetailSeperator / ScrollAreaWidget 的构造 + 行为。

#include "PageDetail.h"
#include "ut_Head.h"

#include <QPaintEvent>
#include <QPainter>
#include <QSizePolicy>

#include <gtest/gtest.h>

class UT_DetailButton : public UT_HEAD
{
public:
    void SetUp() override
    {
        btn = new DetailButton("More");
    }
    void TearDown() override
    {
        delete btn;
    }
    DetailButton *btn = nullptr;
};

TEST_F(UT_DetailButton, ctor)
{
    EXPECT_NE(btn, nullptr);
    EXPECT_EQ(btn->text().toStdString(), "More");
    EXPECT_EQ(btn->sizePolicy().horizontalPolicy(), QSizePolicy::Minimum);
    EXPECT_EQ(btn->sizePolicy().verticalPolicy(), QSizePolicy::Fixed);
}

TEST_F(UT_DetailButton, updateText_togglesFromMoreToCollapse)
{
    btn->updateText();
    EXPECT_EQ(btn->text().toStdString(), "Collapse");
}

TEST_F(UT_DetailButton, updateText_togglesBackToMore)
{
    btn->updateText();
    EXPECT_EQ(btn->text().toStdString(), "Collapse");
    btn->updateText();
    EXPECT_EQ(btn->text().toStdString(), "More");
}

TEST_F(UT_DetailButton, paintEvent)
{
    QPaintEvent paint(QRect(btn->rect()));
    EXPECT_NO_FATAL_FAILURE(btn->paintEvent(&paint));
    EXPECT_FALSE(btn->grab().isNull());
}

// ===================== DetailSeperator =====================

class UT_DetailSeperator : public UT_HEAD
{
public:
    void SetUp() override
    {
        sep = new DetailSeperator;
    }
    void TearDown() override
    {
        delete sep;
    }
    DetailSeperator *sep = nullptr;
};

TEST_F(UT_DetailSeperator, ctor)
{
    EXPECT_NE(sep, nullptr);
}

TEST_F(UT_DetailSeperator, paintEvent)
{
    QPaintEvent paint(QRect(sep->rect()));
    EXPECT_NO_FATAL_FAILURE(sep->paintEvent(&paint));
    EXPECT_FALSE(sep->grab().isNull());
}

// ===================== ScrollAreaWidget =====================

class UT_ScrollAreaWidget : public UT_HEAD
{
public:
    void SetUp() override
    {
        sa = new ScrollAreaWidget;
    }
    void TearDown() override
    {
        delete sa;
    }
    ScrollAreaWidget *sa = nullptr;
};

TEST_F(UT_ScrollAreaWidget, ctor)
{
    EXPECT_NE(sa, nullptr);
}

TEST_F(UT_ScrollAreaWidget, paintEvent)
{
    QPaintEvent paint(QRect(sa->rect()));
    EXPECT_NO_FATAL_FAILURE(sa->paintEvent(&paint));
    EXPECT_FALSE(sa->grab().isNull());
}
