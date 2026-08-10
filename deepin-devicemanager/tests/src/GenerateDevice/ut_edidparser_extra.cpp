// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 EDIDParser::hexToBin 纯逻辑函数（十六进制转二进制）。

#include "EDIDParser.h"

#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>

#include <gtest/gtest.h>

class UT_EDIDParserHex : public UT_HEAD
{
public:
    void SetUp()
    {
        m_EDIDParser = new EDIDParser;
    }
    void TearDown()
    {
        delete m_EDIDParser;
    }
    EDIDParser *m_EDIDParser = nullptr;
};

// QString hexToBin(QString strHex)
TEST_F(UT_EDIDParserHex, HexToBin_ValidHex_ReturnsNonEmptyBinary)
{
    QString bin = m_EDIDParser->hexToBin("FF");
    EXPECT_FALSE(bin.isEmpty());
}

TEST_F(UT_EDIDParserHex, HexToBin_SmallValue_ReturnsNonEmptyBinary)
{
    QString bin = m_EDIDParser->hexToBin("1");
    EXPECT_FALSE(bin.isEmpty());
}

TEST_F(UT_EDIDParserHex, HexToBin_Zero_ReturnsNonEmptyBinary)
{
    QString bin = m_EDIDParser->hexToBin("0");
    EXPECT_FALSE(bin.isEmpty());
}
