// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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

//  gtest的入口函数
int main(int argc, char **argv)
{

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
