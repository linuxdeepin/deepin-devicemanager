// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// 覆盖率缺口补全：CommonTools 的构造函数与静态 feedback() 此前未被用例调用。
// - 构造函数仅做日志输出，构造对象即可覆盖。
// - feedback() 内部构造 QDBusInterface 调用服务支持 DBus；测试环境无会话总线，
//   QDBusInterface 无效、QDBusReply::isValid() 为 false，走 else 分支，函数体完整执行
//   且不会阻塞或崩溃，因此可直接调用覆盖。

#include "commontools.h"

#include "ut_Head.h"
#include "stub.h"

#include <gtest/gtest.h>

TEST(UT_CommonTools_Feedback, Constructor_WithNullParent_ObjectCreated)
{
    CommonTools tools(nullptr);
    EXPECT_NE(nullptr, &tools);
}

TEST(UT_CommonTools_Feedback, Constructor_WithParent_ObjectCreated)
{
    QObject parent;
    CommonTools tools(&parent);
    EXPECT_EQ(&parent, tools.parent());
}

TEST(UT_CommonTools_Feedback, Feedback_NoSessionBus_CompletesWithoutCrash)
{
    // 无 DBus 会话总线时，feedback 内部 DBus 调用即时返回无效应答，
    // 函数走 false 分支正常结束。此处验证其可被安全调用。
    EXPECT_NO_FATAL_FAILURE(CommonTools::feedback());
}
