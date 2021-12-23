/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      jixiaomei <jixiaomei@uniontech.com>
* Maintainer:  jixiaomei <jixiaomei@uniontech.com>
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

#include "DriverWaitingWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>


class UT_DriverWaitingWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_DriverWaitingWidget = new DriverWaitingWidget("");
    }
    void TearDown()
    {
        delete m_DriverWaitingWidget;
    }

    DriverWaitingWidget *m_DriverWaitingWidget;
};

TEST_F(UT_DriverWaitingWidget, UT_WaitingWidget_setValue)
{
    m_DriverWaitingWidget->setValue(10);

    EXPECT_EQ(10, m_DriverWaitingWidget->mp_Progress->value());
}

TEST_F(UT_DriverWaitingWidget, UT_WaitingWidget_setText)
{
    m_DriverWaitingWidget->setText("10");
    EXPECT_STREQ("10", m_DriverWaitingWidget->mp_Label->text().toStdString().c_str());
}
