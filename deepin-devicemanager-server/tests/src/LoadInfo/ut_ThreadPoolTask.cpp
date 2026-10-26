// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Unit tests for ThreadPoolTask (deepin-deviceinfo/src/loadinfo/threadpooltask.cpp)
 *
 * Branch list (source: MCP get_code_snippet @ 6ccddaf4):
 * ThreadPoolTask::ThreadPoolTask:
 *   B1: all parameters stored (cmd/file/replace/waiting)
 *   B2: default waiting (-1) when omitted
 * ThreadPoolTask::run:
 *   B1: m_Cmd == "lscpu" -> loadCpuInfo() + return
 *   B2: else -> runCmdToCache(m_Cmd)
 * ThreadPoolTask::runCmd(cmd, info):
 *   B1: cmdStr empty (">..." only) -> return, info untouched
 *   B2: cmd.startsWith("ls /dev/sg*") -> runAsteriskCmd(cmdStr, args.first())
 *   B3: cmdExec.startsWith("cat /boot/config*"):
 *       B3.1 filter empty -> return
 *       B3.2 runAsteriskCmd("ls", "/boot/config*") empty -> return
 *       B3.3 path empty -> continue
 *       B3.4 cat exitStatus!=NormalExit || exitCode!=0 -> info1 empty -> continue
 *       B3.5 results non-empty -> info = results.join('\n')
 *   B4: normal command -> start/waitForFinished(m_Waiting)/readAllStandardOutput
 *   B5: waitForFinished false (timeout) -> return with empty info
 * ThreadPoolTask::runAsteriskCmd:
 *   B1: cmd empty -> ""
 *   B2: arg not "/dev/sg*"/"/boot/config*" -> ""
 *   B3: matching lines -> path + "/" + line, joined with ' '
 *   B4: no matching lines -> ""
 * ThreadPoolTask::runCmdToCache:
 *   B1: m_CanNotReplace && isInfoExisted -> return (keep old value)
 *   B2: m_File == "lsblk_d.txt" -> loadSmartCtlInfoToCache(info)
 *   B3: m_File == "ls_sg.txt" -> loadSgSmartCtlInfoToCache(info)
 *   B4: m_File == "lspci.txt" -> loadLspciVSInfoToCache(info)
 *   B5: m_File == "hwinfo_display.txt" -> loadDisplayWidth(info)
 *   B6: addInfo(key, info) always on the runnable path
 * ThreadPoolTask::loadSmartCtlInfoToCache:
 *   B1: words.size() != 2 || words[0] == "NAME" -> skip
 *   B2: sInfo contains "Read Device Identity failed:" -> retry cmd + "1"
 *   B3: addInfo("smartctl_%1", sInfo) per device
 * ThreadPoolTask::loadSgSmartCtlInfoToCache:
 *   B1: line empty -> continue
 *   B2: words[2] = device -> runCmd smartctl + addInfo("smartctl_%1")
 * ThreadPoolTask::loadLspciVSInfoToCache:
 *   B1: words.size() < 2 -> continue
 *   B2: words[1] == "ISA" -> runCmd("lspci -v -s %1") + addInfo("lspci_vs") + break
 * ThreadPoolTask::loadDisplayWidth:
 *   B1: lines.size() < 5 -> continue
 *   B2: line contains "SysFS ID" -> width = getDisplayWidthFromLspci(line.right(7))
 *   B3: addInfo("width", widthS) once per param block (accumulated)
 * ThreadPoolTask::getDisplayWidthFromLspci:
 *   B1: line contains "Memory at" && "32-bit" -> 32
 *   B2: line contains "Memory at" (no 32-bit) -> 64
 *   B3: no "Memory at" line -> 64
 * ThreadPoolTask::loadCpuInfo:
 *   B1: CpuInfo::loadCpuInfo() true -> addInfo("lscpu") + addInfo("lscpu_num")
 *   B2: false -> nothing cached
 * ThreadPoolTask::runCmdToFile:
 *   B1: m_CanNotReplace && file.exists() -> return (no command run)
 *   B2: m_File == "lsblk_d.txt" -> loadSmartctlInfoToFile(file)
 *   B3: m_File == "lspci.txt" -> loadLspciVSInfoToFile(file)
 *   B4: other -> runCmd(cmd) only
 * ThreadPoolTask::loadSmartctlInfoToFile:
 *   B1: file not open -> nothing
 *   B2: words.size() != 2 || words[0] == "NAME" -> skip
 *   B3: 2 words -> runCmd("smartctl --all /dev/%1 > ...")
 * ThreadPoolTask::loadLspciVSInfoToFile:
 *   B1: file not open -> nothing
 *   B2: words.size() < 2 -> continue
 *   B3: words[1] == "ISA" -> runCmd("lspci -v -s %1 > ...") + break
 * ~ThreadPoolTask: implicit cleanup via QObject parent chain
 *
 * External dependencies stubbed (dependency-tracer §3 matrix):
 *   QProcess (start/waitForFinished/readAllStandardOutput/exitCode/exitStatus)
 *   CpuInfo::loadCpuInfo/logicalCpus/physicalNum/coreNum/logicalNum
 *   ThreadPoolTask::runCmd (both overloads) for cache-enrichment paths
 * DeviceInfoManager is a pure in-memory singleton (project-internal, no IO):
 *   kept REAL and used as the observation point (getInfo / isInfoExisted).
 */

#include "threadpooltask.h"
#include "deviceinfomanager.h"
#include "cpu/cpuinfo.h"
#include "DDLog.h"

#include <gtest/gtest.h>
#include <QProcess>
#include <QTemporaryFile>
#include <QDir>
#include <functional>

#include "../stub.h"
#include <dlfcn.h>
#include <cstring>

// Expose the PATH macro context (threadpooltask.h already included unmodified)

// ===== stub configuration state (reset in SetUp) =====
static QString ut_tt_procCmd;
static QStringList ut_tt_procArgs;
static bool ut_tt_waitResult = true;
static QStringList ut_tt_readOutQueue;   // per-call readAllStandardOutput; exhausted -> ""
static int ut_tt_exitCodeVal = 0;
static QProcess::ExitStatus ut_tt_exitStatusVal = QProcess::NormalExit;

static bool ut_tt_cpuLoadOk = false;
static QString ut_tt_cpuLogicalInfo = "cpu0\n";

static QStringList ut_tt_runCmd1Calls;
static QStringList ut_tt_runCmd2Calls;
static std::function<QString(const QString &)> ut_tt_runCmd2Filler;
static bool ut_tt_fileExistsRetVal = false;
static bool ut_tt_fopenRetVal = false;
static QByteArray ut_tt_freadContent;
// NOTE: &QFile::open / &QFile::exists / &QIODevice::readAll are VIRTUAL member
// functions; taking their address yields a vtable-index-encoded pmf (not a real
// code address), so prologue-patching with them corrupts nothing useful and
// crashes on use. Resolve the real code addresses via dlsym and build pmfs
// from them instead (lazy-initialized in SetUp before any stub.set()).
template <typename T>
static T ut_tt_pmfFromSymbol(const char *mangled)
{
    void *addr = dlsym(RTLD_DEFAULT, mangled);
    if (!addr) {
        fprintf(stderr, "[UT] dlsym failed for %s\n", mangled);
        abort();
    }
    T pmf;
    std::memset(&pmf, 0, sizeof(pmf));
    std::memcpy(&pmf, &addr, sizeof(void *));   // Itanium ABI: pmf = {fn ptr, adjustment}
    return pmf;
}

static bool (QFile::*ut_tt_fopenFn)(QIODevice::OpenMode) = nullptr;
static bool (QFile::*ut_tt_fexistsFn)() const = nullptr;
static QByteArray (QIODevice::*ut_tt_freadAllFn)() = nullptr;

static void ut_tt_initFilePmfs()
{
    static bool ready = false;
    if (ready) {
        return;
    }
    ut_tt_fopenFn = ut_tt_pmfFromSymbol<bool (QFile::*)(QIODevice::OpenMode)>(
        "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    ut_tt_fexistsFn = ut_tt_pmfFromSymbol<bool (QFile::*)() const>("_ZNK5QFile6existsEv");
    ut_tt_freadAllFn = ut_tt_pmfFromSymbol<QByteArray (QIODevice::*)()>("_ZN9QIODevice7readAllEv");
    ready = true;
}

static bool ut_tt_fileExists(QFile *)
{
    return ut_tt_fileExistsRetVal;
}

static bool ut_tt_fileOpen(QFile *, QIODevice::OpenMode)
{
    return ut_tt_fopenRetVal;
}

static QByteArray ut_tt_fileReadAll(QIODevice *)
{
    return ut_tt_freadContent;
}

// overload selectors
static void (QProcess::*ut_tt_procStartFn)(const QString &, const QStringList &, QIODevice::OpenMode) = &QProcess::start;  // __DBG_STUB_INVOKE__
static void (ThreadPoolTask::*ut_tt_runCmd1Fn)(const QString &) = &ThreadPoolTask::runCmd;
static void (ThreadPoolTask::*ut_tt_runCmd2Fn)(const QString &, QString &) = &ThreadPoolTask::runCmd;

static void ut_tt_procStart(QProcess *, const QString &cmd, const QStringList &args, QIODevice::OpenMode)
{
    ut_tt_procCmd = cmd;
    ut_tt_procArgs = args;
}

static bool ut_tt_waitFinished(QProcess *, int)
{
    return ut_tt_waitResult;
}

static QByteArray ut_tt_readOut(QProcess *)
{
    if (ut_tt_readOutQueue.isEmpty())
        return QByteArray();
    return ut_tt_readOutQueue.takeFirst().toUtf8();
}

static int ut_tt_exitCodeFn(QProcess *)
{
    return ut_tt_exitCodeVal;
}

static QProcess::ExitStatus ut_tt_exitStatusFn(QProcess *)
{
    return ut_tt_exitStatusVal;
}

static bool ut_tt_cpuLoad(CpuInfo *)
{
    return ut_tt_cpuLoadOk;
}

static int ut_tt_physNum(CpuInfo *)
{
    return 1;
}

static int ut_tt_coreNum(CpuInfo *)
{
    return 8;
}

static int ut_tt_logNum(CpuInfo *)
{
    return 16;
}

static void ut_tt_logicalCpus(CpuInfo *, QString &info)
{
    info = ut_tt_cpuLogicalInfo;
}

static void ut_tt_runCmd1(ThreadPoolTask *, const QString &cmd)
{
    ut_tt_runCmd1Calls << cmd;
}

static void ut_tt_runCmd2(ThreadPoolTask *, const QString &cmd, QString &info)
{
    ut_tt_runCmd2Calls << cmd;
    info = ut_tt_runCmd2Filler ? ut_tt_runCmd2Filler(cmd) : QString();
}

static void ut_tt_resetState()
{
    ut_tt_procCmd.clear();
    ut_tt_procArgs.clear();
    ut_tt_waitResult = true;
    ut_tt_readOutQueue.clear();
    ut_tt_exitCodeVal = 0;
    ut_tt_exitStatusVal = QProcess::NormalExit;
    ut_tt_cpuLoadOk = false;
    ut_tt_cpuLogicalInfo = "cpu0\n";
    ut_tt_runCmd1Calls.clear();
    ut_tt_runCmd2Calls.clear();
    ut_tt_runCmd2Filler = nullptr;
    ut_tt_fileExistsRetVal = false;
    ut_tt_fopenRetVal = false;
    ut_tt_freadContent.clear();
}

class ThreadPoolTask_UT : public ::testing::Test
{
public:
    Stub stub;
    void SetUp() override
    {
        ut_tt_resetState();
        ut_tt_initFilePmfs();   // resolve real QFile/QIODevice code addresses before stubbing
        stub.set(ut_tt_procStartFn, ut_tt_procStart);
        stub.set(ADDR(QProcess, waitForFinished), ut_tt_waitFinished);
        stub.set(ADDR(QProcess, readAllStandardOutput), ut_tt_readOut);
        stub.set(ADDR(QProcess, exitCode), ut_tt_exitCodeFn);
        stub.set(ADDR(QProcess, exitStatus), ut_tt_exitStatusFn);
        stub.set(ADDR(CpuInfo, loadCpuInfo), ut_tt_cpuLoad);
        stub.set(ADDR(CpuInfo, physicalNum), ut_tt_physNum);
        stub.set(ADDR(CpuInfo, coreNum), ut_tt_coreNum);
        stub.set(ADDR(CpuInfo, logicalNum), ut_tt_logNum);
        stub.set(ADDR(CpuInfo, logicalCpus), ut_tt_logicalCpus);
        stub.set(ut_tt_runCmd1Fn, ut_tt_runCmd1);
        stub.set(ut_tt_runCmd2Fn, ut_tt_runCmd2);
        stub.set(ut_tt_fexistsFn, ut_tt_fileExists);
        stub.set(ut_tt_fopenFn, ut_tt_fileOpen);
        stub.set(ut_tt_freadAllFn, ut_tt_fileReadAll);
        DeviceInfoManager::getInstance()->m_MapInfo.clear();   // isolate shared singleton state
        QDir().mkpath(PATH);
    }
    void TearDown() override
    {
        DeviceInfoManager::getInstance()->m_MapInfo.clear();
        stub.reset(ut_tt_freadAllFn);
        stub.reset(ut_tt_fopenFn);
        stub.reset(ut_tt_fexistsFn);
        stub.reset(ut_tt_runCmd2Fn);
        stub.reset(ut_tt_runCmd1Fn);
        stub.reset(ADDR(CpuInfo, logicalCpus));
        stub.reset(ADDR(CpuInfo, logicalNum));
        stub.reset(ADDR(CpuInfo, coreNum));
        stub.reset(ADDR(CpuInfo, physicalNum));
        stub.reset(ADDR(CpuInfo, loadCpuInfo));
        stub.reset(ADDR(QProcess, exitStatus));
        stub.reset(ADDR(QProcess, exitCode));
        stub.reset(ADDR(QProcess, readAllStandardOutput));
        stub.reset(ADDR(QProcess, waitForFinished));
        stub.reset(ut_tt_procStartFn);
    }
};

/* ================= constructor ================= */

TEST_F(ThreadPoolTask_UT, ThreadPoolTask_Ctor_StoresCmdFileAndReplaceFlag)
{
    // Arrange
    const int expectedWaiting = 700;
    // Act
    ThreadPoolTask task("lscpu", "lscpu.txt", true, expectedWaiting);
    // Assert
    EXPECT_EQ(task.m_Cmd, "lscpu");
    EXPECT_EQ(task.m_File, "lscpu.txt");
    EXPECT_TRUE(task.m_CanNotReplace);
    EXPECT_EQ(task.m_Waiting, 700);
}

TEST_F(ThreadPoolTask_UT, Ctor_ExplicitArgs_StoresAllValues)
{
    // Arrange
    const bool replaceAllowed = false;
    const int waitingTime = 500;
    // Act
    ThreadPoolTask task("hwinfo", "hwinfo.txt", replaceAllowed, waitingTime);
    // Assert
    EXPECT_EQ(task.m_Waiting, 500);   // branch: waiting stored verbatim
    EXPECT_EQ(task.parent(), nullptr);
    EXPECT_FALSE(task.m_CanNotReplace);
}

TEST_F(ThreadPoolTask_UT, Dtor_ParentedTask_CleansUpThroughParent)
{
    // Arrange
    QObject owner;
    // Act
    auto *task = new ThreadPoolTask("cmd", "file.txt", false, -1, &owner);
    // Assert
    EXPECT_EQ(task->parent(), &owner);   // branch: parent chain ownership
    EXPECT_EQ(task->m_Cmd, QString("cmd"));
    // task and owner destroyed together at scope end (~ThreadPoolTask)
}

/* ================= run ================= */

TEST_F(ThreadPoolTask_UT, Run_LscpuCommand_CachesCpuAndNumInfo)
{
    // Arrange
    ut_tt_cpuLoadOk = true;
    ut_tt_cpuLogicalInfo = "Model: ut-cpu\n";
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    // Act
    task.run();   // branch: m_Cmd == "lscpu" -> loadCpuInfo()
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu"), QString("Model: ut-cpu\n"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu_num"),
              QString("physical : 1\ncore : 8\nlogical : 16\n"));
}

TEST_F(ThreadPoolTask_UT, Run_LscpuCommand_LoadFailed_NothingCached)
{
    // Arrange
    ut_tt_cpuLoadOk = false;
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    DeviceInfoManager::getInstance()->addInfo("ut_lscpu_failed", "sentinel");
    // Act
    task.run();
    // Assert
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("lscpu"));   // branch: load failed -> skip
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("lscpu_num"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);   // no cache enrichment triggered
}

TEST_F(ThreadPoolTask_UT, Run_NonLscpuCommand_RunsCmdToCache)
{
    // Arrange
    ut_tt_runCmd2Filler = [](const QString &) { return QString("ut-payload"); };
    ThreadPoolTask task("hwinfo --all", "ut_run_hwinfo.txt", false, -1);
    // Act
    task.run();   // branch: m_Cmd != "lscpu" -> runCmdToCache
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_run_hwinfo"), QString("ut-payload"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 1);
}

TEST_F(ThreadPoolTask_UT, Run_NonLscpuCommand_CanNotReplaceExisting_KeepsOldValue)
{
    // Arrange
    DeviceInfoManager::getInstance()->addInfo("ut_run_cnr", "old-value");
    ut_tt_runCmd2Filler = [](const QString &) { return QString("new-value"); };
    ThreadPoolTask task("hwinfo --all", "ut_run_cnr.txt", true, -1);
    // Act
    task.run();   // branch: CanNotReplace && existed -> early return
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_run_cnr"), QString("old-value"));
    EXPECT_TRUE(ut_tt_runCmd2Calls.isEmpty());
}

/* ================= runCmd(cmd, info) ================= */

TEST_F(ThreadPoolTask_UT, RunCmd_EmptyCommandString_LeavesInfoUntouched)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd early-return branch
    ThreadPoolTask task("any", "ut_rc_empty.txt", false, -1);
    QString info = "keep-me";
    // Act
    task.runCmd("> /tmp/device-info/ut_rc_empty.txt", info);   // branch: cmdStr empty
    // Assert
    EXPECT_EQ(info, QString("keep-me"));
    EXPECT_TRUE(ut_tt_procCmd.isEmpty());
}

TEST_F(ThreadPoolTask_UT, RunCmd_NormalCommand_CapturesStdout)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("any", "ut_rc_normal.txt", false, 42);
    ut_tt_readOutQueue << "stdout-data\n";
    QString info;
    // Act
    task.runCmd("uname -r", info);   // branch: normal command path
    // Assert
    EXPECT_EQ(info, QString("stdout-data\n"));
    EXPECT_EQ(ut_tt_procCmd, QString("uname"));
    EXPECT_EQ(ut_tt_procArgs, QStringList() << "-r");
}

TEST_F(ThreadPoolTask_UT, RunCmd_NormalCommand_Timeout_LeavesInfoEmpty)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("any", "ut_rc_timeout.txt", false, 5);
    ut_tt_waitResult = false;
    QString info;
    // Act
    task.runCmd("sleep 100", info);   // branch: waitForFinished false -> return
    // Assert
    EXPECT_TRUE(info.isEmpty());
    EXPECT_EQ(ut_tt_procCmd, QString("sleep"));
}

TEST_F(ThreadPoolTask_UT, RunCmd_LsDevSg_UsesAsteriskHandler)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("ls /dev/sg* > /tmp/device-info/ls_sg.txt", "ls_sg.txt", false, -1);
    ut_tt_readOutQueue << "sg0\nsg1\n";   // consumed by runAsteriskCmd's ls process
    QString info;
    // Act
    task.runCmd("ls /dev/sg* > /tmp/device-info/ls_sg.txt", info);   // branch: startsWith("ls /dev/sg*")
    // Assert
    EXPECT_TRUE(info.contains("/dev/sg0"));
    EXPECT_TRUE(info.contains("/dev/sg1"));
    EXPECT_EQ(ut_tt_procCmd, QString("ls"));
}

TEST_F(ThreadPoolTask_UT, RunCmd_CatBootConfig_FiltersMatchingLines)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("cat", "ut_rc_cat.txt", false, -1);
    ut_tt_readOutQueue << "config-5.10\n"                        // ls /boot output: bare names
                       << "line-a\nCONFIG_X86=y\nline-b\n";      // cat output
    QString info;
    // Act
    task.runCmd("cat /boot/config* | grep 'CONFIG_X86' > /tmp/device-info/config.txt", info);
    // Assert
    EXPECT_EQ(info, QString("CONFIG_X86=y"));   // branch: results joined with '\n'
    EXPECT_EQ(ut_tt_procCmd, QString("cat"));
    EXPECT_EQ(ut_tt_procArgs, QStringList() << "/boot/config-5.10");
}

TEST_F(ThreadPoolTask_UT, RunCmd_CatBootConfig_CatFails_SkipsPath)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("cat", "ut_rc_catfail.txt", false, -1);
    ut_tt_readOutQueue << "config-5.10\n";
    ut_tt_exitCodeVal = 1;   // cat exits non-zero
    QString info;
    // Act
    task.runCmd("cat /boot/config* | grep 'CONFIG_X86' > /tmp/device-info/config.txt", info);
    // Assert
    EXPECT_TRUE(info.isEmpty());   // branch: exitCode != 0 -> info1 empty -> continue
    EXPECT_EQ(ut_tt_procCmd, QString("cat"));
}

TEST_F(ThreadPoolTask_UT, RunCmd_CatBootConfig_EmptyFilter_ReturnsEarly)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("cat", "ut_rc_catnofilter.txt", false, -1);
    QString info = "untouched";
    // Act
    task.runCmd("cat /boot/config* | > /tmp/device-info/config.txt", info);   // branch: filter empty
    // Assert
    EXPECT_EQ(info, QString("untouched"));
    EXPECT_TRUE(ut_tt_procCmd.isEmpty());   // no process started
}

TEST_F(ThreadPoolTask_UT, RunCmd_CatBootConfig_LsOutputEmpty_ReturnsEarly)
{
    // Arrange
    stub.reset(ut_tt_runCmd2Fn);   // exercise the real runCmd logic
    ThreadPoolTask task("cat", "ut_rc_catnols.txt", false, -1);
    ut_tt_readOutQueue << "";   // ls /boot/config* produced nothing
    QString info;
    // Act
    task.runCmd("cat /boot/config* | grep 'CONFIG_X86' > /tmp/device-info/config.txt", info);
    // Assert
    EXPECT_TRUE(info.isEmpty());   // branch: outPut empty -> return
    EXPECT_EQ(ut_tt_procCmd, QString("ls"));
}

/* ================= runAsteriskCmd ================= */

TEST_F(ThreadPoolTask_UT, RunAsteriskCmd_EmptyCmd_ReturnsEmpty)
{
    // Arrange
    ThreadPoolTask task("any", "file.txt", false, -1);
    // Act
    QString ret = task.runAsteriskCmd("", "/dev/sg*");   // branch: cmd empty
    // Assert
    EXPECT_EQ(ret, QString(""));
    EXPECT_TRUE(ut_tt_procCmd.isEmpty());
}

TEST_F(ThreadPoolTask_UT, RunAsteriskCmd_UnsupportedArg_ReturnsEmpty)
{
    // Arrange
    ThreadPoolTask task("any", "file.txt", false, -1);
    // Act
    QString ret = task.runAsteriskCmd("ls", "/dev/sda*");   // branch: arg not supported
    // Assert
    EXPECT_EQ(ret, QString(""));
    EXPECT_TRUE(ut_tt_procCmd.isEmpty());
}

TEST_F(ThreadPoolTask_UT, RunAsteriskCmd_SgArg_FiltersPrefixedDevices)
{
    // Arrange
    ThreadPoolTask task("any", "file.txt", false, -1);
    ut_tt_readOutQueue << "sg0\nsg1\nsdz\n";
    // Act
    QString ret = task.runAsteriskCmd("ls", "/dev/sg*");   // branch: /dev/sg* supported
    // Assert
    EXPECT_TRUE(ret.contains("/dev/sg0\n"));
    EXPECT_TRUE(ret.contains("/dev/sg1\n"));
    EXPECT_FALSE(ret.contains("sdz"));   // branch: startWord filter drops non-matching
    EXPECT_EQ(ut_tt_procCmd, QString("ls"));
}

TEST_F(ThreadPoolTask_UT, RunAsteriskCmd_BootConfigArg_FiltersConfigs)
{
    // Arrange
    ThreadPoolTask task("any", "file.txt", false, -1);
    ut_tt_readOutQueue << "config-5.10\nconfig-6.1\nother\n";
    // Act
    QString ret = task.runAsteriskCmd("ls", "/boot/config*");   // branch: /boot/config* supported
    // Assert
    EXPECT_TRUE(ret.contains("/boot/config-5.10\n"));
    EXPECT_TRUE(ret.contains("/boot/config-6.1\n"));
    EXPECT_FALSE(ret.contains("other"));
    EXPECT_EQ(ut_tt_procCmd, QString("ls"));
}

TEST_F(ThreadPoolTask_UT, RunAsteriskCmd_NoMatchingLines_ReturnsEmpty)
{
    // Arrange
    ThreadPoolTask task("any", "file.txt", false, -1);
    ut_tt_readOutQueue << "sdz\nsr0\n";
    // Act
    QString ret = task.runAsteriskCmd("ls", "/dev/sg*");   // branch: filterLines empty
    // Assert
    EXPECT_EQ(ret, QString(""));
    EXPECT_EQ(ut_tt_procCmd, QString("ls"));   // ls process was started but output filtered out
}

/* ================= runCmdToCache ================= */

TEST_F(ThreadPoolTask_UT, RunCmdToCache_NewKey_CachesStdout)
{
    // Arrange
    ut_tt_runCmd2Filler = [](const QString &) { return QString("cache-payload"); };
    ThreadPoolTask task("hwinfo --all", "ut_cache_new.txt", false, -1);
    // Act
    task.runCmdToCache("hwinfo --all");   // branch: runnable -> addInfo(key, info)
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_cache_new"), QString("cache-payload"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 1);
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_CanNotReplaceExisting_KeepsOldValue)
{
    // Arrange
    DeviceInfoManager::getInstance()->addInfo("ut_cache_cnr", "old");
    ut_tt_runCmd2Filler = [](const QString &) { return QString("new"); };
    ThreadPoolTask task("hwinfo --all", "ut_cache_cnr.txt", true, -1);
    // Act
    task.runCmdToCache("hwinfo --all");   // branch: m_CanNotReplace && existed -> return
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_cache_cnr"), QString("old"));
    EXPECT_TRUE(ut_tt_runCmd2Calls.isEmpty());
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_CanReplaceExisting_UpdatesValue)
{
    // Arrange
    DeviceInfoManager::getInstance()->addInfo("ut_cache_upd", "old");
    ut_tt_runCmd2Filler = [](const QString &) { return QString("updated"); };
    ThreadPoolTask task("hwinfo --all", "ut_cache_upd.txt", false, -1);
    // Act
    task.runCmdToCache("hwinfo --all");   // branch: replace allowed
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("ut_cache_upd"), QString("updated"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 1);
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_LsblkFile_EnrichesSmartctlCache)
{
    // Arrange
    ut_tt_runCmd2Filler = [](const QString &cmd) {
        return cmd.contains("/dev/sda") ? QString("rota-sda")
                                       : QString("NAME ROTA\nsda 1\n");
    };
    ThreadPoolTask task("lsblk -d > f", "lsblk_d.txt", false, -1);
    // Act
    task.runCmdToCache("lsblk -d > f");   // branch: m_File == "lsblk_d.txt"
    // Assert
    EXPECT_TRUE(DeviceInfoManager::getInstance()->isInfoExisted("lsblk_d"));
    EXPECT_TRUE(DeviceInfoManager::getInstance()->isInfoExisted("smartctl_sda"));
    EXPECT_EQ(ut_tt_runCmd2Calls.at(1), QString("smartctl --all /dev/sda"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sda"), QString("rota-sda"));
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_LsSgFile_EnrichesSmartctlCache)
{
    // Arrange — /dev/sgN lines: loadSgSmartCtlInfoToCache splits by "/" and reads words[2]
    ut_tt_runCmd2Filler = [](const QString &cmd) {
        return cmd.startsWith("smartctl") ? QString("sg-info") : QString("/dev/sg0\n/dev/sg1\n");
    };
    ThreadPoolTask task("ls /dev/sg* > f", "ls_sg.txt", false, -1);
    // Act
    task.runCmdToCache("ls /dev/sg* > /tmp/device-info/ls_sg.txt");   // branch: m_File == "ls_sg.txt"
    // Assert
    EXPECT_TRUE(DeviceInfoManager::getInstance()->isInfoExisted("smartctl_sg0"));
    EXPECT_TRUE(DeviceInfoManager::getInstance()->isInfoExisted("smartctl_sg1"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 3);   // one ls + one smartctl per sg device
    EXPECT_EQ(ut_tt_runCmd2Calls.at(1), QString("smartctl --all /dev/sg0"));
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_LspciFile_EnrichesLspciVsCache)
{
    // Arrange — loadLspciVSInfoToCache needs a "<pci> ISA ..." line to trigger lspci -v -s
    ut_tt_runCmd2Filler = [](const QString &cmd) {
        return cmd.contains("-v -s") ? QString("vs-detail") : QString("00:1f.0 ISA bridge\n");
    };
    ThreadPoolTask task("lspci > f", "lspci.txt", false, -1);
    // Act
    task.runCmdToCache("lspci > /tmp/device-info/lspci.txt");   // branch: m_File == "lspci.txt"
    // Assert
    EXPECT_TRUE(DeviceInfoManager::getInstance()->isInfoExisted("lspci_vs"));
    EXPECT_EQ(ut_tt_runCmd2Calls.at(1), QString("lspci -v -s 00:1f.0"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lspci_vs"), QString("vs-detail"));
}

TEST_F(ThreadPoolTask_UT, RunCmdToCache_HwinfoDisplayFile_ComputesWidth)
{
    // Arrange — loadDisplayWidth needs ≥5 lines with "SysFS ID 00:02.0" (right(7) = pci)
    ut_tt_runCmd2Filler = [](const QString &cmd) {
        return cmd.contains("-v -s")
            ? QString("Memory at f0000000 (32-bit, non-prefetchable)\n")
            : QString("blk1\nblk2\nblk3\nblk4\nSysFS ID 00:02.0\n");
    };
    ThreadPoolTask task("hwinfo --display", "hwinfo_display.txt", false, -1);
    // Act
    task.runCmdToCache("hwinfo --display > f");   // branch: m_File == "hwinfo_display.txt"
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("width"), QString("00:02.0-32\n"));
    EXPECT_EQ(ut_tt_runCmd2Calls.at(1), QString("lspci -v -s 00:02.0"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 2);   // hwinfo + lspci -v -s
}

/* ================= loadSmartCtlInfoToCache ================= */

TEST_F(ThreadPoolTask_UT, LoadSmartCtlInfoToCache_TwoDevices_CachesPerDevice)
{
    // Arrange
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) { return QString("rota-data"); };
    // Act
    task.loadSmartCtlInfoToCache("NAME ROTA\nsda 1\nsdb 0\nmalformed\n");   // branch: NAME/malformed skipped
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sda"), QString("rota-data"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sdb"), QString("rota-data"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 2);   // branch: words.size()==2 only
}

TEST_F(ThreadPoolTask_UT, LoadSmartCtlInfoToCache_IdentityFailed_RetriesWithPartition)
{
    // Arrange
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &cmd) {
        return cmd.endsWith("/dev/sda") ? QString("Read Device Identity failed: Inappropriate")
                                        : QString("retry-ok-data");
    };
    // Act
    task.loadSmartCtlInfoToCache("NAME ROTA\nsda 1\n");   // branch: retry cmd + "1"
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sda"), QString("retry-ok-data"));
    EXPECT_EQ(ut_tt_runCmd2Calls.at(1), QString("smartctl --all /dev/sda1"));
}

TEST_F(ThreadPoolTask_UT, LoadSmartCtlInfoToCache_EmptyInput_NothingCached)
{
    // Arrange
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    // Act
    task.loadSmartCtlInfoToCache("");   // loop boundary: empty input
    // Assert
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("smartctl_sda"));
}

/* ================= loadSgSmartCtlInfoToCache ================= */

TEST_F(ThreadPoolTask_UT, LoadSgSmartCtlInfoToCache_SgLines_CachesPerDevice)
{
    // Arrange
    ThreadPoolTask task("ls /dev/sg*", "ls_sg.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) { return QString("sg-data"); };
    // Act
    task.loadSgSmartCtlInfoToCache("/dev/sg0\n\n/dev/sg1\n");   // branch: empty lines skipped
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sg0"), QString("sg-data"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("smartctl_sg1"), QString("sg-data"));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 2);
}

TEST_F(ThreadPoolTask_UT, LoadSgSmartCtlInfoToCache_EmptyLinesOnly_NothingCached)
{
    // Arrange
    ThreadPoolTask task("ls /dev/sg*", "ls_sg.txt", false, -1);
    // Act
    task.loadSgSmartCtlInfoToCache("\n\n");   // branch: all lines empty -> continue
    // Assert
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("smartctl_sg0"));
}

/* ================= loadLspciVSInfoToCache ================= */

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToCache_IsaLine_CachesVsInfo)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) { return QString("vs-info"); };
    // Act
    task.loadLspciVSInfoToCache("00:14.0 USB controller: X\n00:1f.0 ISA bridge: Y\n");   // branch: ISA found
    // Assert
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 1);   // break after first ISA line
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:1f.0"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lspci_vs"), QString("vs-info"));
}

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToCache_NoIsaLine_NothingCached)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    // Act
    task.loadLspciVSInfoToCache("00:14.0 USB controller: X\nshort\n");   // branch: no ISA / short line
    // Assert
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);
    EXPECT_FALSE(DeviceInfoManager::getInstance()->isInfoExisted("lspci_vs"));
}

/* ================= loadDisplayWidth ================= */

TEST_F(ThreadPoolTask_UT, LoadDisplayWidth_SysFsIdBlock_ComputesWidth32)
{
    // Arrange
    ThreadPoolTask task("hwinfo", "hwinfo_display.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("Memory at f0000000 (32-bit, non-prefetchable)");
    };
    const QString block = "  20: PCI 02.0: 0300 VGA controller\n"
                          "  [Created at pci.378]\n"
                          "  Unique ID: abc\n"
                          "  SysFS ID: /devices/pci0000:00/0000:00:02.0\n"
                          "  Module: nouveau\n";
    // Act
    task.loadDisplayWidth(block);   // branch: SysFS ID found, lines >= 5
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("width"), QString("00:02.0-32\n"));
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:02.0"));
}

TEST_F(ThreadPoolTask_UT, LoadDisplayWidth_Memory64Bit_ComputesWidth64)
{
    // Arrange
    ThreadPoolTask task("hwinfo", "hwinfo_display.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("Memory at e0000000 (64-bit, non-prefetchable)");
    };
    const QString block = "  20: PCI 03.0: 0300 VGA controller\n"
                          "  [Created at pci.378]\n"
                          "  Unique ID: def\n"
                          "  SysFS ID: /devices/pci0000:00/0000:00:03.0\n"
                          "  Module: i915\n";
    // Act
    task.loadDisplayWidth(block);   // branch: getDisplayWidthFromLspci returns 64
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("width"), QString("00:03.0-64\n"));
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:03.0"));
}

TEST_F(ThreadPoolTask_UT, LoadDisplayWidth_ShortBlock_Skipped)
{
    // Arrange
    ThreadPoolTask task("hwinfo", "hwinfo_display.txt", false, -1);
    // Act
    task.loadDisplayWidth("only\nthree\nlines\n");   // branch: lines.size() < 5 -> continue
    // Assert
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("width"), QString(""));
}

TEST_F(ThreadPoolTask_UT, LoadDisplayWidth_TwoBlocks_AccumulatesPerBlock)
{
    // Arrange
    ThreadPoolTask task("hwinfo", "hwinfo_display.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("Memory at f0000000 (32-bit, non-prefetchable)");
    };
    const QString blocks = "  a\n  b\n  c\n  SysFS ID: /devices/0000:00:02.0\n  d\n"
                           "\n"
                           "  e\n  f\n  g\n  SysFS ID: /devices/0000:00:03.0\n  h\n";
    // Act
    task.loadDisplayWidth(blocks);   // branch: addInfo called once per block, widthS accumulates
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("width"),
              QString("00:02.0-32\n00:03.0-32\n"));   // loop boundary: 2 param blocks
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 2);
}

/* ================= getDisplayWidthFromLspci ================= */

TEST_F(ThreadPoolTask_UT, GetDisplayWidthFromLspci_Memory32Bit_Returns32)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("Region 0: Memory at f0000000 (32-bit, non-prefetchable)\n");
    };
    // Act
    int width = task.getDisplayWidthFromLspci("00:02.0");   // branch: Memory at + 32-bit
    // Assert
    EXPECT_EQ(width, 32);
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:02.0"));
}

TEST_F(ThreadPoolTask_UT, GetDisplayWidthFromLspci_Memory64Bit_Returns64)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("Region 0: Memory at e0000000 (64-bit, prefetchable)\n");
    };
    // Act
    int width = task.getDisplayWidthFromLspci("00:03.0");   // branch: Memory at, no 32-bit
    // Assert
    EXPECT_EQ(width, 64);
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:03.0"));
}

TEST_F(ThreadPoolTask_UT, GetDisplayWidthFromLspci_NoMemoryLine_Returns64)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) { return QString("no memory info here\n"); };
    // Act
    int width = task.getDisplayWidthFromLspci("00:04.0");   // branch: no "Memory at" line
    // Assert
    EXPECT_EQ(width, 64);
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:04.0"));
}

TEST_F(ThreadPoolTask_UT, GetDisplayWidthFromLspci_MemoryInLaterLine_FindsIt)
{
    // Arrange
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    ut_tt_runCmd2Filler = [](const QString &) {
        return QString("first line\nsecond line\nMemory at f0000000 (32-bit)\n");
    };
    // Act
    int width = task.getDisplayWidthFromLspci("00:05.0");   // loop boundary: Memory at in 3rd line
    // Assert
    EXPECT_EQ(width, 32);
    EXPECT_EQ(ut_tt_runCmd2Calls.first(), QString("lspci -v -s 00:05.0"));
}

/* ================= loadCpuInfo ================= */

TEST_F(ThreadPoolTask_UT, LoadCpuInfo_Success_CachesExactNumFormat)
{
    // Arrange
    ut_tt_cpuLoadOk = true;
    ut_tt_cpuLogicalInfo = "Model name: UT-TEST-CPU\n";
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    // Act
    task.loadCpuInfo();   // branch: loadCpuInfo true
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu"), QString("Model name: UT-TEST-CPU\n"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu_num"),
              QString("physical : 1\ncore : 8\nlogical : 16\n"));
}

TEST_F(ThreadPoolTask_UT, LoadCpuInfo_Failure_NothingCached)
{
    // Arrange
    ut_tt_cpuLoadOk = false;
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    // Act
    task.loadCpuInfo();   // branch: loadCpuInfo false -> skip
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu"), QString(""));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu_num"), QString(""));
    EXPECT_EQ(ut_tt_runCmd2Calls.size(), 0);
}

TEST_F(ThreadPoolTask_UT, LoadCpuInfo_RepeatedLoad_UpdatesExistingKeys)
{
    // Arrange
    ut_tt_cpuLoadOk = true;
    ut_tt_cpuLogicalInfo = "first\n";
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    task.loadCpuInfo();
    // Act
    ut_tt_cpuLogicalInfo = "second\n";
    task.loadCpuInfo();   // branch: addInfo update path on existing key
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu"), QString("second\n"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu_num"),
              QString("physical : 1\ncore : 8\nlogical : 16\n"));
}

TEST_F(ThreadPoolTask_UT, LoadCpuInfo_StubbedNumbers_AreUsedInNumInfo)
{
    // Arrange
    ut_tt_cpuLoadOk = true;
    ut_tt_cpuLogicalInfo = "stubbed\n";
    ThreadPoolTask task("lscpu", "lscpu.txt", false, -1);
    // Act
    task.loadCpuInfo();   // branch: physicalNum/coreNum/logicalNum stubbed values
    // Assert
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu_num"),
              QString("physical : 1\ncore : 8\nlogical : 16\n"));   // stubbed values flow into format
    EXPECT_EQ(DeviceInfoManager::getInstance()->getInfo("lscpu"), QString("stubbed\n"));
}

/* ================= runCmdToFile ================= */

TEST_F(ThreadPoolTask_UT, RunCmdToFile_CanNotReplaceExisting_SkipsCommand)
{
    // Arrange
    ut_tt_fileExistsRetVal = true;   // cached file already exists
    ThreadPoolTask task("lshw > f", "ut_r2f_cnr.txt", true, -1);
    // Act
    task.runCmdToFile("lshw > f");   // branch: CanNotReplace && exists -> return
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);
    EXPECT_EQ(ut_tt_procCmd, QString(""));   // no real process ever started
}

TEST_F(ThreadPoolTask_UT, RunCmdToFile_CanReplaceExisting_RunsCommand)
{
    // Arrange
    ut_tt_fileExistsRetVal = false;   // no cache file yet
    ThreadPoolTask task("lshw > f", "ut_r2f_rep.txt", false, -1);
    // Act
    task.runCmdToFile("lshw > f");   // branch: replace allowed -> runCmd(cmd)
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls, QStringList() << QString("lshw > f"));
    EXPECT_EQ(ut_tt_procCmd, QString(""));   // runCmd stubbed -> no real process
}

TEST_F(ThreadPoolTask_UT, RunCmdToFile_LsblkFile_TriggersSmartctlLoad)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("NAME ROTA\nsda 1\n");
    ThreadPoolTask task("lsblk > f", "lsblk_d.txt", false, -1);
    // Act
    task.runCmdToFile("lsblk -d -o NAME,ROTA > /tmp/device-info/lsblk_d.txt");   // branch: m_File == lsblk_d.txt
    // Assert
    ASSERT_EQ(ut_tt_runCmd1Calls.size(), 2);
    EXPECT_EQ(ut_tt_runCmd1Calls.first(), QString("lsblk -d -o NAME,ROTA > /tmp/device-info/lsblk_d.txt"));
    EXPECT_EQ(ut_tt_runCmd1Calls.last(), QString("smartctl --all /dev/sda > /tmp/device-info/smartctl_sda.txt"));
}

TEST_F(ThreadPoolTask_UT, RunCmdToFile_LspciFile_TriggersVsLoad)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("00:14.0 USB controller: X\n00:1f.0 ISA bridge: Y\n");
    ThreadPoolTask task("lspci > f", "lspci.txt", false, -1);
    // Act
    task.runCmdToFile("lspci > /tmp/device-info/lspci.txt");   // branch: m_File == lspci.txt
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 2);
    EXPECT_EQ(ut_tt_runCmd1Calls.last(), QString("lspci -v -s 00:1f.0 > /tmp/device-info/lspci_vs.txt"));
    EXPECT_EQ(ut_tt_runCmd1Calls.first(), QString("lspci > /tmp/device-info/lspci.txt"));
}

TEST_F(ThreadPoolTask_UT, RunCmdToFile_OtherFile_OnlyBaseCommand)
{
    // Arrange
    ThreadPoolTask task("upower -d > f", "ut_r2f_other.txt", false, -1);
    // Act
    task.runCmdToFile("upower -d > /tmp/device-info/ut_r2f_other.txt");   // branch: no enrichment file
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 1);
    EXPECT_EQ(ut_tt_runCmd1Calls.first(), QString("upower -d > /tmp/device-info/ut_r2f_other.txt"));
}

/* ================= loadSmartctlInfoToFile ================= */

TEST_F(ThreadPoolTask_UT, LoadSmartctlInfoToFile_ValidDevices_RunsPerDeviceCmd)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("NAME ROTA\nsda 1\nsdb 0\n\n");
    QFile file(QString(PATH) + QStringLiteral("lsblk_d.txt"));
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    // Act
    task.loadSmartctlInfoToFile(file);   // branch: words.size()==2 -> runCmd
    // Assert
    ASSERT_EQ(ut_tt_runCmd1Calls.size(), 2);
    EXPECT_EQ(ut_tt_runCmd1Calls.at(0), QString("smartctl --all /dev/sda > /tmp/device-info/smartctl_sda.txt"));
    EXPECT_EQ(ut_tt_runCmd1Calls.at(1), QString("smartctl --all /dev/sdb > /tmp/device-info/smartctl_sdb.txt"));
}

TEST_F(ThreadPoolTask_UT, LoadSmartctlInfoToFile_HeaderAndMalformed_Skipped)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("NAME ROTA\nmalformed-line\n\n");
    QFile file(QString(PATH) + QStringLiteral("lsblk_d.txt"));
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    // Act
    task.loadSmartctlInfoToFile(file);
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);   // branch: NAME / size!=2 skipped
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

TEST_F(ThreadPoolTask_UT, LoadSmartctlInfoToFile_MultiSpaceLine_NormalizedToTwoWords)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("sda    1\n");   // multiple spaces -> regex collapse
    QFile file(QString(PATH) + QStringLiteral("lsblk_d.txt"));
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    // Act
    task.loadSmartctlInfoToFile(file);   // branch: regex normalization
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 1);
    EXPECT_EQ(ut_tt_runCmd1Calls.first(), QString("smartctl --all /dev/sda > /tmp/device-info/smartctl_sda.txt"));
}

TEST_F(ThreadPoolTask_UT, LoadSmartctlInfoToFile_UnopenableFile_NoCommands)
{
    // Arrange
    ut_tt_fopenRetVal = false;   // unopenable file
    QFile file(QString(PATH) + QStringLiteral("ut_no_such_smartctl_file.txt"));
    ThreadPoolTask task("lsblk", "lsblk_d.txt", false, -1);
    // Act
    task.loadSmartctlInfoToFile(file);   // branch: open fails -> nothing
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

/* ================= loadLspciVSInfoToFile ================= */

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToFile_IsaLine_RunsCmdAndStops)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("00:14.0 USB controller: X\n00:1f.0 ISA bridge: Y\n00:1f.1 ISA bridge: Z\n");
    QFile file(QString(PATH) + QStringLiteral("lspci.txt"));
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    // Act
    task.loadLspciVSInfoToFile(file);   // branch: ISA -> runCmd + break
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 1);   // break: second ISA line not processed
    EXPECT_EQ(ut_tt_runCmd1Calls.first(), QString("lspci -v -s 00:1f.0 > /tmp/device-info/lspci_vs.txt"));
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToFile_NoIsaLine_NoCommand)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("00:14.0 USB controller: X\n");   // no ISA line
    QFile file(QString(PATH) + QStringLiteral("lspci.txt"));
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    // Act
    task.loadLspciVSInfoToFile(file);   // branch: no ISA line
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToFile_ShortLines_Skipped)
{
    // Arrange
    ut_tt_fopenRetVal = true;
    ut_tt_freadContent = QByteArray("lonelyline\n");   // single word, no device
    QFile file(QString(PATH) + QStringLiteral("lspci.txt"));
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    // Act
    task.loadLspciVSInfoToFile(file);   // branch: words.size() < 2 -> continue
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

TEST_F(ThreadPoolTask_UT, LoadLspciVsInfoToFile_UnopenableFile_NoCommand)
{
    // Arrange
    QFile file(QString(PATH) + QStringLiteral("ut_no_such_lspci_file.txt"));
    ThreadPoolTask task("lspci", "lspci.txt", false, -1);
    // Act
    task.loadLspciVSInfoToFile(file);   // branch: open fails
    // Assert
    EXPECT_EQ(ut_tt_runCmd1Calls.size(), 0);
    EXPECT_EQ(ut_tt_procCmd, QString(""));
}

/*
 * 用例计数声明 (usecase count declaration)
 *
 * | method                  | level | factors                      | min | actual |
 * |-------------------------|-------|------------------------------|-----|--------|
 * | ThreadPoolTask          | mid   | in_degree:1                  | 2   | 3      |
 * | ~ThreadPoolTask         | low   | -                            | 1   | 1      |
 * | run                     | high  | transitive_loop_depth:4      | 4   | 4      |
 * | runCmd                  | high  | complexity:14, lines:60      | 4   | 8      |
 * | runAsteriskCmd          | high  | complexity:5, in_degree:1    | 3   | 5      |
 * | runCmdToCache           | high  | complexity:5                 | 3   | 7      |
 * | loadSmartCtlInfoToCache | mid   | in_degree:1                  | 2   | 3      |
 * | loadSgSmartCtlInfoToCache | mid | in_degree:1                  | 2   | 2      |
 * | loadLspciVSInfoToCache  | mid   | in_degree:1                  | 2   | 2      |
 * | loadDisplayWidth        | mid   | in_degree:1                  | 2   | 4      |
 * | getDisplayWidthFromLspci| high  | transitive_loop_depth:3      | 4   | 4      |
 * | loadCpuInfo             | high  | transitive_loop_depth:4      | 4   | 4      |
 * | runCmdToFile            | high  | transitive_loop_depth:3      | 4   | 5      |
 * | loadSmartctlInfoToFile  | high  | transitive_loop_depth:3      | 4   | 4      |
 * | loadLspciVSInfoToFile   | high  | transitive_loop_depth:3      | 4   | 4      |
 */
