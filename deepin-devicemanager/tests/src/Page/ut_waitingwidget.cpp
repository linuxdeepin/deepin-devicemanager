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
#include "../src/Page/PageInfo.h"
#include "../src/Page/WaitingWidget.h"
#include "../src/DeviceManager/DeviceInput.h"
#include "../src/DeviceManager/DeviceInfo.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>
#include "../stub.h"

class WaitingWidget_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_waitingWidget = new WaitingWidget;
    }
    void TearDown()
    {
        delete m_waitingWidget;
    }
    WaitingWidget *m_waitingWidget = nullptr;
};

TEST_F(WaitingWidget_UT, WaitingWidget_UT_start)
{
    m_waitingWidget->start();
    m_waitingWidget->stop();
}
