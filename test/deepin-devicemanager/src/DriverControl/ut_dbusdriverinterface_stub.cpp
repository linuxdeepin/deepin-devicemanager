// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// DBusDriverInterface 业务方法用例(任务06):
// 按调用点精确实例化打桩 QDBusInterface::call / asyncCall(Qt6 内联模板, comdat 合并),
// 断言返回值与发出的 DBus 方法名; 槽转发用 QSignalSpy 断言信号。
// 不连接真实 org.deepin.DeviceControl 服务。

#include "DBusDriverInterface.h"
#include "ut_Head.h"
#include "stub.h"

#include <QDBusError>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusUnixFileDescriptor>
#include <QSignalSpy>
#include <QTemporaryFile>

#include <gtest/gtest.h>

// ---------------- stub 状态 ----------------
static QString ut_dbus_lastMethod;   // 最近一次被桩到的 DBus 方法名
static bool ut_dbus_replyValid = true;
static bool ut_dbus_replyValue = true;
static int ut_slotCallFinished_calls = 0;

static void ut_reset_dbus_stubs()
{
    ut_dbus_lastMethod.clear();
    ut_dbus_replyValid = true;
    ut_dbus_replyValue = true;
    ut_slotCallFinished_calls = 0;
}

static QDBusMessage ut_make_dbus_reply()
{
    QDBusMessage msg;
    if (ut_dbus_replyValid)
        return msg.createReply(ut_dbus_replyValue);
    return msg.createErrorReply(QDBusError::ServiceUnknown, QStringLiteral("ut: no service"));
}

// ---------------- call 桩(带 this 首参) ----------------
static QDBusMessage ut_dbus_call0(void *, const QString &method)
{
    ut_dbus_lastMethod = method;
    return ut_make_dbus_reply();
}
static QDBusMessage ut_dbus_call1v(void *, const QString &method, QVariant &&)
{
    ut_dbus_lastMethod = method;
    return ut_make_dbus_reply();
}
static QDBusMessage ut_dbus_call1s(void *, const QString &method, const QString &)
{
    ut_dbus_lastMethod = method;
    return ut_make_dbus_reply();
}
static QDBusMessage ut_dbus_call2vs(void *, const QString &method, QVariant &&, const QString &)
{
    ut_dbus_lastMethod = method;
    return ut_make_dbus_reply();
}

// ---------------- asyncCall 桩 ----------------
static QDBusPendingCall ut_dbus_acall0(void *, const QString &method)
{
    ut_dbus_lastMethod = method;
    return QDBusPendingCall::fromCompletedCall(ut_make_dbus_reply());
}
static QDBusPendingCall ut_dbus_acall1s(void *, const QString &method, const QString &)
{
    ut_dbus_lastMethod = method;
    return QDBusPendingCall::fromCompletedCall(ut_make_dbus_reply());
}
static QDBusPendingCall ut_dbus_acall2ss(void *, const QString &method, const QString &, const QString &)
{
    ut_dbus_lastMethod = method;
    return QDBusPendingCall::fromCompletedCall(ut_make_dbus_reply());
}
static QDBusPendingCall ut_dbus_acall2vs(void *, const QString &method, QVariant &&, const QString &)
{
    ut_dbus_lastMethod = method;
    return QDBusPendingCall::fromCompletedCall(ut_make_dbus_reply());
}

static void ut_stub_slotCallFinished(QDBusPendingCallWatcher *)
{
    ++ut_slotCallFinished_calls;
}

// 桩目标: 按调用点实参类型选取的精确实例化(类为 QDBusAbstractInterface;
// lvalue QString 实参在调用点推导为 Args=const QString&, 桩目标参数必须一致)
struct UtDbusCallAddrs
{
    QDBusMessage (QDBusAbstractInterface::*call0)(const QString &) = &QDBusAbstractInterface::call;
    QDBusMessage (QDBusAbstractInterface::*call1v)(const QString &, QVariant &&) =
        static_cast<QDBusMessage (QDBusAbstractInterface::*)(const QString &, QVariant &&)>(&QDBusAbstractInterface::call);
    QDBusMessage (QDBusAbstractInterface::*call1s)(const QString &, const QString &) =
        static_cast<QDBusMessage (QDBusAbstractInterface::*)(const QString &, const QString &)>(&QDBusAbstractInterface::call);
    QDBusMessage (QDBusAbstractInterface::*call2vs)(const QString &, QVariant &&, const QString &) =
        static_cast<QDBusMessage (QDBusAbstractInterface::*)(const QString &, QVariant &&, const QString &)>(&QDBusAbstractInterface::call);

    QDBusPendingCall (QDBusAbstractInterface::*acall0)(const QString &) = &QDBusAbstractInterface::asyncCall;
    QDBusPendingCall (QDBusAbstractInterface::*acall1s)(const QString &, const QString &) =
        static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QString &)>(&QDBusAbstractInterface::asyncCall);
    QDBusPendingCall (QDBusAbstractInterface::*acall2ss)(const QString &, const QString &, const QString &) =
        static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, const QString &, const QString &)>(&QDBusAbstractInterface::asyncCall);
    QDBusPendingCall (QDBusAbstractInterface::*acall2vs)(const QString &, QVariant &&, const QString &) =
        static_cast<QDBusPendingCall (QDBusAbstractInterface::*)(const QString &, QVariant &&, const QString &)>(&QDBusAbstractInterface::asyncCall);
};

static void ut_stub_all_dbus_calls(Stub &stub)
{
    UtDbusCallAddrs a;
    stub.set(a.call0, ut_dbus_call0);
    stub.set(a.call1v, ut_dbus_call1v);
    stub.set(a.call1s, ut_dbus_call1s);
    stub.set(a.call2vs, ut_dbus_call2vs);
    stub.set(a.acall0, ut_dbus_acall0);
    stub.set(a.acall1s, ut_dbus_acall1s);
    stub.set(a.acall2ss, ut_dbus_acall2ss);
    stub.set(a.acall2vs, ut_dbus_acall2vs);
}

class UT_DBusDriverInterfaceStub : public UT_HEAD
{
public:
    void SetUp() override
    {
        ut_reset_dbus_stubs();
        pDriver = DBusDriverInterface::getInstance();
        ASSERT_NE(pDriver, nullptr);
    }
    void TearDown() override { }

    DBusDriverInterface *pDriver = nullptr;
};

// ---------------- 同步 bool 方法 ----------------
TEST_F(UT_DBusDriverInterfaceStub, aptUpdate_replyTrue)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    EXPECT_TRUE(pDriver->aptUpdate());
    EXPECT_EQ(QStringLiteral("aptUpdate"), ut_dbus_lastMethod);
}

TEST_F(UT_DBusDriverInterfaceStub, delDeb_replyValue)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    ut_dbus_replyValue = true;
    EXPECT_TRUE(pDriver->delDeb("/tmp/ut-pkg/ut-pkg_1.0.deb"));
    EXPECT_EQ(QStringLiteral("delDeb"), ut_dbus_lastMethod);
    ut_dbus_replyValue = false;
    EXPECT_FALSE(pDriver->delDeb("/tmp/ut-pkg/ut-pkg_1.0.deb"));
}

TEST_F(UT_DBusDriverInterfaceStub, backupDebFd_replyValue)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    QDBusUnixFileDescriptor ufd;
    ut_dbus_replyValue = true;
    EXPECT_TRUE(pDriver->backupDebFd(ufd, "ut-pkg"));
    EXPECT_EQ(QStringLiteral("backupDebFd"), ut_dbus_lastMethod);
    ut_dbus_replyValue = false;
    EXPECT_FALSE(pDriver->backupDebFd(ufd, "ut-pkg"));
}

TEST_F(UT_DBusDriverInterfaceStub, isArchMatchedFd_replyTrue)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    QDBusUnixFileDescriptor ufd(tmpFile.handle());
    EXPECT_TRUE(pDriver->isArchMatchedFd(ufd));
    EXPECT_EQ(QStringLiteral("isArchMatchedFd"), ut_dbus_lastMethod);
}

TEST_F(UT_DBusDriverInterfaceStub, isDebValidFd_replyFalse)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    QDBusUnixFileDescriptor ufd;
    ut_dbus_replyValue = false;
    EXPECT_FALSE(pDriver->isDebValidFd(ufd));
    EXPECT_EQ(QStringLiteral("isDebValidFd"), ut_dbus_lastMethod);
}

TEST_F(UT_DBusDriverInterfaceStub, isArchMatchedFd_invalidReply_false)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    ut_dbus_replyValid = false;   // createErrorReply -> QDBusReply 无效 -> false
    QDBusUnixFileDescriptor ufd;
    EXPECT_FALSE(pDriver->isArchMatchedFd(ufd));
}

TEST_F(UT_DBusDriverInterfaceStub, undoInstallDriver_callName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    pDriver->undoInstallDriver();
    EXPECT_EQ(QStringLiteral("undoInstallDriver"), ut_dbus_lastMethod);
}

// ---------------- 异步方法: 方法名 + watcher 触发 slotCallFinished ----------------
TEST_F(UT_DBusDriverInterfaceStub, uninstallDriver_asyncName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_stub_slotCallFinished);
    pDriver->uninstallDriver("ut-pkg");
    EXPECT_EQ(QStringLiteral("unInstallDriver"), ut_dbus_lastMethod);
    QDBusPendingCallWatcher *cw = pDriver->findChild<QDBusPendingCallWatcher *>();
    ASSERT_NE(cw, nullptr);
    emit cw->finished(cw);
    EXPECT_EQ(1, ut_slotCallFinished_calls);
    cw->deleteLater();
}

TEST_F(UT_DBusDriverInterfaceStub, uninstallPrinter_asyncName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_stub_slotCallFinished);
    pDriver->uninstallPrinter("Canon", "iR-ADV");
    EXPECT_EQ(QStringLiteral("unInstallPrinter"), ut_dbus_lastMethod);
    QDBusPendingCallWatcher *cw = pDriver->findChild<QDBusPendingCallWatcher *>();
    ASSERT_NE(cw, nullptr);
    emit cw->finished(cw);
    EXPECT_EQ(1, ut_slotCallFinished_calls);
    cw->deleteLater();
}

TEST_F(UT_DBusDriverInterfaceStub, installDriver_twoArg_asyncName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_stub_slotCallFinished);
    pDriver->installDriver("ut-pkg", "1.0.0");
    EXPECT_EQ(QStringLiteral("installDriver"), ut_dbus_lastMethod);
    QDBusPendingCallWatcher *cw = pDriver->findChild<QDBusPendingCallWatcher *>();
    ASSERT_NE(cw, nullptr);
    emit cw->finished(cw);
    EXPECT_EQ(1, ut_slotCallFinished_calls);
    cw->deleteLater();
}

TEST_F(UT_DBusDriverInterfaceStub, installDriver_singleArg_asyncName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_stub_slotCallFinished);
    pDriver->installDriver(QStringLiteral("/tmp/ut-driver.deb"));
    EXPECT_EQ(QStringLiteral("installDriver"), ut_dbus_lastMethod);
    QDBusPendingCallWatcher *cw = pDriver->findChild<QDBusPendingCallWatcher *>();
    ASSERT_NE(cw, nullptr);
    emit cw->finished(cw);
    EXPECT_EQ(1, ut_slotCallFinished_calls);
    cw->deleteLater();
}

TEST_F(UT_DBusDriverInterfaceStub, installDriverFd_asyncName)
{
    Stub stub;
    ut_stub_all_dbus_calls(stub);
    stub.set(ADDR(DBusDriverInterface, slotCallFinished), ut_stub_slotCallFinished);
    QTemporaryFile tmpFile;
    ASSERT_TRUE(tmpFile.open());
    QDBusUnixFileDescriptor ufd(tmpFile.handle());
    pDriver->installDriverFd(ufd, "ut-driver.ko");
    EXPECT_EQ(QStringLiteral("installDriverFd"), ut_dbus_lastMethod);
    QDBusPendingCallWatcher *cw = pDriver->findChild<QDBusPendingCallWatcher *>();
    ASSERT_NE(cw, nullptr);
    emit cw->finished(cw);
    EXPECT_EQ(1, ut_slotCallFinished_calls);
    cw->deleteLater();
}

// ---------------- 槽转发: QSignalSpy 实断言 ----------------
TEST_F(UT_DBusDriverInterfaceStub, slotProcessEnd_success_emitsBoth)
{
    QSignalSpy spyChange(pDriver, &DBusDriverInterface::processChange);
    QSignalSpy spyEnd(pDriver, &DBusDriverInterface::processEnd);
    pDriver->slotProcessEnd(true, "ok");
    EXPECT_EQ(1, spyChange.count());
    EXPECT_EQ(100, spyChange.at(0).at(0).toInt());
    EXPECT_EQ(1, spyEnd.count());
    EXPECT_TRUE(spyEnd.at(0).at(0).toBool());
    EXPECT_EQ(QStringLiteral("ok"), spyEnd.at(0).at(1).toString());
}

TEST_F(UT_DBusDriverInterfaceStub, slotProcessEnd_failure_emitsEndOnly)
{
    QSignalSpy spyChange(pDriver, &DBusDriverInterface::processChange);
    QSignalSpy spyEnd(pDriver, &DBusDriverInterface::processEnd);
    pDriver->slotProcessEnd(false, "err");
    EXPECT_EQ(0, spyChange.count());
    EXPECT_EQ(1, spyEnd.count());
    EXPECT_FALSE(spyEnd.at(0).at(0).toBool());
    EXPECT_EQ(QStringLiteral("err"), spyEnd.at(0).at(1).toString());
}

TEST_F(UT_DBusDriverInterfaceStub, slotProcessChange_forwards)
{
    QSignalSpy spy(pDriver, &DBusDriverInterface::processChange);
    pDriver->slotProcessChange(50, "installing");
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(50, spy.at(0).at(0).toInt());
    EXPECT_EQ(QStringLiteral("installing"), spy.at(0).at(1).toString());
}

TEST_F(UT_DBusDriverInterfaceStub, slotDownloadProgressChanged_forwards)
{
    QSignalSpy spy(pDriver, &DBusDriverInterface::downloadProgressChanged);
    QStringList msgs{ "a.deb", "b.deb" };
    pDriver->slotDownloadProgressChanged(msgs);
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(msgs, spy.at(0).at(0).toStringList());
}

TEST_F(UT_DBusDriverInterfaceStub, slotDownloadFinished_forwards)
{
    QSignalSpy spy(pDriver, &DBusDriverInterface::downloadFinished);
    pDriver->slotDownloadFinished();
    EXPECT_EQ(1, spy.count());
}

TEST_F(UT_DBusDriverInterfaceStub, slotInstallProgressChanged_forwards)
{
    QSignalSpy spy(pDriver, &DBusDriverInterface::installProgressChanged);
    pDriver->slotInstallProgressChanged(75);
    EXPECT_EQ(1, spy.count());
    EXPECT_EQ(75, spy.at(0).at(0).toInt());
}

TEST_F(UT_DBusDriverInterfaceStub, slotInstallProgressFinished_forwards)
{
    QSignalSpy spy(pDriver, &DBusDriverInterface::installProgressFinished);
    pDriver->slotInstallProgressFinished(false, 1);
    EXPECT_EQ(1, spy.count());
    EXPECT_FALSE(spy.at(0).at(0).toBool());
    EXPECT_EQ(1, spy.at(0).at(1).toInt());
}
