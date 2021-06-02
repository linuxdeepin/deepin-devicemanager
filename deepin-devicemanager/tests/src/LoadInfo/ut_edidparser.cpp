/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../src/LoadInfo/EDIDParser.h"

#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class EDIDParser_UT : public UT_HEAD
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

TEST_F(EDIDParser_UT, EDIDParser_UT_setEdid)
{
    QString meg = "0x0f";
    m_EDIDParser->setEdid("f00ffffffffffff0", meg, "", true);
    m_EDIDParser->setEdid("00ffffffffffff00", meg, "", true);
}

TEST_F(EDIDParser_UT, EDIDParser_UT_vendor)
{
    m_EDIDParser->vendor();
    m_EDIDParser->releaseDate();
    m_EDIDParser->screenSize();
}

TEST_F(EDIDParser_UT, EDIDParser_UT_binToDec)
{
    QString strBin = "0xffff";
    m_EDIDParser->binToDec(strBin);
}

TEST_F(EDIDParser_UT, EDIDParser_UT_decToHex)
{
    QString strHex = "101";
    m_EDIDParser->decToHex(strHex);
}

TEST_F(EDIDParser_UT, EDIDParser_UT_hex2)
{
    unsigned char hex = 'B';
    m_EDIDParser->hex2(hex);
    unsigned char hex1 = 'b';
    m_EDIDParser->hex2(hex1);
}
