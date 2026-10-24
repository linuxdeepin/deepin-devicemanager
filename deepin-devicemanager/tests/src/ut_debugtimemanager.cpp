// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 重写说明(Mode 2/t17): 原文件 3 个用例命名不规范(UT_001)且断言单薄。
// 本文件针对 src/DebugTimeManager.cpp 的 4 个方法全覆盖:
//   DebugTimeManager() / beginPointLinux / endPointLinux / clear
// usecase_count: 8
// 打桩矩阵:
//   gettimeofday(libc)      -> stub.set((void*)gettimeofday, ...) 固定时钟,
//                              保证 elapsed 计算确定性(方法体内部调用经 PLT 被拦截)
//   qCInfo 日志             -> qInstallMessageHandler 捕获 [GRABPOINT] 输出
//   私有成员 m_MapPoint     -> #define private public 直接断言内部状态
// 无真实设备/DBus/文件依赖。
// ============================================================================

#include <gtest/gtest.h>

#include <QJsonArray>
#include <QJsonObject>

#include <QDateTime>
#include <QLoggingCategory>
#include <QTimer>
#include <QtGlobal>

#include <dlfcn.h>
#include <sys/time.h>

#define private public
#define protected public
#include "DebugTimeManager.h"
#undef private
#undef protected

#include "ut_Head.h"
#include "stub.h"

// ---- 打桩辅助: 固定时钟 ----
static time_t ut_stub_tv_sec = 0;
static suseconds_t ut_stub_tv_usec = 0;

// 与 gettimeofday(struct timeval *, void *) 签名一致
static int ut_gettimeofday_stub(struct timeval *tv, void *tz)
{
    Q_UNUSED(tz);
    tv->tv_sec = ut_stub_tv_sec;
    tv->tv_usec = ut_stub_tv_usec;
    return 0;
}

// ---- 打桩辅助: 消息捕获 ----
static QStringList ut_capturedMessages;
static QtMessageHandler ut_originalHandler = nullptr;

static void ut_message_capture_handler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    ut_capturedMessages << msg;
    if (ut_originalHandler)
        ut_originalHandler(type, context, msg);
}

class UT_DebugTimeManager : public UT_HEAD
{
public:
    void SetUp() override
    {
        // Arrange: 重置桩时钟与日志捕获, 安装消息处理器;
        // gettimeofday 经 dlsym 取真实函数体地址打桩(直接取函数符号会因
        // IFUNC/PLT 解析地址不一致而拦截不到, 探针实测 dlsym 路径有效)
        ut_stub_tv_sec = 0;
        ut_stub_tv_usec = 0;
        ut_capturedMessages.clear();
        ut_originalHandler = qInstallMessageHandler(ut_message_capture_handler);
        stub.set(dlsym(RTLD_DEFAULT, "gettimeofday"), ut_gettimeofday_stub);
        m_manager = new DebugTimeManager();
    }

    void TearDown() override
    {
        qInstallMessageHandler(ut_originalHandler);
        ut_originalHandler = nullptr;
        delete m_manager;
        m_manager = nullptr;
    }

    DebugTimeManager *m_manager = nullptr;
    Stub stub;
};

// DebugTimeManager: 构造后映射表为空
TEST_F(UT_DebugTimeManager, Constructor_Default_MapIsEmpty)
{
    // Arrange: SetUp 中已构造 DebugTimeManager 实例并重置桩时钟
    ASSERT_NE(m_manager, nullptr);

    // Act: 直接读取构造后的映射表状态
    const int size = m_manager->m_MapPoint.size();

    // Assert: 实例有效且映射表为空
    EXPECT_EQ(0, size);
    EXPECT_TRUE(m_manager->m_MapPoint.isEmpty());
    EXPECT_EQ(0, m_manager->m_MapPoint.size());
}

// beginPointLinux: 新打点插入映射表, desc 与起始毫秒正确
TEST_F(UT_DebugTimeManager, BeginPointLinux_NewPoint_InsertsWithDescAndStartTime)
{
    // Arrange: 固定时钟 1000s + 500000us => 1000500 ms
    ut_stub_tv_sec = 1000;
    ut_stub_tv_usec = 500000;

    // Act
    m_manager->beginPointLinux("P1", "stage-one");

    // Assert
    EXPECT_TRUE(m_manager->m_MapPoint.contains("P1"));
    EXPECT_EQ(1, m_manager->m_MapPoint.size());
    EXPECT_EQ(QString("stage-one"), m_manager->m_MapPoint["P1"].desc);
    EXPECT_EQ(1000500, m_manager->m_MapPoint["P1"].time);
}

// beginPointLinux: 同名打点覆盖旧值而非新增
TEST_F(UT_DebugTimeManager, BeginPointLinux_SamePointTwice_OverwritesEntry)
{
    // Arrange: 两次不同时刻打点
    ut_stub_tv_sec = 1;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P1", "first");

    ut_stub_tv_sec = 2;
    ut_stub_tv_usec = 0;

    // Act
    m_manager->beginPointLinux("P1", "second");

    // Assert: 仍只有一个条目, 内容为第二次打点
    EXPECT_EQ(1, m_manager->m_MapPoint.size());
    EXPECT_EQ(QString("second"), m_manager->m_MapPoint["P1"].desc);
    EXPECT_EQ(2000, m_manager->m_MapPoint["P1"].time);
}

// endPointLinux: 已知打点计算 elapsed 毫秒并输出 [GRABPOINT] 日志
TEST_F(UT_DebugTimeManager, EndPointLinux_KnownPoint_LogsElapsedDelta)
{
    // Arrange: begin @1000.000s, end @1502.500s => delta 502500 ms
    ut_stub_tv_sec = 1000;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P1", "run");
    int logCountBefore = ut_capturedMessages.size();

    ut_stub_tv_sec = 1502;
    ut_stub_tv_usec = 500000;

    // Act
    m_manager->endPointLinux("P1");

    // Assert: 内部状态与日志均体现 502500ms
    EXPECT_EQ(502500, m_manager->m_MapPoint["P1"].time);
    bool foundGrabpoint = false;
    for (const QString &msg : ut_capturedMessages.mid(logCountBefore)) {
        if (msg.contains("[GRABPOINT] P1") && msg.contains("time=502500ms"))
            foundGrabpoint = true;
    }
    EXPECT_TRUE(foundGrabpoint);
}

// endPointLinux: 指定子打点时扣除子打点已记录时间
TEST_F(UT_DebugTimeManager, EndPointLinux_WithSubPoint_SubtractsSubTime)
{
    // Arrange: P1 begin @1000s, P2 begin @2000s, end P1 @2500s with sub P2
    // delta = 1500000ms, 再减 P2.time(2000000ms) => -500000ms
    ut_stub_tv_sec = 1000;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P1", "outer");

    ut_stub_tv_sec = 2000;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P2", "inner");

    ut_stub_tv_sec = 2500;
    ut_stub_tv_usec = 0;

    // Act
    m_manager->endPointLinux("P1", "P2");

    // Assert: delta(1500000ms) 减去子打点未结束时的绝对时间(2000000ms)
    EXPECT_EQ(-500000, m_manager->m_MapPoint["P1"].time);
    // 复核: 结果与 [GRABPOINT] 日志一致, 且异常负值已暴露
    bool foundGrabpoint = false;
    for (const QString &msg : ut_capturedMessages) {
        if (msg.contains("[GRABPOINT] P1") && msg.contains("time=-500000ms"))
            foundGrabpoint = true;
    }
    EXPECT_TRUE(foundGrabpoint);
}

// endPointLinux: 子打点不存在时仅计算自身 delta
TEST_F(UT_DebugTimeManager, EndPointLinux_SubPointMissing_UsesOwnDeltaOnly)
{
    // Arrange
    ut_stub_tv_sec = 1000;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P1", "outer");

    ut_stub_tv_sec = 1500;
    ut_stub_tv_usec = 250000;

    // Act: "PZ" 从未 begin
    m_manager->endPointLinux("P1", "PZ");

    // Assert: 仅自身 delta(1500.25s - 1000s = 500250ms), 不受缺失子打点影响
    EXPECT_EQ(500250, m_manager->m_MapPoint["P1"].time);
    // 复核: [GRABPOINT] 日志与内部状态一致
    bool foundGrabpoint = false;
    for (const QString &msg : ut_capturedMessages) {
        if (msg.contains("[GRABPOINT] P1") && msg.contains("time=500250ms"))
            foundGrabpoint = true;
    }
    EXPECT_TRUE(foundGrabpoint);
}

// endPointLinux: 未知打点不崩溃、不产生 [GRABPOINT] 日志
TEST_F(UT_DebugTimeManager, EndPointLinux_UnknownPoint_DoesNothing)
{
    // Arrange: 空表 + 捕获基线
    int logCountBefore = ut_capturedMessages.size();

    // Act
    m_manager->endPointLinux("NotExist");

    // Assert: 无新增 [GRABPOINT] 日志, 表仍为空
    for (const QString &msg : ut_capturedMessages.mid(logCountBefore)) {
        EXPECT_FALSE(msg.contains("[GRABPOINT]"));
    }
    EXPECT_TRUE(m_manager->m_MapPoint.isEmpty());
}

// clear: 清空所有打点
TEST_F(UT_DebugTimeManager, Clear_AfterBegin_EmptiesMap)
{
    // Arrange
    ut_stub_tv_sec = 10;
    ut_stub_tv_usec = 0;
    m_manager->beginPointLinux("P1", "a");
    m_manager->beginPointLinux("P2", "b");
    EXPECT_EQ(2, m_manager->m_MapPoint.size());

    // Act
    m_manager->clear();

    // Assert
    EXPECT_TRUE(m_manager->m_MapPoint.isEmpty());
}
