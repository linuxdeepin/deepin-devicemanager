// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 覆盖 DBusEnableInterface 各业务方法。测试环境 DBus isValid=false,走失败/else 分支。

#include "DBusEnableInterface.h"
#include "ut_Head.h"

#include <QString>

#include <gtest/gtest.h>

class UT_DBusEnableInterface : public UT_HEAD
{
};

TEST_F(UT_DBusEnableInterface, getRemoveInfo)
{
    QString info;
    EXPECT_NO_FATAL_FAILURE(DBusEnableInterface::getInstance()->getRemoveInfo(info));
}

TEST_F(UT_DBusEnableInterface, getAuthorizedInfo)
{
    QString lst;
    EXPECT_NO_FATAL_FAILURE(DBusEnableInterface::getInstance()->getAuthorizedInfo(lst));
}

TEST_F(UT_DBusEnableInterface, isDeviceEnabled)
{
    EXPECT_NO_FATAL_FAILURE(DBusEnableInterface::getInstance()->isDeviceEnabled("unique_id"));
}

TEST_F(UT_DBusEnableInterface, enable)
{
    EXPECT_NO_FATAL_FAILURE(
        DBusEnableInterface::getInstance()->enable("mouse", "name", "/sys/path", "value", true));
}

TEST_F(UT_DBusEnableInterface, enablePrinter)
{
    EXPECT_NO_FATAL_FAILURE(
        DBusEnableInterface::getInstance()->enablePrinter("printer", "name", "/sys/path", false));
}

TEST_F(UT_DBusEnableInterface, enableKeyboard)
{
    EXPECT_NO_FATAL_FAILURE(DBusEnableInterface::getInstance()->enableKeyboard(
        "0x1234", "0x5678", "keyboard", "name", "/sys/path", "value", true, "drv"));
}
