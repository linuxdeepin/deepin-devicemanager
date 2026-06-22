// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusTouchPad.h"
#include <QDBusInterface>
#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

class UT_DBusTouchPad : public UT_HEAD
{
};

// 真实环境: sessionBus 是否有目标服务不确定,只保证调用不崩(覆盖 if/else 两条路径)
TEST_F(UT_DBusTouchPad, isExists_real)
{
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->isExists());
}

TEST_F(UT_DBusTouchPad, setEnable_real)
{
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->setEnable(true));
}

TEST_F(UT_DBusTouchPad, getEnable_real)
{
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->getEnable());
}

// 桩 isValid()=true -> 覆盖 if 分支(property/call 调用,无服务不崩)
static bool ut_QDBusInterface_isValid_true()
{
    return true;
}

TEST_F(UT_DBusTouchPad, isExists_valid)
{
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_QDBusInterface_isValid_true);
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->isExists());
}

TEST_F(UT_DBusTouchPad, setEnable_valid)
{
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_QDBusInterface_isValid_true);
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->setEnable(false));
}

TEST_F(UT_DBusTouchPad, getEnable_valid)
{
    Stub stub;
    stub.set(ADDR(QDBusInterface, isValid), ut_QDBusInterface_isValid_true);
    EXPECT_NO_FATAL_FAILURE(DBusTouchPad::instance()->getEnable());
}
