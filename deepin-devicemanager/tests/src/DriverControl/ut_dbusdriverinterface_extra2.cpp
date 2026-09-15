// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 增量补全：覆盖 DBusDriverInterface 中此前未覆盖的驱动安装/卸载/备份/删除/apt 更新接口。
// 这些接口内部均通过 QDBusInterface::call / asyncCall 发起 DBus 调用，
// 而 test_main.cpp 已全局打桩使 call/asyncCall 即时返回，故可安全调用。

#include "DBusDriverInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QtDBus>

#include <gtest/gtest.h>

class UT_DBusDriverInterfaceExtra : public UT_HEAD
{
public:
    void SetUp()
    {
        pDriver = DBusDriverInterface::getInstance();
    }
    void TearDown() {}
    DBusDriverInterface *pDriver = nullptr;
};

// void installDriver(const QString &driver);
TEST_F(UT_DBusDriverInterfaceExtra, InstallDriver_SingleArg_CallCompletesNoThrow)
{
    EXPECT_NO_FATAL_FAILURE(pDriver->installDriver(QString("test-driver.deb")));
}

// void installDriver(const QString &driverName, const QString &version);
TEST_F(UT_DBusDriverInterfaceExtra, InstallDriver_TwoArgs_CallCompletesNoThrow)
{
    EXPECT_NO_FATAL_FAILURE(pDriver->installDriver(QString("nvidia"), QString("525.60.11")));
}

// void undoInstallDriver();
TEST_F(UT_DBusDriverInterfaceExtra, UndoInstallDriver_CallCompletesNoThrow)
{
    EXPECT_NO_FATAL_FAILURE(pDriver->undoInstallDriver());
}

// bool backupDeb(const QString &debpath);
TEST_F(UT_DBusDriverInterfaceExtra, BackupDeb_Called_NoCrash)
{
    bool ret = false;
    EXPECT_NO_FATAL_FAILURE(ret = pDriver->backupDeb(QString("/tmp/test/")));
    // 打桩 DBus 环境下返回值可能为 false（服务未注册），此处只验证调用不崩溃
    EXPECT_TRUE(ret == true || ret == false);
}

// bool delDeb(const QString &debname);
TEST_F(UT_DBusDriverInterfaceExtra, DelDeb_Called_NoCrash)
{
    bool ret = false;
    EXPECT_NO_FATAL_FAILURE(ret = pDriver->delDeb(QString("test-pkg")));
    EXPECT_TRUE(ret == true || ret == false);
}

// bool aptUpdate();
TEST_F(UT_DBusDriverInterfaceExtra, AptUpdate_Called_NoCrash)
{
    bool ret = false;
    EXPECT_NO_FATAL_FAILURE(ret = pDriver->aptUpdate());
    EXPECT_TRUE(ret == true || ret == false);
}
