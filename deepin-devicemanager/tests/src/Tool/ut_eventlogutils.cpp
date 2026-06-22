// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QJsonObject>
#include <QJsonDocument>
#include <string>

// 访问私有函数指针成员 writeEventLog 以覆盖实际写入分支
#define private public
#include "eventlogutils.h"
#undef private

#include "ut_Head.h"

#include <gtest/gtest.h>

class UT_EventLogUtils : public UT_HEAD
{
};

// 单例构造: QLibrary resolve 失败时 init=nullptr 安全 return
TEST_F(UT_EventLogUtils, get_instance)
{
    EXPECT_NO_FATAL_FAILURE(EventLogUtils::get());
}

// writeLogs: 测试环境 writeEventLog 函数指针为空,走空指针保护分支
TEST_F(UT_EventLogUtils, writeLogs_nullptr)
{
    QJsonObject obj;
    obj["tid"] = EventLogUtils::Start;
    obj["key"] = "value";
    EXPECT_NO_FATAL_FAILURE(EventLogUtils::get().writeLogs(obj));
}

// writeLogs: 注入函数指针,覆盖实际 writeEventLog(...) 调用分支
static void ut_writeEventLog_func(const std::string &)
{
}

TEST_F(UT_EventLogUtils, writeLogs_withFunc)
{
    EventLogUtils &e = EventLogUtils::get();
    auto *old = e.writeEventLog;
    e.writeEventLog = ut_writeEventLog_func;

    QJsonObject obj;
    obj["tid"] = EventLogUtils::Start;
    obj["key"] = "value";
    EXPECT_NO_FATAL_FAILURE(e.writeLogs(obj));

    e.writeEventLog = old;
}
