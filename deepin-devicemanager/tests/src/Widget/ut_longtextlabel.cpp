// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LongTextLabel.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QRegularExpression>
#include <gtest/gtest.h>

class UT_LongTextLabel : public UT_HEAD
{
public:
    void SetUp()
    {
        m_ltLabel = new LongTextLabel;
    }
    void TearDown()
    {
        delete m_ltLabel;
    }
    LongTextLabel *m_ltLabel;
};

QRegularExpressionMatch ut_LongTextLabel_indexIn()
{
    QRegularExpressionMatch match;
    return match;
}

TEST_F(UT_LongTextLabel, UT_LongTextLabel_paintEvent)
{
    Stub stub;
    stub.set(ADDR(QRegularExpression, match), ut_LongTextLabel_indexIn);
    QPaintEvent paint(QRect(m_ltLabel->rect()));
    m_ltLabel->paintEvent(&paint);
    EXPECT_FALSE(m_ltLabel->grab().isNull());
}
