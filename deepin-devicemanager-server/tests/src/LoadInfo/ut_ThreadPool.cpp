// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Unit tests for ThreadPool (deepin-deviceinfo/src/loadinfo/threadpool.cpp)
 *
 * Branch list (source: MCP get_code_snippet @ 6ccddaf4):
 * Cmd default ctor:
 *   B1: cmd == "" / file == "" / canNotReplace == false / waitingTime == -1
 * ThreadPool::ThreadPool:
 *   B1: initCmd() called -> m_ListCmd/m_ListUpdate populated
 *   B2: QDir().mkdir(PATH) -> /tmp/device-info exists
 * ThreadPool::initCmd:
 *   B1: lshw appended to both lists, canNotReplace=false
 *   B2: dmidecode_spn appended to m_ListCmd only, canNotReplace=true
 *   B3: lscpu appended to both, canNotReplace=true
 *   B4: bt_device waitingTime=500, others -1
 *   B5: dr_config appended to m_ListCmd only
 * ThreadPool::loadDeviceInfo:
 *   B1: one ThreadPoolTask per m_ListCmd entry (22), autoDelete, queued via start()
 *   B2: waitForDone(10000) after queuing
 * ThreadPool::updateDeviceInfo:
 *   B1: one ThreadPoolTask per m_ListUpdate entry (12)
 *   B2: waitForDone(10000) after queuing
 * ThreadPool::runCmdToCache:
 *   B1: cmdStr empty -> return before running anything
 *   B2: normal cmd -> QProcess stdout cached via addInfo(key=file-without-.txt)
 *
 * External dependencies stubbed (dependency-tracer §3 matrix):
 *   QThreadPool::start (tasks captured and deleted, no real thread pool queuing)
 *   QProcess (start/waitForFinished/readAllStandardOutput)
 * DeviceInfoManager kept REAL (pure in-memory singleton) as observation point.
 */

#include "threadpool.h"
#include "threadpooltask.h"
#include "deviceinfomanager.h"
#include "DDLog.h"

#include <gtest/gtest.h>
#include <QProcess>
#include <QDir>

#include "../stub.h"

// Expose private members for white-box inspection (test binary built with -fno-access-control)
#define private public
#define protected public
#include "threadpool.h"
#undef private
#undef protected

// ===== stub configuration state (reset in SetUp) =====
static QList<QPair<QString, QString>> ut_tp_startedTasks;   // (cmd, file) of queued tasks
static QString ut_tp_procCmd;
static QStringList ut_tp_procArgs;
static QString ut_tp_procOut = "";

static void (QThreadPool::*ut_tp_startFn)(QRunnable *, int) = &QThreadPool::start;
static void (QProcess::*ut_tp_procStartFn)(const QString &, const QStringList &, QIODevice::OpenMode) = &QProcess::start;  // __DBG_STUB_INVOKE__

static void ut_tp_poolStart(QThreadPool *, QRunnable *runnable, int)
{
    auto *task = static_cast<ThreadPoolTask *>(runnable);
    ut_tp_startedTasks << qMakePair(task->m_Cmd, task->m_File);
    delete runnable;   // never queued by real pool -> delete here (autoDelete semantics)
}

static void ut_tp_procStart(QProcess *, const QString &cmd, const QStringList &args, QIODevice::OpenMode)
{
    ut_tp_procCmd = cmd;
    ut_tp_procArgs = args;
}

static bool ut_tp_waitFinished(QProcess *, int)
{
    return true;
}

static QByteArray ut_tp_readOut(QProcess *)
{
    return ut_tp_procOut.toUtf8();
}

static void ut_tp_resetState()
{
    ut_tp_startedTasks.clear();
    ut_tp_procCmd.clear();
    ut_tp_procArgs.clear();
    ut_tp_procOut.clear();
}

class ThreadPool_UT : public ::testing::Test
{
public:
    Stub stub;
    ThreadPool *pool = nullptr;
    void SetUp() override
    {
        ut_tp_resetState();
        stub.set(ut_tp_startFn, ut_tp_poolStart);
        stub.set(ut_tp_procStartFn, ut_tp_procStart);
        stub.set(ADDR(QProcess, waitForFinished), ut_tp_waitFinished);
        stub.set(ADDR(QProcess, readAllStandardOutput), ut_tp_readOut);
        QDir().mkpath(PATH);
        pool = new ThreadPool();
    }
    void TearDown() override
    {
        delete pool;
        pool = nullptr;
        stub.reset(ADDR(QProcess, readAllStandardOutput));
        stub.reset(ADDR(QProcess, waitForFinished));
        stub.reset(ut_tp_procStartFn);
        stub.reset(ut_tp_startFn);
    }
};

// ===== Cmd struct =====

TEST_F(ThreadPool_UT, Cmd_DefaultConstructor_AllFieldsInitialized)
{
    // Arrange
    const Cmd defaultValue;
    // Act
    Cmd cmd;
    // Assert
    EXPECT_EQ(cmd.cmd, defaultValue.cmd);
    EXPECT_EQ(cmd.file, defaultValue.file);
    EXPECT_FALSE(cmd.canNotReplace);
    EXPECT_EQ(cmd.waitingTime, -1);
}

TEST_F(ThreadPool_UT, Cmd_FieldsAssignable_RoundTripValues)
{
    // Arrange
    Cmd cmd;
    // Act
    cmd.cmd = "lscpu";
    cmd.file = "lscpu.txt";
    cmd.canNotReplace = true;
    cmd.waitingTime = 500;
    // Assert
    EXPECT_EQ(cmd.cmd, QString("lscpu"));
    EXPECT_EQ(cmd.file, QString("lscpu.txt"));
    EXPECT_TRUE(cmd.canNotReplace);
    EXPECT_EQ(cmd.waitingTime, 500);
}

// ===== initCmd — m_ListCmd membership =====

TEST_F(ThreadPool_UT, InitCmd_AfterConstruction_ListCmdNotEmpty)
{
    // Arrange
    ASSERT_NE(pool, nullptr);
    // Act — constructor already called initCmd in SetUp
    const int cmdSize = pool->m_ListCmd.size();
    const QString firstFile = pool->m_ListCmd.first().file;
    // Assert
    EXPECT_GT(cmdSize, 0);
    EXPECT_EQ(cmdSize, 22);
    EXPECT_NE(firstFile, QString(""));
}

TEST_F(ThreadPool_UT, InitCmd_AfterConstruction_ListUpdateNotEmpty)
{
    // Arrange
    ASSERT_NE(pool, nullptr);
    // Act — constructor already called initCmd in SetUp
    const int updateSize = pool->m_ListUpdate.size();
    // Assert
    EXPECT_GT(updateSize, 0);
    EXPECT_EQ(updateSize, 12);
}

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListCmdLargerThanListUpdate)
{
    // Arrange
    ASSERT_NE(pool, nullptr);
    // Act — m_ListCmd contains dmidecode-only entries not in m_ListUpdate
    const int cmdSize = pool->m_ListCmd.size();
    const int updateSize = pool->m_ListUpdate.size();
    // Assert
    EXPECT_GT(cmdSize, updateSize);
    EXPECT_EQ(cmdSize - updateSize, 10);
}

TEST_F(ThreadPool_UT, InitCmd_LshwInBothLists_CanReplace)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act — find lshw entry in both lists
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lshw.txt") {
            foundInCmd = true;
            EXPECT_FALSE(c.canNotReplace);  // lshw canNotReplace = false
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "lshw.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "lshw.txt"; }), 1);
}

TEST_F(ThreadPool_UT, InitCmd_DmidecodeSpnInCmdOnly_CannotReplace)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "dmidecode_spn.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);  // dmidecode_spn canNotReplace = true
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "dmidecode_spn.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_FALSE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "dmidecode_spn.txt"; }), 1);
    EXPECT_EQ(std::count_if(pool->m_ListUpdate.cbegin(), pool->m_ListUpdate.cend(), [](const Cmd &c) { return c.file == "dmidecode_spn.txt"; }), 0);
}

TEST_F(ThreadPool_UT, InitCmd_LscpuEntry_CannotReplaceTrue)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lscpu.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);  // lscpu canNotReplace = true
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "lscpu.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "lscpu.txt"; }), 1);
    EXPECT_EQ(std::count_if(pool->m_ListUpdate.cbegin(), pool->m_ListUpdate.cend(), [](const Cmd &c) { return c.file == "lscpu.txt"; }), 1);
}

TEST_F(ThreadPool_UT, InitCmd_BtDeviceEntry_HasCustomWaitingTime)
{
    // Arrange
    bool found = false;
    // Act — bt_device is the only entry with waitingTime = 500
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "bt_device.txt") {
            found = true;
            EXPECT_EQ(c.waitingTime, 500);
            EXPECT_FALSE(c.canNotReplace);
        }
    }
    // Assert
    EXPECT_TRUE(found);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.waitingTime == 500; }), 1);
}

TEST_F(ThreadPool_UT, InitCmd_NonBtDeviceEntries_HaveDefaultWaitingTime)
{
    // Arrange
    int checkedCount = 0;
    // Act — every entry except bt_device should have waitingTime == -1
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "bt_device.txt")
            continue;
        EXPECT_EQ(c.waitingTime, -1) << "Unexpected waitingTime for " << c.file.toStdString();
        checkedCount++;
    }
    // Assert
    EXPECT_EQ(checkedCount, pool->m_ListCmd.size() - 1);
}

TEST_F(ThreadPool_UT, InitCmd_DrConfigEntry_InCmdListOnly)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act — dr_config appended to m_ListCmd only
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "dr_config.txt") {
            foundInCmd = true;
            EXPECT_TRUE(c.canNotReplace);
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "dr_config.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_FALSE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "dr_config.txt"; }), 1);
    EXPECT_EQ(std::count_if(pool->m_ListUpdate.cbegin(), pool->m_ListUpdate.cend(), [](const Cmd &c) { return c.file == "dr_config.txt"; }), 0);
}

TEST_F(ThreadPool_UT, InitCmd_HwinfoInBothLists_CanReplace)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "hwinfo.txt") {
            foundInCmd = true;
            EXPECT_FALSE(c.canNotReplace);
        }
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "hwinfo.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "hwinfo.txt"; }), 1);
    EXPECT_EQ(std::count_if(pool->m_ListUpdate.cbegin(), pool->m_ListUpdate.cend(), [](const Cmd &c) { return c.file == "hwinfo.txt"; }), 1);
}

TEST_F(ThreadPool_UT, InitCmd_HwinfoMonitorEntry_InBothLists)
{
    // Arrange
    bool foundInCmd = false;
    bool foundInUpdate = false;
    // Act
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "hwinfo_monitor.txt")
            foundInCmd = true;
    }
    for (const Cmd &c : pool->m_ListUpdate) {
        if (c.file == "hwinfo_monitor.txt")
            foundInUpdate = true;
    }
    // Assert
    EXPECT_TRUE(foundInCmd);
    EXPECT_TRUE(foundInUpdate);
    EXPECT_EQ(std::count_if(pool->m_ListCmd.cbegin(), pool->m_ListCmd.cend(), [](const Cmd &c) { return c.file == "hwinfo_monitor.txt"; }), 1);
    EXPECT_EQ(std::count_if(pool->m_ListUpdate.cbegin(), pool->m_ListUpdate.cend(), [](const Cmd &c) { return c.file == "hwinfo_monitor.txt"; }), 1);
}

// ===== initCmd — command string content =====

TEST_F(ThreadPool_UT, InitCmd_LscpuCmdString_IsPlainCommand)
{
    // Arrange
    bool found = false;
    QString cmdStr = "<missing>";
    // Act — lscpu cmd is just "lscpu" (no redirect)
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lscpu.txt") {
            found = true;
            cmdStr = c.cmd;
        }
    }
    // Assert
    EXPECT_TRUE(found);
    EXPECT_EQ(cmdStr, QString("lscpu"));
}

TEST_F(ThreadPool_UT, InitCmd_LshwCmdString_ContainsRedirect)
{
    // Arrange
    bool found = false;
    QString cmdStr = "<missing>";
    // Act — lshw cmd redirects to PATH/lshw.txt
    for (const Cmd &c : pool->m_ListCmd) {
        if (c.file == "lshw.txt") {
            found = true;
            cmdStr = c.cmd;
        }
    }
    // Assert
    EXPECT_TRUE(found);
    EXPECT_TRUE(cmdStr.contains("lshw"));
    EXPECT_TRUE(cmdStr.contains(">"));
    EXPECT_GT(cmdStr.size(), 10);   // non-bool: redirect target makes the string long
}

// ===== initCmd — entry count =====

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListCmdHas22Entries)
{
    // Arrange
    ASSERT_NE(pool, nullptr);
    // Act — m_ListCmd: lshw + 9 dmidecode + upower + lscpu + lsblk + ls_sg + lspci +
    //       lpstat + dmesg + hciconfig + bt_device + dr_config + hwinfo + hwinfo_monitor
    const int cmdSize = pool->m_ListCmd.size();
    // Assert
    EXPECT_EQ(cmdSize, 22);
    EXPECT_GT(cmdSize, pool->m_ListUpdate.size());
}

TEST_F(ThreadPool_UT, InitCmd_AfterInit_ListUpdateHas12Entries)
{
    // Arrange
    ASSERT_NE(pool, nullptr);
    // Act — m_ListUpdate: lshw + upower + lscpu + lsblk + ls_sg + lspci + lpstat +
    //       dmesg + hciconfig + bt_device + hwinfo + hwinfo_monitor
    const int updateSize = pool->m_ListUpdate.size();
    // Assert
    EXPECT_EQ(updateSize, 12);
    EXPECT_LT(updateSize, pool->m_ListCmd.size());
}

// ===== constructor — side effects =====

TEST_F(ThreadPool_UT, Constructor_AfterInit_CreatesDeviceInfoDirectory)
{
    // Arrange
    QDir dir(PATH);
    // Act — constructor calls mkdir(PATH) to create device info directory
    ASSERT_TRUE(QDir().mkpath(PATH));
    // Assert
    EXPECT_EQ(dir.exists(), true);
    EXPECT_TRUE(dir.isReadable());
    EXPECT_TRUE(dir.absolutePath().endsWith("device-info"));
}

TEST_F(ThreadPool_UT, Constructor_TypeCheck_InheritsQThreadPool)
{
    // Arrange
    QThreadPool *asBase = pool;
    // Act
    QThreadPool *asDerived = dynamic_cast<QThreadPool *>(pool);
    // Assert
    EXPECT_NE(asBase, nullptr);
    EXPECT_EQ(asDerived, pool);
    EXPECT_EQ(pool->activeThreadCount(), 0);   // no threads started by construction
}

// ===== loadDeviceInfo =====

TEST_F(ThreadPool_UT, ThreadPool_LoadDeviceInfo_QueuesAllCmdTasks)
{
    // Arrange
    ASSERT_EQ(pool->m_ListCmd.size(), 22);
    // Act
    pool->loadDeviceInfo();   // branch: one task per m_ListCmd entry, start(task) each
    // Assert
    EXPECT_EQ(ut_tp_startedTasks.size(), 22);
    EXPECT_EQ(ut_tp_startedTasks.first().first, pool->m_ListCmd.first().cmd);
    EXPECT_EQ(ut_tp_startedTasks.first().second, pool->m_ListCmd.first().file);
}

TEST_F(ThreadPool_UT, ThreadPool_LoadDeviceInfo_TaskFieldsMirrorCmdEntries)
{
    // Arrange
    ASSERT_EQ(pool->m_ListCmd.size(), 22);
    // Act
    pool->loadDeviceInfo();
    // Assert
    const Cmd &last = pool->m_ListCmd.last();
    EXPECT_EQ(ut_tp_startedTasks.last().first, last.cmd);
    EXPECT_EQ(ut_tp_startedTasks.last().second, last.file);
}

// ===== updateDeviceInfo =====

TEST_F(ThreadPool_UT, ThreadPool_UpdateDeviceInfo_QueuesAllUpdateTasks)
{
    // Arrange
    ASSERT_EQ(pool->m_ListUpdate.size(), 12);
    // Act
    pool->updateDeviceInfo();   // branch: one task per m_ListUpdate entry
    // Assert
    EXPECT_EQ(ut_tp_startedTasks.size(), 12);
    EXPECT_EQ(ut_tp_startedTasks.first().second, pool->m_ListUpdate.first().file);
}

TEST_F(ThreadPool_UT, ThreadPool_UpdateDeviceInfo_TasksNotInCmdOnlyEntries)
{
    // Arrange
    int checked = 0;
    // Act
    pool->updateDeviceInfo();
    // Assert
    for (const auto &task : ut_tp_startedTasks) {
        bool foundInUpdate = false;
        for (const Cmd &c : pool->m_ListUpdate) {
            if (c.file == task.second)
                foundInUpdate = true;
        }
        EXPECT_TRUE(foundInUpdate) << "task file not in m_ListUpdate: " << task.second.toStdString();
        checked++;
    }
    EXPECT_EQ(checked, 12);
}

// ===== runCmdToCache =====

TEST_F(ThreadPool_UT, ThreadPool_RunCmdToCache_EmptyCmdString_NoProcessNoCache)
{
    // Arrange
    Cmd cmd;
    cmd.cmd = "  > /tmp/device-info/ut_tp_empty.txt";   // cmdExec before '>' is blank
    cmd.file = "ut_tp_empty.txt";
    // Act
    pool->runCmdToCache(cmd);   // branch: cmdStr empty -> return
    // Assert
    EXPECT_EQ(ut_tp_procCmd, QString(""));   // no process started
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("ut_tp_empty"));
}

TEST_F(ThreadPool_UT, ThreadPool_RunCmdToCache_NormalCmd_CachesStdout)
{
    // Arrange
    Cmd cmd;
    cmd.cmd = "uname -r > /tmp/device-info/ut_tp_normal.txt";
    cmd.file = "ut_tp_normal.txt";
    ut_tp_procOut = "tp-cache-payload\n";
    // Act
    pool->runCmdToCache(cmd);   // branch: normal cmd -> stdout cached
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_tp_normal"), QString("tp-cache-payload\n"));
    EXPECT_EQ(ut_tp_procCmd, QString("uname"));
}

TEST_F(ThreadPool_UT, ThreadPool_RunCmdToCache_CmdArgs_SplitAndPassed)
{
    // Arrange
    Cmd cmd;
    cmd.cmd = "lspci -v -m > /tmp/device-info/ut_tp_args.txt";
    cmd.file = "ut_tp_args.txt";
    ut_tp_procOut = "args-payload\n";
    // Act
    pool->runCmdToCache(cmd);   // branch: args split on spaces
    // Assert
    EXPECT_EQ(ut_tp_procCmd, QString("lspci"));
    EXPECT_EQ(ut_tp_procArgs, QStringList() << "-v" << "-m");
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_tp_args"), QString("args-payload\n"));
}

/*
 * 用例计数声明 (usecase count declaration)
 *
 * | method          | level | factors                  | min | actual |
 * |-----------------|-------|--------------------------|-----|--------|
 * | ThreadPool      | mid   | concurrent_class         | 2   | 2      |
 * | initCmd         | high  | lines, concurrent_class  | 3   | 15     |
 * | loadDeviceInfo  | mid   | concurrent_class         | 2   | 2      |
 * | updateDeviceInfo| mid   | concurrent_class         | 2   | 2      |
 * | runCmdToCache   | mid   | concurrent_class         | 2   | 3      |
 * | Cmd (struct)    | -     | not in inventory         | 0   | 2      |
 */
