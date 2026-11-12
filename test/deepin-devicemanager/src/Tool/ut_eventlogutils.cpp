// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// ============================================================================
// 重写说明(Mode 2/t17): 原文件 3 个用例均为 EXPECT_NO_FATAL_FAILURE 单断言,
// 判定为断言空洞(SOLE_NO_FATAL/EMPTY_ASSERT)。本文件重写为强断言闭环:
// usecase_count: 4
// 覆盖方法: EventLogUtils() / get() / writeLogs
// 打桩矩阵:
//   writeEventLog 函数指针  -> #define private public 注入录制桩,
//                              捕获 payload 做round-trip断言, 不触真实事件日志
//   无 QLibrary/DBus/文件/网络真实调用。
// ============================================================================

#include <gtest/gtest.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <string>

// 访问私有函数指针成员 writeEventLog 以注入录制桩
#define private public
#include "eventlogutils.h"
#undef private

#include "ut_Head.h"
#include "stub.h"

// ---- 录制桩: 捕获 writeEventLog 收到的 payload ----
static std::string ut_lastEventPayload;
static int ut_eventLogCallCount = 0;

static void ut_writeEventLog_recorder(const std::string &eventdata)
{
    ut_lastEventPayload = eventdata;
    ++ut_eventLogCallCount;
}

class UT_EventLogUtils : public UT_HEAD
{
public:
    void SetUp() override
    {
        // Arrange: 重置录制状态并保存原始函数指针
        ut_lastEventPayload.clear();
        ut_eventLogCallCount = 0;
        m_utils = &EventLogUtils::get();
        m_savedWriteEventLog = m_utils->writeEventLog;
    }

    void TearDown() override
    {
        // 恢复原始函数指针, 避免影响其它用例
        m_utils->writeEventLog = m_savedWriteEventLog;
    }

    EventLogUtils *m_utils = nullptr;
    void (*m_savedWriteEventLog)(const std::string &) = nullptr;
};

// get: 单例地址稳定且非空
TEST_F(UT_EventLogUtils, Get_CalledTwice_ReturnsSameSingleton)
{
    // Arrange: SetUp 已重置录制桩, 单例尚未创建
    // Act
    EventLogUtils &first = EventLogUtils::get();
    EventLogUtils &second = EventLogUtils::get();

    // Assert
    EXPECT_EQ(&first, &second);
    EXPECT_EQ(m_utils, &first);
}

// writeLogs: 函数指针为空时直接返回, 不崩溃也不调用录制桩
TEST_F(UT_EventLogUtils, WriteLogs_NullFunctionPointer_EarlyReturnWithoutCall)
{
    // Arrange: 置空函数指针
    m_utils->writeEventLog = nullptr;
    QJsonObject obj;
    obj["tid"] = EventLogUtils::Start;
    obj["key"] = "value";

    // Act
    m_utils->writeLogs(obj);

    // Assert: 未触发任何写入
    EXPECT_EQ(0, ut_eventLogCallCount);
    EXPECT_TRUE(ut_lastEventPayload.empty());
}

// writeLogs: 注入录制桩后以 Compact JSON 形式回调, payload 可还原
TEST_F(UT_EventLogUtils, WriteLogs_ValidJson_CallsRecorderWithCompactPayload)
{
    // Arrange: 注入录制桩
    m_utils->writeEventLog = ut_writeEventLog_recorder;
    QJsonObject obj;
    obj["tid"] = EventLogUtils::Start;
    obj["key"] = "value";

    // Act
    m_utils->writeLogs(obj);

    // Assert: 回调发生且 payload 是合法 Compact JSON
    EXPECT_EQ(1, ut_eventLogCallCount);
    EXPECT_FALSE(ut_lastEventPayload.empty());
    EXPECT_EQ(std::string::npos, ut_lastEventPayload.find('\n'));

    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(ut_lastEventPayload));
    EXPECT_TRUE(doc.isObject());
    EXPECT_EQ(static_cast<int>(EventLogUtils::Start), doc.object()["tid"].toInt());
    EXPECT_EQ(QString("value"), doc.object()["key"].toString());
}

// writeLogs: 连续两次写入, 计数与最后一次 payload 均正确
TEST_F(UT_EventLogUtils, WriteLogs_Twice_RecordsBothCalls)
{
    // Arrange
    m_utils->writeEventLog = ut_writeEventLog_recorder;
    QJsonObject first;
    first["tid"] = EventLogUtils::Start;
    first["phase"] = "one";
    QJsonObject second;
    second["tid"] = EventLogUtils::Quit;
    second["phase"] = "two";

    // Act
    m_utils->writeLogs(first);
    m_utils->writeLogs(second);

    // Assert
    EXPECT_EQ(2, ut_eventLogCallCount);
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(ut_lastEventPayload));
    EXPECT_EQ(static_cast<int>(EventLogUtils::Quit), doc.object()["tid"].toInt());
    EXPECT_EQ(QString("two"), doc.object()["phase"].toString());
}
