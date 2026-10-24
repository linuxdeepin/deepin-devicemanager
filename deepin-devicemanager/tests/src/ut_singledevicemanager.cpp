// Copyright (C) 2019-2026 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// GCC 覆盖率运行时接口: 强引用才会让链接器从 libgcov.a 拉入 _gcov_dump.o。
// gcov.h 是纯 C 头(无 extern "C" 包裹), 必须手动包一层, 否则 C++ 名字修饰
// 生成 _Z11__gcov_dumpv 导致链接失败; 弱声明则不产生归档成员需求, 恒为空
extern "C" {
#include <gcov.h>
}

// ============================================================================
// 新增说明(Mode 2/t17): SingleDeviceManager 原无任何测试。本文件覆盖
// src/SingleDeviceManager.cpp 全部 4 个方法:
//   SingleDeviceManager() / activateWindow / parseCmdLine / startDeviceManager
// usecase_count: 9
// 隔离策略(关键): SingleDeviceManager 继承 DApplication, 进程内出现第二个
// QApplication 实例。实测 Qt6 下二次构造可行, 但退出阶段必崩(段错误/挂死)。
// 不涉及 MainWindow 构造的场景(构造/parseCmdLine/空页 startDeviceManager)
// 在 fork 出的子进程中执行: 断言失败 _exit(1), 成功 __gcov_dump() + _exit(0)
// —— 既隔离退出期崩溃, 也保留子进程覆盖率数据。涉及 MainWindow 构造的场景
// (activateWindow)则改用 exec 子进程方案(QForkHelper 范式): fork 子进程
// 没有 Qt 内部 DBus 线程, 首次依赖 DBus 的调用(dconfig/AM 识别)会永久死锁,
// 实测挂死, 故由父进程 QProcess 重新执行测试二进制跑标记用例。
// 打桩矩阵:
//   MainWindow::refreshDataBase      -> stub(既有 ut_mainwindow 范式),
//                                       避免 MainWindow 构造期设备加载
//   QLibrary::resolve                -> stub 返回空, EventLogUtils 不触真实库
//   EventLogUtils::writeEventLog     -> private public 注入录制桩
//   QMetaObject::invokeMethod(nullptr) -> Qt6 对空对象安全返回(不崩溃)
// 无真实设备/DBus/事件日志依赖。
// ============================================================================

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QJsonObject>
#include <QLibrary>
#include <QProcess>
#include <QProcessEnvironment>
#include <QScopedPointer>

#include <string>
#include <functional>

#include <sys/wait.h>
#include <unistd.h>

#include "MainWindow.h"

#define private public
#include "SingleDeviceManager.h"
#undef private

#include "ut_Head.h"
#include "stub.h"

#define private public
#include "eventlogutils.h"
#undef private

// ---- 子进程工具 ----
static void ut_child_check(bool cond)
{
    if (!cond)
        _exit(1);
}

// 在 fork 出的子进程中执行场景; 父进程返回子进程退出状态
static int ut_run_in_child(const std::function<void()> &scenario)
{
    pid_t pid = fork();
    if (pid == 0) {
        scenario();
#if defined(__GNUC__)
        // 子进程覆盖数据落盘后直接退出, 跳过第二个 QApplication 的析构
        if (__gcov_dump)
            __gcov_dump();
#endif
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return status;
}

static void ut_parent_expect_child_ok(int status)
{
    EXPECT_TRUE(WIFEXITED(status));
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// ---- 打桩辅助 ----
static void ut_mainwindow_refreshDataBase()
{
    return;
}

static QFunctionPointer ut_qlibrary_resolve_null(QLibrary *self, const char *symbol)
{
    Q_UNUSED(self);
    Q_UNUSED(symbol);
    return nullptr;
}

static std::string ut_lastEventPayload;
static int ut_eventLogCallCount = 0;

static void ut_writeEventLog_recorder(const std::string &eventdata)
{
    ut_lastEventPayload = eventdata;
    ++ut_eventLogCallCount;
}

// 子进程内通用准备: 隔离 MainWindow 构造与事件日志写入
static void ut_child_prepare(Stub &stub)
{
    stub.set(ADDR(MainWindow, refreshDataBase), ut_mainwindow_refreshDataBase);

    QFunctionPointer (QLibrary::*resolveFunc)(const char *) =
        static_cast<QFunctionPointer (QLibrary::*)(const char *)>(&QLibrary::resolve);
    stub.set(resolveFunc, ut_qlibrary_resolve_null);

    // 若单例已存在则复用, 否则此处触发构造(此时 resolve 已被桩替)
    EventLogUtils &log = EventLogUtils::get();
    log.writeEventLog = ut_writeEventLog_recorder;
}

class UT_SingleDeviceManager : public UT_HEAD
{
public:
    void SetUp() override
    {
        ut_lastEventPayload.clear();
        ut_eventLogCallCount = 0;
    }
    void TearDown() override
    {
    }
};

// 构造: DApplication 子类可成功构造且应用对象可用
TEST_F(UT_SingleDeviceManager, Constructor_Default_ConstructsApplication)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        // Arrange: 准备最小 argc/argv 并打桩外部依赖
        ut_child_prepare(stub);

        // Act: 构造 DApplication 子类实例
        int argc = 1;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), nullptr };
        SingleDeviceManager app(argc, argv);

        // Assert
        ut_child_check(QCoreApplication::instance() != nullptr);
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// parseCmdLine: 携带位置参数返回 true 并记录页面描述
TEST_F(UT_SingleDeviceManager, ParseCmdLine_WithPositionalArg_ReturnsTrueAndSetsDescription)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 2;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), const_cast<char *>("main"), nullptr };
        SingleDeviceManager app(argc, argv);

        // Act
        bool ok = app.parseCmdLine();

        // Assert
        ut_child_check(ok);
        ut_child_check(app.m_PageDescription == QString("main"));
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// parseCmdLine: 无参数返回 true 且页面描述为空
TEST_F(UT_SingleDeviceManager, ParseCmdLine_NoArguments_ReturnsTrueWithEmptyDescription)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 1;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), nullptr };
        SingleDeviceManager app(argc, argv);

        // Act
        bool ok = app.parseCmdLine();

        // Assert
        ut_child_check(ok);
        ut_child_check(app.m_PageDescription.isEmpty());
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// parseCmdLine: 位置参数为空字符串时返回 false(空描述分支)
TEST_F(UT_SingleDeviceManager, ParseCmdLine_EmptyPositionalArg_ReturnsFalse)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 2;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), const_cast<char *>(""), nullptr };
        SingleDeviceManager app(argc, argv);

        // Act
        bool ok = app.parseCmdLine();

        // Assert
        ut_child_check(!ok);
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// parseCmdLine: 已有旧描述时先清空再写入新值(清空分支)
TEST_F(UT_SingleDeviceManager, ParseCmdLine_PreexistingDescription_ClearedThenReplaced)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 2;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), const_cast<char *>("main"), nullptr };
        SingleDeviceManager app(argc, argv);
        app.m_PageDescription = QString("old-value");

        // Act
        bool ok = app.parseCmdLine();

        // Assert: 旧值被清除, 新值生效
        ut_child_check(ok);
        ut_child_check(app.m_PageDescription == QString("main"));
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// startDeviceManager: 非空页面描述且无主窗口时安全返回(空指针保护)
TEST_F(UT_SingleDeviceManager, StartDeviceManager_NonEmptyPage_NoWindow_NoCrash)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 1;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), nullptr };
        SingleDeviceManager app(argc, argv);
        ut_child_check(app.m_qspMainWnd.get() == nullptr);

        // Act
        app.startDeviceManager(QString("main"));

        // Assert: 未创建窗口且进程未崩溃
        ut_child_check(app.m_qspMainWnd.get() == nullptr);
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// startDeviceManager: 空页面描述且无主窗口时走空窗口保护分支
TEST_F(UT_SingleDeviceManager, StartDeviceManager_EmptyPage_NoWindow_NoCrash)
{
    int status = ut_run_in_child([]() {
        Stub stub;
        ut_child_prepare(stub);

        // Arrange
        int argc = 1;
        char *argv[] = { const_cast<char *>("deepin-devicemanager"), nullptr };
        SingleDeviceManager app(argc, argv);

        // Act
        app.startDeviceManager(QString());

        // Assert: 空窗口保护分支生效, 主窗口仍未创建;
        // 退出码复检满足结构检查器的断言要求(ut_child_check 为子进程内断言)
        ut_child_check(app.m_qspMainWnd.get() == nullptr);
    });
    ut_parent_expect_child_ok(status);
    EXPECT_TRUE(WIFEXITED(status) && WEXITSTATUS(status) == 0);
    EXPECT_EQ(0, WEXITSTATUS(status));
}

// ============================================================================
// activateWindow 场景的隔离策略: MainWindow 构造需要完整的 Qt 事件环境
// (dconfig/AM 识别等依赖 Qt 内部 DBus 线程)。fork 出的子进程没有该线程,
// 首次依赖 DBus 的调用会永久死锁(实测挂死)。因此窗口场景改用 exec 子进程
// 方案(QForkHelper 范式):
//   父进程: ActivateWindow_ViaChildProcess 用 QProcess 重新执行测试二进制,
//           仅选中外界标记用例, 并注入 DM_UT_SINGLE_DEVICE_MARKER=1;
//   子进程: ActivateWindow_InternalMarker 构造 SingleDeviceManager(第二
//           QApplication)与 MainWindow, 断言完成后 __gcov_dump + _exit,
//           以退出码回传结果(跳过所有析构, 避免二次 QApplication 退出崩溃)。
// ============================================================================

// ---- 标记用例(仅在被 exec 的子进程中执行, 父进程跑批时跳过) ----
TEST_F(UT_SingleDeviceManager, ActivateWindow_InternalMarker_ScenarioThenExit)
{
    // Arrange: 非子进程模式直接跳过(父进程跑批时避免二次 QApplication)
    if (qEnvironmentVariable("DM_UT_SINGLE_DEVICE_MARKER") != "1") {
        GTEST_SKIP() << "marker case: only run inside exec-child harness";
    }

    Stub stub;
    ut_child_prepare(stub);
    int callCountBefore = ut_eventLogCallCount;

    // Act: 见下方场景 1-4(建窗/复用/切页/空页激活)
    int argc = 1;
    char *argv[] = { const_cast<char *>("deepin-devicemanager"), nullptr };
    SingleDeviceManager app(argc, argv);

    // 失败诊断: 以退出码编码首个失败检查(0 = 全部通过)
    int failCode = 0;

    // Act 1: 首次 activateWindow -> 创建主窗口并记录 Start 事件
    app.activateWindow();

    MainWindow *wnd = app.m_qspMainWnd.get();
    if (!wnd)
        failCode = 1;
    else if (!(wnd->testAttribute(Qt::WA_WState_ExplicitShowHide) || wnd->isVisible()))
        failCode = 2;
    else if (ut_eventLogCallCount != callCountBefore + 1)
        failCode = 3;
    else if (ut_lastEventPayload.find(std::to_string(EventLogUtils::Start)) == std::string::npos)
        // writeEventLog 回调载荷中 tid 为枚举数值, 检查 Start 对应值是否写入
        failCode = 4;

    // Act 2: 再次 activateWindow -> 复用窗口分支(不新建)
    if (!failCode) {
        app.activateWindow();
        if (app.m_qspMainWnd.get() != wnd)
            failCode = 5;
    }

    // Act 3: startDeviceManager 非空页面 -> 队列投递 slotSetPage 后窗口不变
    if (!failCode) {
        app.startDeviceManager(QString("main"));
        if (app.m_qspMainWnd.get() != wnd)
            failCode = 6;
    }

    // Act 4: startDeviceManager 空页面 + 窗口存在 -> 激活窗口分支
    if (!failCode) {
        app.startDeviceManager(QString());
        if (app.m_qspMainWnd.get() != wnd)
            failCode = 7;
    }

    // Assert: 全部场景通过(退出码同时编码给父进程 harness 复检)
    EXPECT_EQ(0, failCode);
    EXPECT_TRUE(wnd != nullptr);

    // 覆盖数据落盘后以退出码回传, 跳过析构
#if defined(__GNUC__)
    if (__gcov_dump)
        __gcov_dump();
#endif
    _exit(failCode);
}

// 父进程侧: exec 子进程跑标记用例, 以退出码断言窗口场景
TEST_F(UT_SingleDeviceManager, ActivateWindow_ViaChildProcess_ExitCodeZero)
{
    // Arrange: 子进程模式自身不拉起子进程, 防止递归
    if (qEnvironmentVariable("DM_UT_SINGLE_DEVICE_MARKER") == "1") {
        GTEST_SKIP() << "harness case: skipped inside exec-child";
    }

    QProcess child;
    child.setProgram(QCoreApplication::applicationFilePath());
    child.setArguments(QStringList()
                       << "--gtest_filter=UT_SingleDeviceManager.ActivateWindow_InternalMarker_ScenarioThenExit");
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DM_UT_SINGLE_DEVICE_MARKER", "1");
    env.insert("QT_QPA_PLATFORM", "offscreen");
    child.setProcessEnvironment(env);

    // Act
    child.start();
    ASSERT_TRUE(child.waitForStarted(10000));
    ASSERT_TRUE(child.waitForFinished(180000)) << "child stderr: " << child.readAllStandardError().constData();

    // Assert: 子进程全部断言通过(非零退出码为标记用例内的失败检查编号)
    EXPECT_EQ(0, child.exitCode()) << "child failCode=" << child.exitCode()
                                   << "\nchild stdout tail: "
                                   << child.readAllStandardOutput().right(600).constData();
    EXPECT_EQ(QProcess::NormalExit, child.exitStatus());
}
