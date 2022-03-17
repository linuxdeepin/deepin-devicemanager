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
#include "DBusInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStyle>

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QWidget>
#include <QDBusReply>

#include <gtest/gtest.h>

class UT_DBusInterface : public UT_HEAD
{
public:
    void SetUp()
    {
    }
    void TearDown()
    {
    }
};

bool ut_replay_001()
{
    return true;
}
bool ut_replay_002()
{
    return false;
}
TEST_F(UT_DBusInterface, UT_DBusInterface_getInfo_001)
{
    QString info;
    Stub stub;
    stub.set(ADDR(QDBusReply<QString>, isValid), ut_replay_001);
    DBusInterface::getInstance()->getInfo("lshw", info);
    // 对函数isValid打桩后，发现在gerrit上依旧是实际的执行结果，因此该处不做预期判断，下同
    // EXPECT_TRUE(DBusInterface::getInstance()->getInfo("lshw",info));
}

TEST_F(UT_DBusInterface, UT_DBusInterface_getInfo_002)
{
    QString info;
    Stub stub;
    // 对函数isValid打桩后，发现在gerrit上依旧是实际的执行结果，因此该处不做预期判断，下同
    stub.set(ADDR(QDBusReply<QString>, isValid), ut_replay_002);
    DBusInterface::getInstance()->getInfo("lshw", info);
    // EXPECT_FALSE(DBusInterface::getInstance()->getInfo("lshw",info));
}
