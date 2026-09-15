// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 覆盖率缺口补全：EDIDParser::hexToBin 此前未被任何用例调用，函数覆盖率为 0。
// 本文件补全该函数的用例，覆盖合法十六进制输入与边界输入。

#include "EDIDParser.h"

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

// hexToBin 内部依次调用 hexToDec 与 decTobin，最终结果按 4 位对齐补零。
TEST(UT_EDIDParser_HexToBin, HexToBin_TwoFF_ReturnsEightOnes)
{
    EDIDParser parser;
    EXPECT_EQ(parser.hexToBin("FF"), "11111111");
}

TEST(UT_EDIDParser_HexToBin, HexToBin_SingleZero_ReturnsFourZeros)
{
    EDIDParser parser;
    EXPECT_EQ(parser.hexToBin("0"), "0000");
}

TEST(UT_EDIDParser_HexToBin, HexToBin_SingleOne_ReturnsPaddedOne)
{
    EDIDParser parser;
    EXPECT_EQ(parser.hexToBin("1"), "0001");
}

TEST(UT_EDIDParser_HexToBin, HexToBin_SingleA_ReturnsBinary1010)
{
    EDIDParser parser;
    EXPECT_EQ(parser.hexToBin("A"), "1010");
}

TEST(UT_EDIDParser_HexToBin, HexToBin_EmptyInput_ReturnsFourZeros)
{
    EDIDParser parser;
    EXPECT_EQ(parser.hexToBin(""), "0000");
}
