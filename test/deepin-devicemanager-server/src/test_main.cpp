// SPDX-FileCopyrightText: 2019-2026 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
using namespace std;

#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include "gtest/gtest.h"
#include <QApplication>

// 退出阶段崩溃时强制刷新 gcov 覆盖率数据(见 deepin-devicemanager 客户端
// 测试 main 中的同名实现:崩溃若发生在 gcov atexit 刷新之前,会丢掉整轮
// 运行的 .gcda,lcov 统计出 0% 覆盖率)。
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
    ::testing::InitGoogleTest(&argc, argv);
    auto c = RUN_ALL_TESTS();
    //Q_UNUSED(c);
    return c;
}
