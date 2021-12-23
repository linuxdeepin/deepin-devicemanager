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
#include "GetDriverPathWidget.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDir>

#include <gtest/gtest.h>


class UT_GetDriverPathWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverPathWidget = new GetDriverPathWidget;
    }
    void TearDown()
    {
        delete m_GetDriverPathWidget;
    }

    GetDriverPathWidget *m_GetDriverPathWidget;
};

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_path)
{
    QString ret = m_GetDriverPathWidget->path();
    EXPECT_STREQ(QDir::homePath().toStdString().c_str(), ret.toStdString().c_str());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_includeSubdir_001)
{
    m_GetDriverPathWidget->mp_includeCheckBox->setCheckState(Qt::Checked);
    EXPECT_TRUE(m_GetDriverPathWidget->includeSubdir());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_includeSubdir_002)
{
    m_GetDriverPathWidget->mp_includeCheckBox->setCheckState(Qt::Unchecked);
    EXPECT_FALSE(m_GetDriverPathWidget->includeSubdir());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_updateTipLabelText)
{
    m_GetDriverPathWidget->updateTipLabelText("/home");
    EXPECT_STREQ("/home", m_GetDriverPathWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("/home", m_GetDriverPathWidget->mp_tipLabel->toolTip().toStdString().c_str());
}

TEST_F(UT_GetDriverPathWidget, UT_GetDriverPathWidget_clearTipLabelText)
{
    m_GetDriverPathWidget->clearTipLabelText();
    EXPECT_STREQ("", m_GetDriverPathWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("", m_GetDriverPathWidget->mp_tipLabel->toolTip().toStdString().c_str());
}
