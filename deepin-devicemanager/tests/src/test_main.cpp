// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <DApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusError>
#include <QVariant>
#include "stub.h"
#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

DCORE_USE_NAMESPACE

// 全局桩:QDBusInterface::call/asyncCall 即时返回,避免真实 DBus 服务运行时
// (setTimeout 较长)的同步/异步调用阻塞测试。桩对象生命周期与进程相同。
static QDBusMessage ut_dbus_call(const QString &,
                                 const QVariant &, const QVariant &, const QVariant &, const QVariant &,
                                 const QVariant &, const QVariant &, const QVariant &, const QVariant &)
{
    return QDBusMessage().createReply(QVariant(true));
}
static QDBusPendingCall ut_dbus_asyncCall(const QString &,
                                          const QVariant &, const QVariant &, const QVariant &, const QVariant &,
                                          const QVariant &, const QVariant &, const QVariant &, const QVariant &)
{
    return QDBusPendingCall::fromCompletedCall(QDBusMessage().createReply(QVariant(true)));
}

// ── 进程级总线防火墙(打在 QDBusConnection::call/asyncCall 连接层)──
// 服务端(deepin-deviceinfo / deepin-devicecontrol)有 20+ 个 DBus 方法在入口
// 处调用 polkit checkAuthorizationSync(..., AllowUserInteraction), 测试进程
// (uid != 0)的任何一次真实系统总线调用都会在真机上弹出鉴权对话框并阻塞。
// Qt6 中 QDBusInterface::call/asyncCall 的全部重载(含模板)经私有 doCall 汇入
// QDBusAbstractInterface::callWithArgumentList, 最终落到公开的
// QDBusConnection::call(message, mode, timeout)/asyncCall(message, timeout)
// (已用 objdump 验证 libQt6DBus 导出调用链)。在此拦载可保上层不变: 用例内
// 对 callWithArgumentList/QDBusInterface::call 的打桩仍正常生效(在更上层
// 已被拦截), 仅真正出网的消息被替换为 ServiceUnknown 错误应答, 走产品失败
// 分支 —— 既不弹窗也不依赖真实服务。
#include <cstdio>

// 审计: 记录所有被拦截的出网消息, 便于定位仍在触碰受限服务(弹窗源)的调用点
static void utFirewallLog(const char *kind, const QDBusMessage &message)
{
    fprintf(stderr, "[UT-FIREWALL] %s dest=%s iface=%s member=%s path=%s\n",
            kind, qPrintable(message.service()), qPrintable(message.interface()),
            qPrintable(message.member()), qPrintable(message.path()));
    fflush(stderr);
}

static QDBusMessage ut_bus_call_off(QDBusConnection * /*self*/, const QDBusMessage &message,
                                    QDBus::CallMode /*mode*/, int /*timeout*/)
{
    utFirewallLog("call", message);
    return message.createErrorReply(QDBusError::ServiceUnknown,
                                    QStringLiteral("unit-test: bus disabled by global firewall"));
}

static QDBusPendingCall ut_bus_async_off(QDBusConnection * /*self*/,
                                         const QDBusMessage &message, int /*timeout*/)
{
    utFirewallLog("asyncCall", message);
    return QDBusPendingCall::fromError(QDBusError(QDBusError::ServiceUnknown,
                                                  QStringLiteral("unit-test: bus disabled by global firewall")));
}

static bool ut_bus_send_off(QDBusConnection * /*self*/, const QDBusMessage &message)
{
    utFirewallLog("send", message);
    return false;
}

#include <signal.h>
#include <stdio.h>

// 退出阶段崩溃时强制刷新 gcov 覆盖率数据:
// 测试进程在用例全部跑完后的 Qt 清理阶段可能 SEGV(QCoreApplication
// teardown),没有该处理器的崩溃会跳过 gcov 的 atexit 刷新,导致整轮
// 运行的 .gcda 全部丢失(表现为 lcov 统计出 0% 覆盖率)。
extern "C" void __gcov_dump(void);
static void utCrashHandler(int sig)
{
    fprintf(stderr, "\n=== Crash detected (signal %d), flushing coverage data ===\n", sig);
    __gcov_dump();
    _exit(128 + sig);
}
static void setupUtCrashHandler()
{
    signal(SIGSEGV, utCrashHandler);
    signal(SIGABRT, utCrashHandler);
    signal(SIGFPE, utCrashHandler);
}

//  gtest的入口函数
int main(int argc, char **argv)
{
    setupUtCrashHandler();

    // 子进程封口: gdbus 回退等 QProcess 子进程继承此环境变量后, 连接系统总线
    // 直接失败(死路径), 确保任何绕过 Qt 层防火墙的调用点也无法触碰真实系统
    // 服务(否则 org.deepin.DeviceInfo.getInfo 为鉴权方法, 会触发 polkit 弹窗)。
    qputenv("DBUS_SYSTEM_BUS_ADDRESS", "unix:path=/nonexistent-ut-no-bus");

    qputenv("QT_QPA_PLATFORM", "offscreen");
    QApplication a(argc, argv);

    // 持久桩(进程生命周期内有效),让所有 DBus call/asyncCall 即时返回
    Stub *stub = new Stub;
    QDBusMessage (QDBusInterface::*pc)(const QString &, const QVariant &, const QVariant &,
        const QVariant &, const QVariant &, const QVariant &, const QVariant &,
        const QVariant &, const QVariant &) = &QDBusInterface::call;
    stub->set(pc, ut_dbus_call);
    QDBusPendingCall (QDBusInterface::*pa)(const QString &, const QVariant &, const QVariant &,
        const QVariant &, const QVariant &, const QVariant &, const QVariant &,
        const QVariant &, const QVariant &) = &QDBusInterface::asyncCall;
    stub->set(pa, ut_dbus_asyncCall);

    // 总线防火墙(连接层): 所有真实出网调用在此替换为错误应答, 防止 polkit 弹窗
    QDBusMessage (QDBusConnection::*pbc)(const QDBusMessage &, QDBus::CallMode, int) const =
        &QDBusConnection::call;
    stub->set(pbc, ut_bus_call_off);
    QDBusPendingCall (QDBusConnection::*pba)(const QDBusMessage &, int) const =
        &QDBusConnection::asyncCall;
    stub->set(pba, ut_bus_async_off);
    bool (QDBusConnection::*pbs)(const QDBusMessage &) const = &QDBusConnection::send;
    stub->set(pbs, ut_bus_send_off);

    ::testing::InitGoogleTest(&argc, argv);
    auto c = RUN_ALL_TESTS();

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif
    //Q_UNUSED(c);
    return c;
}
