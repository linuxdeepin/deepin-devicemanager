// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <DApplication>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
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

    ::testing::InitGoogleTest(&argc, argv);
    auto c = RUN_ALL_TESTS();

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif
    //Q_UNUSED(c);
    return c;
}
