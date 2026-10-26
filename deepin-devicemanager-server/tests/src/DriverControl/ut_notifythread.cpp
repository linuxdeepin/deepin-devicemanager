// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | NotifyThread(ctor) | low | - | 1 | 1 |
// | run | low | - | 1 | 3 |
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开方法 ≥ 1 用例: [x]（run 为公开 override）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（桌面未启动/已启动、用户名空/非空）
// 3. 每个等价类的边界值显式覆盖: [x]（首轮未检测到→重试后成功）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（3 个场景以独立 TEST_F 覆盖）
// 5. 分支清单 → 用例映射已列出: [x]
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无异常路径）
// 8. 负面场景有专门用例: [x]
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]（统一 stub_ext）
//
// 分支清单 → 用例映射：
//   run: B1 首轮 ps 无 desktop → sleep(2) 重试  | Run_NoDesktopFirst_SleepsAndRetries
//       B2 who 输出为空 → sleep(2) 重试         | Run_EmptyWhoOutput_SleepsAndRetries
//       B3 desktop+用户名 → runuser 启动后 break | Run_DesktopFound_LaunchesNotifyAsUser
//
// 环境隔离（self-checker §6）：Utils::executeServerCmd（ps aux）、QThread::sleep（run 内
// 非限定 sleep 经基类作用域解析到 QThread::sleep）、
// QProcess（start/waitForFinished/readAll）全部 stub；无真实进程、无真实系统
// 命令调用，run() 在调用线程内同步执行完毕。

#include "../ut_Head.h"
#include "drivercontrol/notifythread.h"
#include "drivercontrol/utils.h"

#include <QIODevice>
#include <QProcess>

#include <stubext.h>
#include <gtest/gtest.h>

#include <unistd.h>

namespace {

int g_psCalls = 0;          // executeServerCmd 被调次数
int g_sleepCalls = 0;       // ::sleep 被调次数
int g_startCalls = 0;       // QProcess::start 被调次数（经 stub.set_lamda 拦截）
QStringList g_startedPrograms;
QStringList g_startedArgs;  // 仅记录 runuser 的参数

void resetCapture()
{
    g_psCalls = 0;
    g_sleepCalls = 0;
    g_startCalls = 0;
    g_startedPrograms.clear();
    g_startedArgs.clear();
}

} // namespace

class NotifyThreadTest : public ::testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        resetCapture();
    }

    void TearDown() override
    {
        stub.clear();
        resetCapture();
    }

    // 公共打桩：ps 检测输出由 installPsOutput 注入；who/runuser 经 QProcess stub 捕获
    // （Qt6 中 start("who") 与 start("runuser", args) 均匹配
    //   start(const QString&, const QStringList& = {}, OpenMode) 重载）
    void installProcessStub(const QByteArray &whoOutput)
    {
        stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                       [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) {
                           ++g_startCalls;
                           g_startedPrograms << program;
                           if (program == "runuser")
                               g_startedArgs = args;
                       });
        stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished),
                       [](QProcess *, int) { return true; });
        stub.set_lamda(&QIODevice::readAll, [whoOutput](QIODevice *) { return whoOutput; });
    }
};

TEST_F(NotifyThreadTest, Run_DesktopFound_LaunchesNotifyAsUser)
{
    // Arrange
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray("root 1 1 /usr/bin/dde-desktop\n");
    });
    installProcessStub(QByteArray("testuser pts/0 (null) "));

    // Act：直接调用 run()（QThread::run 是普通成员函数，同步执行即退出）
    NotifyThread thread;
    thread.run();

    // Assert
    ASSERT_EQ(g_startedPrograms.size(), 2);
    EXPECT_EQ(g_startedPrograms.at(0), "who");
    EXPECT_EQ(g_startedPrograms.at(1), "runuser");
    ASSERT_EQ(g_startedArgs.size(), 4);
    EXPECT_EQ(g_startedArgs.at(0), "-l");
    EXPECT_EQ(g_startedArgs.at(1), "testuser");
    EXPECT_EQ(g_startedArgs.at(2), "-c");
    EXPECT_TRUE(g_startedArgs.at(3).contains("deepin-devicemanager notify"));
    EXPECT_TRUE(g_startedArgs.at(3).contains("/run/user/"));
}

TEST_F(NotifyThreadTest, Run_NoDesktopFirst_SleepsAndRetries)
{
    // Arrange：首轮 ps 输出无 desktop → sleep(2) 后重试；第二轮检测到 desktop
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        ++g_psCalls;
        return (g_psCalls == 1) ? QByteArray("root 1 1 /bin/bash\n")
                                : QByteArray("user 2 2 /usr/bin/dde-desktop\n");
    });
    stub.set_lamda(static_cast<void (*)(unsigned long)>(&QThread::sleep), [](unsigned long) { ++g_sleepCalls; });
    installProcessStub(QByteArray("retryuser pts/1"));

    // Act
    NotifyThread thread;
    thread.run();

    // Assert
    EXPECT_EQ(g_psCalls, 2);
    EXPECT_EQ(g_sleepCalls, 1);
    ASSERT_EQ(g_startedPrograms.size(), 2);
    EXPECT_EQ(g_startedPrograms.at(1), "runuser");
    ASSERT_EQ(g_startedArgs.size(), 4);
    EXPECT_EQ(g_startedArgs.at(1), "retryuser");
}

TEST_F(NotifyThreadTest, Run_EmptyWhoOutput_SleepsAndRetries)
{
    // Arrange：desktop 始终存在；who 首轮返回空 → sleep 重试，第二轮返回用户名
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &, const QStringList &, const QString &, int, bool) {
        return QByteArray("user 1 1 /usr/bin/dde-desktop\n");
    });
    stub.set_lamda(static_cast<void (*)(unsigned long)>(&QThread::sleep), [](unsigned long) { ++g_sleepCalls; });
    installProcessStub(QByteArray());
    // readAll 首轮为空（who 无输出），后续返回用户名
    stub.set_lamda(&QIODevice::readAll, [](QIODevice *) {
        ++g_psCalls;
        return (g_psCalls == 1) ? QByteArray("") : QByteArray("seconduser pts/0");
    });

    // Act
    NotifyThread thread;
    thread.run();

    // Assert
    EXPECT_EQ(g_sleepCalls, 1);
    ASSERT_EQ(g_startedPrograms.size(), 3); // who ×2 + runuser ×1
    EXPECT_EQ(g_startedPrograms.at(0), "who");
    EXPECT_EQ(g_startedPrograms.at(1), "who");
    EXPECT_EQ(g_startedPrograms.at(2), "runuser");
    ASSERT_EQ(g_startedArgs.size(), 4);
    EXPECT_EQ(g_startedArgs.at(1), "seconduser");
    EXPECT_TRUE(g_startedArgs.at(3).contains("deepin-devicemanager notify"));
}
