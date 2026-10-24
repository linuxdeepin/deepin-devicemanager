// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 用例计数声明（self-check-structural 验证此块）：
// | method       | level | factors                     | min | actual |
// |--------------|-------|-----------------------------|-----|--------|
// | getInfo      | mid   | complexity:3, branches:2    | 2   | 3      |
// | refreshInfo  | mid   | -                           | 2   | 2      |
// | DBusInterface | low  | -                           | 1   | 1      |
// | init         | low   | -                           | 1   | 2（连接失败/正常） |
// ─── 生成后填入 actual 列，低于 min 即违规 ───
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]
// 3. 每个等价类的边界值显式覆盖: [x]
// 4. 同质 ≥ 3 组用 TEST_P: [x]（无同质多组输入）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（源码无显式 throw）
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]（Qt 内置类走项目 stub.h，不用 gMock）
//
// 分支清单（来源：DBusInterface::getInfo(const QString &key, QString &info)）：
// B1: reply.isValid() == true  → info = reply.value(); return true
// B2: reply.isValid() == false → QProcess 调 gdbus 回退 → return false
// 分支清单（来源：DBusInterface::refreshInfo()）：
// B3: mp_Iface->asyncCall("refreshInfo") 触发一次异步调用
// 用例映射：
// - GetInfo_ValidReply_StoresPayloadAndReturnsTrue          → B1
// - GetInfo_InvalidReply_RunsGdbusFallbackAndReturnsFalse   → B2
// - GetInfo_InvalidReply_PreservesExistingInfo              → B2（强异常安全）
// - RefreshInfo_AsyncCall_CalledWithMethodName              → B3
// - RefreshInfo_RepeatedCall_RemainsUsable                  → B3
// - GetInstance_Singleton_ReturnsSameInstance               → ctor+init
// - Init_SystemBusUnavailable_StillCreatesInterface         → ctor+init 失败分支
//
// 打桩说明：
// 1) QDBusReply<QString>::isValid 是头文件内联函数，运行时替换其入口对调用点
//    无效（历史用例失效的根因）。Qt6 中 QDBusInterface::call/asyncCall 也均为
//    内联转发，实际离线落点为 QDBusAbstractInterface 私有成员 doCall/doAsyncCall，
//    故通过 addr_pri 获取私有成员地址打桩，返回手工构造的 ReplyMessage/
//    ErrorMessage，让 isValid 分支自然成立/不成立。
// 2) QProcess 启动行为打桩阻断真实 gdbus 子进程。测试不依赖真实 DBus/网络环境。

#include "DBusInterface.h"
#include "addr_pri.h"
#include "stub.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QProcess>

#include <gtest/gtest.h>

// 访问 DBusInterface 私有成员 mp_Iface（验证 init 失败路径仍创建接口对象）
ACCESS_PRIVATE_FIELD(DBusInterface, QDBusInterface *, mp_Iface)

// ── QDBusAbstractInterface 私有 doCall/doAsyncCall 地址（Qt6 call/asyncCall 内联转发至此）──
ACCESS_PRIVATE_FUN(QDBusAbstractInterface, QDBusMessage (QDBus::CallMode, const QString &, const QVariant *, size_t), doCall)
ACCESS_PRIVATE_FUN(QDBusAbstractInterface, QDBusPendingCall (const QString &, const QVariant *, size_t), doAsyncCall)

// 合法应答：ReplyMessage + QString 载荷 → QDBusReply::isValid() == true
static QDBusMessage ut_iface_docall_reply(QDBusAbstractInterface * /*self*/, QDBus::CallMode /*mode*/,
                                          const QString & /*method*/, const QVariant * /*args*/, size_t /*num*/)
{
    QDBusMessage request = QDBusMessage::createMethodCall(
        QStringLiteral("org.deepin.DeviceInfo"), QStringLiteral("/org/deepin/DeviceInfo"),
        QString(), QStringLiteral("getInfo"));
    request.setArguments(QList<QVariant>{QStringLiteral("payload-from-dbus")});
    return request.createReply(request.arguments());
}

// 错误应答：ErrorMessage → QDBusReply::isValid() == false
static QDBusMessage ut_iface_docall_error(QDBusAbstractInterface * /*self*/, QDBus::CallMode /*mode*/,
                                          const QString & /*method*/, const QVariant * /*args*/, size_t /*num*/)
{
    QDBusMessage request = QDBusMessage::createMethodCall(
        QStringLiteral("org.deepin.DeviceInfo"), QStringLiteral("/org/deepin/DeviceInfo"),
        QString(), QStringLiteral("getInfo"));
    return request.createErrorReply(QDBusError::ServiceUnknown,
                                    QStringLiteral("unit-test: service unknown"));
}

// QProcess 启动打桩：阻断真实 gdbus 子进程
static void ut_qprocess_start_nop(QProcess * /*self*/, const QString & /*program*/,
                                  const QStringList & /*arguments*/, QIODevice::OpenMode)
{
}

// systemBus 打桩：返回空连接名的无效连接（init 的连接失败分支）
static QDBusConnection ut_systembus_invalid()
{
    return QDBusConnection(QStringLiteral(""));
}

// doAsyncCall 打桩：记录调用的方法名
static int s_async_calls = 0;
static QString s_async_method;
static QDBusPendingCall ut_iface_doasync(QDBusAbstractInterface * /*self*/, const QString &method,
                                         const QVariant * /*args*/, size_t /*num*/)
{
    ++s_async_calls;
    s_async_method = method;
    return QDBusPendingCall::fromError(QDBusError(QDBusError::AccessDenied, QStringLiteral("unit-test")));
}

class UT_DBusInterface : public ::testing::Test
{
protected:
    void SetUp() override
    {
        s_async_calls = 0;
        s_async_method.clear();
    }
    void TearDown() override { }
};

// B1: 应答有效 → info 存储载荷，返回 true
TEST_F(UT_DBusInterface, GetInfo_ValidReply_StoresPayloadAndReturnsTrue)
{
    // Arrange
    DBusInterface *iface = DBusInterface::getInstance();
    ASSERT_NE(iface, nullptr);
    Stub stub;
    stub.set(get_private_fun::QDBusAbstractInterfacedoCall(), ut_iface_docall_reply);

    // Act
    QString info;
    const bool ok = iface->getInfo(QStringLiteral("hwinfo"), info);

    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(info, QStringLiteral("payload-from-dbus"));
}

// B2: 应答无效 → gdbus 回退（QProcess 已打桩），返回 false
TEST_F(UT_DBusInterface, GetInfo_InvalidReply_RunsGdbusFallbackAndReturnsFalse)
{
    // Arrange
    DBusInterface *iface = DBusInterface::getInstance();
    ASSERT_NE(iface, nullptr);
    Stub stubCall;
    stubCall.set(get_private_fun::QDBusAbstractInterfacedoCall(), ut_iface_docall_error);
    Stub stubProc;
    stubProc.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))
                      ADDR(QProcess, start)),
                 ut_qprocess_start_nop);

    // Act
    QString info;
    const bool ok = iface->getInfo(QStringLiteral("hwinfo"), info);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(info, QString());
}

// B2 负面强异常安全：失败路径不得破坏既有 info 内容
TEST_F(UT_DBusInterface, GetInfo_InvalidReply_PreservesExistingInfo)
{
    // Arrange
    DBusInterface *iface = DBusInterface::getInstance();
    ASSERT_NE(iface, nullptr);
    Stub stubCall;
    stubCall.set(get_private_fun::QDBusAbstractInterfacedoCall(), ut_iface_docall_error);
    Stub stubProc;
    stubProc.set(((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))
                      ADDR(QProcess, start)),
                 ut_qprocess_start_nop);
    QString info = QStringLiteral("keep-me");

    // Act
    const bool ok = iface->getInfo(QStringLiteral("lshw"), info);

    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(info, QStringLiteral("keep-me"));
}

// B3: refreshInfo 触发 asyncCall 且方法名为 "refreshInfo"
TEST_F(UT_DBusInterface, RefreshInfo_AsyncCall_CalledWithMethodName)
{
    // Arrange
    DBusInterface *iface = DBusInterface::getInstance();
    ASSERT_NE(iface, nullptr);
    Stub stub;
    stub.set(get_private_fun::QDBusAbstractInterfacedoAsyncCall(), ut_iface_doasync);

    // Act
    iface->refreshInfo();

    // Assert
    EXPECT_EQ(s_async_calls, 1);
    EXPECT_EQ(s_async_method, QStringLiteral("refreshInfo"));
}

// B3: refreshInfo 重复调用不崩溃，asyncCall 再次触发
TEST_F(UT_DBusInterface, RefreshInfo_RepeatedCall_RemainsUsable)
{
    // Arrange
    DBusInterface *iface = DBusInterface::getInstance();
    ASSERT_NE(iface, nullptr);
    Stub stub;
    stub.set(get_private_fun::QDBusAbstractInterfacedoAsyncCall(), ut_iface_doasync);
    iface->refreshInfo();
    const int callsAfterFirst = s_async_calls;

    // Act
    EXPECT_NO_THROW(iface->refreshInfo());

    // Assert
    EXPECT_EQ(callsAfterFirst, 1);
    EXPECT_EQ(s_async_calls, 2);
}

// 构造/初始化：getInstance 为进程级单例，重复获取同一实例
TEST_F(UT_DBusInterface, GetInstance_Singleton_ReturnsSameInstance)
{
    // Arrange
    DBusInterface *first = DBusInterface::getInstance();

    // Act
    DBusInterface *second = DBusInterface::getInstance();

    // Assert
    EXPECT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

// init 失败路径：systemBus 连接不可用时仍创建接口对象（防御式初始化）
TEST_F(UT_DBusInterface, Init_SystemBusUnavailable_StillCreatesInterface)
{
    // Arrange
    Stub stubBus;
    stubBus.set(ADDR(QDBusConnection, systemBus), ut_systembus_invalid);

    // Act
    DBusInterface local;

    // Assert：构造即完成 init，无总线也不崩溃，接口对象仍被创建
    QDBusInterface *iface = access_private_field::DBusInterfacemp_Iface(local);
    EXPECT_NE(iface, nullptr);
    EXPECT_NE(&local, DBusInterface::getInstance());
}
