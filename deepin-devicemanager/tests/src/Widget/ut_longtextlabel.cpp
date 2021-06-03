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
#include "../src/Widget/LongTextLabel.h"

#include "../ut_Head.h"
#include "../stub.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class LongTextLabel_UT : public UT_HEAD
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

int ut_LongTextLabel_indexIn()
{
    return 1;
}

TEST_F(LongTextLabel_UT, ut_paintEvent)
{
    Stub stub;
    stub.set(ADDR(QRegExp, indexIn), ut_LongTextLabel_indexIn);
    QPaintEvent paint(QRect(m_ltLabel->rect()));
    m_ltLabel->paintEvent(&paint);
}
