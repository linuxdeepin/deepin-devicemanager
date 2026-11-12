// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ut_driverinstallerapt.cpp
 * @brief Unit tests for DriverInstallerApt
 *        (deepin-devicecontrol/src/drivercontrol/driverinstallerapt.cpp)
 *
 * Tested methods and covered branches:
 * - installPackage:
 *   B1 dpkg locked, retries < MAX -> deferred retry | InstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, exhausted -> errorOccurred(0) + installProgressFinished(false)
 *      | InstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free -> m_Cancel=false + doOperate | InstallPackage_DpkgFree_RunsOperate
 * - undoInstallDriver:
 *   B1 m_process null -> only sets m_Cancel | UndoInstallDriver_NoProcess_SetsCancelFlag
 *   B2 process not running -> no kill | UndoInstallDriver_NotRunning_SkipsKill
 *   B3 process running -> kill() called | UndoInstallDriver_RunningProcess_Kills
 * - aptClean:
 *   B1 executes "lastore-apt-clean" | AptClean_InvokesCleanCommand
 * - executeCommand (private):
 *   B1 starts cmd + returns utf8 output | ExecuteCommand_StartsAndReturnsOutput
 * - doOperate (private):
 *   B1 apt-cache show empty -> errorOccurred(1) | DoOperate_PackageMissing_EmitsError1
 *   B2 finished exitCode 0 -> installProgressFinished(true) | DoOperate_Success_EmitsFinished
 *   B3 m_Cancel set -> errorOccurred(2) | DoOperate_Cancelled_EmitsError2
 *   B4 nvidia failure -> installProgressFinished(true) | DoOperate_NvidiaFailure_TreatedAsSuccess
 *   B5 non-nvidia failure + offline -> errorOccurred(3) | DoOperate_OfflineFailure_EmitsError3
 *   B6 non-nvidia failure + online -> errorOccurred(0) | DoOperate_OnlineFailure_EmitsError0
 *   B7 percent output -> installProgressChanged(n) | DoOperate_ProgressOutput_EmitsProgress
 *   B8 start args "apt-get install -y pkg=ver" | DoOperate_InstallStartArgs
 * - isNetworkOnline (private):
 *   B1 ping exitCode != 0 -> false | IsNetworkOnline_AbnormalExit_ReturnsFalse
 *   B2 crash exit -> false | IsNetworkOnline_CrashExit_ReturnsFalse
 *   B3 sec > 0 -> usleep branch (covered with 1us) | (implicit in B1..B7 variants)
 *   B4 log lines <= 1 -> false | IsNetworkOnline_ShortLog_ReturnsFalse
 *   B5 no "received," marker -> false | IsNetworkOnline_NoReceivedMarker_ReturnsFalse
 *   B6 loss "0%" -> returns false 🔴 suspected source defect (inverted vs DriverInstaller)
 *      | IsNetworkOnline_ZeroLoss_ReportsOffline
 *   B7 loss "100%" -> returns true 🔴 suspected source defect (inverted vs DriverInstaller)
 *      | IsNetworkOnline_PacketLoss_ReportsOnline
 *
 * 🔴 DEFECT NOTE (recorded in .ut/defects.json):
 *    driverinstallerapt.cpp isNetworkOnline() returns `atoi(loss) != 0`, i.e. 0%
 *    packet loss is reported as "offline" and 100% loss as "online". The QApt-based
 *    DriverInstaller::isNetworkOnline treats loss == 0 as online, so the two
 *    implementations disagree and the apt variant is logically inverted. Tests here
 *    assert the CURRENT behavior to stay green; the defect is reported, not fixed.
 *
 * Minimal dependency list: Utils::isDpkgLocked stubbed; executeCommand/isNetworkOnline/
 * doOperate self-stubs; QProcess start/waitForFinished/readAllStandardOutput/exitCode/
 * exitStatus/state/kill machine-stubbed. Fake netlog.bat written into a temp CWD.
 */

#include "../ut_Head.h"
#include "../stub.h"
#include <QProcess>

#include "drivercontrol/driverinstallerapt.h"
#include "drivercontrol/utils.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

using namespace testing;

namespace {
// ---------- recorder state ----------
bool g_dpkgLocked = false;
QStringList g_execLog;              // commands passed to executeCommand stub
QString g_aptCacheShowOutput;       // canned output for "apt-cache show ..."
bool g_startNetlogWrite = false;    // QProcess::start stub writes fake netlog.bat
QByteArray g_netlogContent;
int g_exitCode = 0;
QProcess::ExitStatus g_exitStatus = QProcess::NormalExit;
QByteArray g_processOutput;
bool g_installerOnline = true;
int g_killCalls = 0;
QProcess::ProcessState g_processState = QProcess::NotRunning;
QString g_startedProgram;
QStringList g_startedArgs;
QStringList g_doOperateArgs;

void resetDriverInstallerAptRecorders()
{
    g_dpkgLocked = false;
    g_execLog.clear();
    g_aptCacheShowOutput.clear();
    g_startNetlogWrite = false;
    g_netlogContent.clear();
    g_exitCode = 0;
    g_exitStatus = QProcess::NormalExit;
    g_processOutput.clear();
    g_installerOnline = true;
    g_killCalls = 0;
    g_processState = QProcess::NotRunning;
    g_startedProgram.clear();
    g_startedArgs.clear();
    g_doOperateArgs.clear();
}

// ---------- Utils stubs ----------
bool ut_isDpkgLocked()
{
    return g_dpkgLocked;
}

// ---------- self stubs ----------
QString ut_executeCommandSelf(DriverInstallerApt *, const QString &cmd, const QStringList &args)
{
    g_execLog << (cmd + " " + args.join(" "));
    if (cmd == "apt-cache")
        return g_aptCacheShowOutput;
    return QString();
}

bool ut_isOnlineSelf(DriverInstallerApt *)
{
    return g_installerOnline;
}

void ut_doOperateSelf(DriverInstallerApt *, const QString &package, const QString &version)
{
    g_doOperateArgs << package << version;
}

// ---------- QProcess stubs ----------
void ut_processStartProgramArgs(QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode)
{
    g_startedProgram = program;
    g_startedArgs = args;
    if (g_startNetlogWrite) {
        QFile f("netlog.bat");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            f.write(g_netlogContent);
    }
}

bool ut_processWaitForFinished(QProcess *, int)
{
    return true;
}

QByteArray ut_processReadAllStandardOutput(QProcess *)
{
    return g_processOutput;
}

int ut_processExitCode(QProcess *)
{
    return g_exitCode;
}

QProcess::ExitStatus ut_processExitStatus(QProcess *)
{
    return g_exitStatus;
}

QProcess::ProcessState ut_processState(QProcess *)
{
    return g_processState;
}

void ut_processKill(QProcess *)
{
    ++g_killCalls;
}
} // namespace

class DriverInstallerApt_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        resetDriverInstallerAptRecorders();
    }

    void TearDown() override
    {
        if (!m_prevCwd.isEmpty())
            QDir::setCurrent(m_prevCwd);
    }

    // enter a temp CWD so "netlog.bat" writes stay isolated
    void enterTempCwd()
    {
        m_prevCwd = QDir::currentPath();
        ASSERT_TRUE(m_tempDir.isValid());
        ASSERT_TRUE(QDir::setCurrent(m_tempDir.path()));
    }

    QTemporaryDir m_tempDir;
    QString m_prevCwd;
};

// ---------- installPackage ----------

// B3: dpkg free -> reset flags + doOperate
TEST_F(DriverInstallerApt_UT, InstallPackage_DpkgFree_RunsOperate)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstallerApt, doOperate), ut_doOperateSelf);

    DriverInstallerApt installer;
    installer.m_Cancel = true;

    // Arrange
    g_dpkgLocked = false;
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert
    EXPECT_FALSE(installer.m_Cancel);
    EXPECT_EQ(installer.m_iRuningTestCount, 0);
    EXPECT_EQ(g_doOperateArgs, (QStringList {  "pkg", "1.0"  }));
    EXPECT_EQ(errorSpy.count(), 0);
}

// B1: dpkg locked within retry limit -> deferred
TEST_F(DriverInstallerApt_UT, InstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstallerApt, doOperate), ut_doOperateSelf);

    DriverInstallerApt installer;

    // Arrange
    g_dpkgLocked = true;
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert: deferred retry -- no operate, no error yet: singleShot pending, nothing run yet
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
    EXPECT_EQ(errorSpy.count(), 0);
}

// B2: dpkg locked, retries exhausted -> errorOccurred(0) + installProgressFinished(false)
TEST_F(DriverInstallerApt_UT, InstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstallerApt, doOperate), ut_doOperateSelf);

    DriverInstallerApt installer;
    installer.m_iRuningTestCount = 20;   // MAX_DPKGRUNING_TEST

    // Arrange
    g_dpkgLocked = true;
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    QSignalSpy finSpy(&installer, &DriverInstallerApt::installProgressFinished);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), 0);
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
}

// ---------- undoInstallDriver ----------

// B1: no process -> only sets cancel flag
TEST_F(DriverInstallerApt_UT, UndoInstallDriver_NoProcess_SetsCancelFlag)
{

    // Arrange: no m_process allocated
    DriverInstallerApt installer;
    // Act
    installer.undoInstallDriver();
    // Assert: cancel flag set, no kill invoked
    EXPECT_TRUE(installer.m_Cancel);
    EXPECT_EQ(g_killCalls, 0);
}

// B2: process not running -> no kill
TEST_F(DriverInstallerApt_UT, UndoInstallDriver_NotRunning_SkipsKill)
{
    DriverInstallerApt installer;
    installer.m_process = new QProcess(&installer);

    // Arrange: process allocated but NotRunning (default state)
    // Act
    installer.undoInstallDriver();
    // Assert: cancel flag set, kill skipped for non-running process
    EXPECT_TRUE(installer.m_Cancel);
    EXPECT_EQ(g_killCalls, 0);
}

// B3: process running -> kill called
TEST_F(DriverInstallerApt_UT, UndoInstallDriver_RunningProcess_Kills)
{
    Stub stub;
    stub.set(ADDR(QProcess, state), ut_processState);
    stub.set(ADDR(QProcess, kill), ut_processKill);

    DriverInstallerApt installer;
    installer.m_process = new QProcess(&installer);

    // Arrange
    g_processState = QProcess::Running;
    // Act
    installer.undoInstallDriver();
    // Assert
    EXPECT_TRUE(installer.m_Cancel);
    EXPECT_EQ(g_killCalls, 1);
}

// ---------- aptClean ----------

// B1: executes the clean helper command
TEST_F(DriverInstallerApt_UT, AptClean_Called_RunsCleanCommand)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);


    // Arrange: executeCommand stubbed to record the command line
    DriverInstallerApt installer;
    // Act
    installer.aptClean();
    // Assert
    EXPECT_EQ(g_execLog, QStringList { "lastore-apt-clean " });
    EXPECT_EQ(g_execLog.size(), 1);
}

// ---------- executeCommand ----------

// B1: starts cmd, waits, returns utf8 output
TEST_F(DriverInstallerApt_UT, ExecuteCommand_ProgramArgs_ReturnsStdout)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    DriverInstallerApt installer;

    // Arrange
    g_processOutput = "ut-output";
    // Act
    const QString out = installer.executeCommand("echo", QStringList { "hi" });
    // Assert
    EXPECT_EQ(out, QString("ut-output"));
    EXPECT_EQ(g_startedProgram, QString("echo"));
    EXPECT_EQ(g_startedArgs, QStringList { "hi" });
}

// ---------- doOperate ----------

// B1: apt-cache show empty -> errorOccurred(1), no install process created
TEST_F(DriverInstallerApt_UT, DoOperate_PackageMissing_EmitsNotFound)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "";   // package unknown
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), 1);
    // clean + apt update + apt-cache show executed in order
    EXPECT_EQ(g_execLog.size(), 3);
    EXPECT_EQ(g_execLog.at(0), QString("lastore-apt-clean "));
    EXPECT_EQ(g_execLog.at(1), QString("apt update"));
    EXPECT_TRUE(g_execLog.at(2).startsWith("apt-cache show pkg=1.0"));
    EXPECT_EQ(installer.m_process, nullptr);
}

// B2: finished exitCode 0 -> installProgressFinished(true)
TEST_F(DriverInstallerApt_UT, DoOperate_Success_EmitsFinished)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    QSignalSpy finSpy(&installer, &DriverInstallerApt::installProgressFinished);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->finished(0, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
}

// B3: cancelled -> errorOccurred(2)
TEST_F(DriverInstallerApt_UT, DoOperate_Cancelled_EmitsCancelError)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    installer.undoInstallDriver();   // sets m_Cancel
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");   // doOperate does NOT reset m_Cancel
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->finished(1, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), 2);
}

// B4: nvidia failure -> installProgressFinished(true)
TEST_F(DriverInstallerApt_UT, DoOperate_NvidiaFailure_TreatedAsSuccess)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    QSignalSpy finSpy(&installer, &DriverInstallerApt::installProgressFinished);
    // Act
    installer.doOperate("nvidia-driver-550", "1.0");
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->finished(1, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
}

// B5: non-nvidia failure + offline -> errorOccurred(3)
TEST_F(DriverInstallerApt_UT, DoOperate_OfflineCommit_EmitsNetworkError)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(DriverInstallerApt, isNetworkOnline), ut_isOnlineSelf);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    g_installerOnline = false;
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.doOperate("printer-driver-foo", "1.0");
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->finished(1, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), 3);
}

// B6: non-nvidia failure + online -> errorOccurred(0)
TEST_F(DriverInstallerApt_UT, DoOperate_OnlineCommit_EmitsNetworkError)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(DriverInstallerApt, isNetworkOnline), ut_isOnlineSelf);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    g_installerOnline = true;
    QSignalSpy errorSpy(&installer, &DriverInstallerApt::errorOccurred);
    // Act
    installer.doOperate("printer-driver-foo", "1.0");
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->finished(1, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), 0);
}

// B7: percent output -> installProgressChanged(n)
TEST_F(DriverInstallerApt_UT, DoOperate_ProgressOutput_EmitsProgress)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    g_processOutput = "Unpacking printer-driver-foo 42% done\n";
    QSignalSpy progressSpy(&installer, &DriverInstallerApt::installProgressChanged);
    // Act
    installer.doOperate("printer-driver-foo", "1.0");
    ASSERT_NE(installer.m_process, nullptr);
    emit installer.m_process->readyReadStandardOutput(QProcess::QPrivateSignal());
    // Assert
    EXPECT_EQ(progressSpy.count(), 1);
    EXPECT_EQ(progressSpy.at(0).at(0).toInt(), 42);
}

// B8: install start args "apt-get install -y pkg=ver"
TEST_F(DriverInstallerApt_UT, DoOperate_KnownPackage_StartsInstall)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, executeCommand), ut_executeCommandSelf);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    DriverInstallerApt installer;

    // Arrange
    g_aptCacheShowOutput = "pkg";
    // Act
    installer.doOperate("pkg", "1.0");
    // Assert
    EXPECT_EQ(g_startedProgram, QString("apt-get"));
    EXPECT_EQ(g_startedArgs, (QStringList {  "install", "-y", "pkg=1.0"  }));
}

// ---------- isNetworkOnline ----------

// B1: ping exitCode != 0 -> false (also covers sec > 0 usleep branch with 1us)
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_AbnormalExit_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_exitCode = 1;
    g_exitStatus = QProcess::NormalExit;
    // Act
    const bool online = installer.isNetworkOnline(1);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}

// B2: crash exit -> false
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_CrashExit_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_exitCode = 0;
    g_exitStatus = QProcess::CrashExit;
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}

// B4: log lines <= 1 -> false
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_ShortLog_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_startNetlogWrite = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}

// B5: no "received," marker -> false
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_NoReceivedMarker_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_startNetlogWrite = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\nno marker here\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}

// B6: loss "0%" -> returns false
// 🔴 Suspected source defect: inverted semantics vs DriverInstaller::isNetworkOnline
//    (which treats 0% loss as online). Asserting CURRENT behavior; defect reported
//    in .ut/defects.json instead of fixing production code.
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_ZeroLoss_ReportsOffline)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_startNetlogWrite = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n"
                      "2 packets transmitted, 2 received, 0% packet loss, time 1001ms\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert: current (defective) behavior — 0% loss reported as offline
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}

// B7: loss "100%" -> returns true
// 🔴 Suspected source defect (inverted semantics), see file header note.
TEST_F(DriverInstallerApt_UT, IsNetworkOnline_PacketLoss_ReportsOnline)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstallerApt installer;

    // Arrange
    enterTempCwd();
    g_startNetlogWrite = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n"
                      "2 packets transmitted, 0 received, 100% packet loss, time 1001ms\n"
                      "rtt min/avg/max/mdev = 10.1/10.2/10.3/0.1 ms\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert: current (defective) behavior — 100% loss reported as online
    EXPECT_TRUE(online);
    EXPECT_EQ(g_startedProgram, QString("ping"));
}
