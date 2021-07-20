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
#include "ZmqOrder.h"
#include "zhelpers.h"
#include "zmq.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <gtest/gtest.h>
#include "../stub.h"

class ZmqOrder_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_zmqOrder = new ZmqOrder;
    }
    void TearDown()
    {
        delete m_zmqOrder;
    }
    ZmqOrder *m_zmqOrder = nullptr;
};

TEST_F(ZmqOrder_UT, ZmqOrder_UT_execDriverOrder)
{
    m_zmqOrder->execDriverOrder("/n");
}

TEST_F(ZmqOrder_UT, ZmqOrder_UT_execIfconfigOrder)
{
    m_zmqOrder->execIfconfigOrder("/n");
}
