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
#include "../src/ZeroMQ/ZMQBase.h"
#include "../ut_Head.h"
#include <QCoreApplication>
#include <gtest/gtest.h>
#include "../stub.h"

class ZMQBase_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_zmqBase = new ZMQBase;
    }
    void TearDown()
    {
        delete m_zmqBase;
    }
    ZMQBase *m_zmqBase = nullptr;
};

TEST_F(ZMQBase_UT, ZMQBase_UT_init)
{
    void *context = nullptr;
    m_zmqBase->init(1, context);
}

TEST_F(ZMQBase_UT, ZMQBase_UT_disConnect)
{
    char *epoint = "a";
    ASSERT_FALSE(m_zmqBase->disConnect(epoint));
}

TEST_F(ZMQBase_UT, ZMQBase_UT_bind)
{
    char *epoint = "a";
    ASSERT_FALSE(m_zmqBase->bind(epoint));
}

TEST_F(ZMQBase_UT, ZMQBase_UT_sendMsg)
{
    char *msg = "msg";
    m_zmqBase->sendMsg(msg);
    m_zmqBase->sendMsg(1);
    m_zmqBase->sendMoreMsg(msg);
    m_zmqBase->sendMoreMsg(1);
}

TEST_F(ZMQBase_UT, ZMQBase_UT_recvMsg)
{
    m_zmqBase->recvMsg();
}

TEST_F(ZMQBase_UT, ZMQBase_UT_recvIntMsg)
{
    //    m_zmqBase->recvIntMsg();
}

TEST_F(ZMQBase_UT, ZMQBase_UT_more)
{
    m_zmqBase->more();
    m_zmqBase->socket();
}

TEST_F(ZMQBase_UT, ZMQBase_UT_setsockopt)
{
    void *value = nullptr;
    size_t len = 0;
    m_zmqBase->setsockopt(1, value, len);
}
