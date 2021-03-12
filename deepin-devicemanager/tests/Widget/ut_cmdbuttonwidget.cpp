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
#include "../src/Widget/CmdButtonWidget.h"

#include "../tests/ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class CmdButtonWidget_UT : public UT_HEAD
{
public:
    virtual void setup()
    {
    }
    void TearDown()
    {
        delete m_cmdBtnWidget;
    }
    CmdButtonWidget *m_cmdBtnWidget;
};

TEST_F(CmdButtonWidget_UT, ut_paintEvent)
{
    m_cmdBtnWidget = new CmdButtonWidget;
    QPaintEvent paint(QRect(m_cmdBtnWidget->rect()));
    m_cmdBtnWidget->paintEvent(&paint);
    m_cmdBtnWidget->expandCommandLinkClicked();
}
