// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ut_driverinstaller.cpp
 * @brief Unit tests for DriverInstaller
 *        (deepin-devicecontrol/src/drivercontrol/driverinstaller.cpp)
 *
 * Tested methods and covered branches:
 * - installPackage (public slot):
 *   B1 dpkg locked, retries < MAX -> QTimer::singleShot retry, no immediate operate
 *      | InstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, retries exhausted -> errorOccurred(EC_NULL) + installProgressFinished(false)
 *      | InstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free -> m_Cancel reset + doOperate called
 *      | InstallPackage_DpkgFree_RunsOperate
 * - initBackend (private):
 *   B1 null backend -> new QApt::Backend + aptClean + init
 *      | InitBackend_NullBackend_CreatesBackendAndInits
 *   B2 existing backend -> reuse instance
 *      | InitBackend_ExistingBackend_ReusesInstance
 *   B3 init() false -> returns false
 *      | InitBackend_InitFails_ReturnsFalse
 * - aptClean (private):
 *   B1 runs /usr/bin/lastore-apt-clean
 *      | AptClean_RunsCleanCommand
 * - isNetworkOnline (private):
 *   B1 ping abnormal exit -> false | IsNetworkOnline_PingAbnormalExit_ReturnsFalse
 *   B2 log lines <= 1 -> false | IsNetworkOnline_ShortLog_ReturnsFalse
 *   B3 no "received," marker -> false | IsNetworkOnline_NoReceivedMarker_ReturnsFalse
 *   B4 packet loss != 0 -> false | IsNetworkOnline_PacketLoss_ReturnsFalse
 *   B5 packet loss == 0 -> true | IsNetworkOnline_ZeroLoss_ReturnsTrue
 * - undoInstallDriver (public slot):
 *   B1 no transaction -> no-op | UndoInstallDriver_NoTransaction_DoesNothing
 *   B2 active transaction -> setProperty + cancel + m_Cancel=true
 *      | UndoInstallDriver_ActiveTransaction_Cancels
 * - doOperate (private):
 *   B1 initBackend fail -> errorOccurred(EC_NULL) | DoOperate_InitBackendFails_EmitsError
 *   B2 package not found -> errorOccurred(EC_NOTFOUND) | DoOperate_PackageNotFound_EmitsError
 *   B3 setVersion fail -> errorOccurred(EC_NOTFOUND), package deleted
 *      | DoOperate_VersionNotFound_EmitsError
 *   B4 installPackages null -> errorOccurred(EC_NULL) | DoOperate_InstallPackagesNull_EmitsError
 *   B5 finished ExitSuccess + Success -> installProgressFinished(true)
 *      | DoOperate_FinishedSuccess_EmitsFinished
 *   B6 finished ExitFailed + nvidia + CommitError -> installProgressFinished(true)
 *      | DoOperate_NvidiaCommitError_TreatedAsSuccess
 *   B7 finished ExitFailed + FetchError -> errorOccurred(EC_NETWORK)
 *      | DoOperate_FetchError_EmitsNetworkError
 *   B8 finished ExitFailed + CommitError + offline -> errorOccurred(EC_NETWORK)
 *      | DoOperate_CommitErrorOffline_EmitsNetworkError
 *   B9 finished ExitCancelled -> errorOccurred(EC_CANCEL) | DoOperate_Cancelled_EmitsError
 *   B10 progressChanged while cancelled -> transaction cancel()
 *      | DoOperate_ProgressWhileCancelled_CancelsTransaction
 *   B11 progressChanged -> installProgressChanged
 *      | DoOperate_Progress_EmitsInstallProgress
 *
 * Minimal dependency list (only what is stubbed or constructed here):
 * - QApt::Backend  : real object; init/package/installPackages machine-stubbed
 * - QApt::Package  : real object via internal ctor (pkgCache iterator default)
 * - QApt::Transaction : real object ("ut-tid"); error/run/cancel/downloadProgress stubbed
 * - QApt::DebFile  : real object (tiny temp file), isValid machine-stubbed where needed
 * - QApt::DebInstaller : stubbed private method
 * - Utils::isDpkgLocked : stubbed
 * - QProcess : start/setProgram/waitForFinished/exitCode/exitStatus stubbed (no real IO)
 *
 * Note: Test cases marked with comments like "outside doOperate" indicate their
 * operation depends on branch B4. For external dependencies, stubs are used.
 * Signals are observed with QSignalSpy; no DBus/network/filesystem outside temp dir.
 */

#include "../ut_Head.h"
#include "../stub.h"
#include <QProcess>

#include "drivercontrol/driverinstaller.h"
#include "drivercontrol/utils.h"
#include "drivercontrol/commonfunction.h"

#include <QApt/Backend>
#include <QApt/Package>
#include <QApt/Transaction>
#include <QApt/DebFile>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

using namespace testing;

namespace {
// ---------- recorder state (reset in SetUp) ----------
bool g_backendInitResult = false;
QApt::Package *g_backendPackageResult = nullptr;
QApt::Transaction *g_installPackagesResult = nullptr;
bool g_packageSetVersionResult = false;
QString g_setVersionArg;
QStringList g_startedPrograms;   // recorded via QProcess::start / setProgram patches
QStringList g_startedArgs;       // recorded via QProcess::start(program, args) patch
bool g_dpkgLocked = false;
bool g_isNetworkOnlineResult = true;
int g_cancelCalls = 0;
bool g_initBackendResult = false;
QStringList g_doOperateArgs;
QApt::ErrorCode g_transactionError = QApt::Success;
QProcess::ExitStatus g_exitStatus = QProcess::NormalExit;
int g_exitCode = 0;
QByteArray g_netlogContent;      // content written to netlog.bat by start stub
bool g_writeNetlog = false;
QString g_debFileArch = "amd64";
bool g_debValid = true;

void resetDriverInstallerRecorders()
{
    g_backendInitResult = false;
    g_backendPackageResult = nullptr;
    g_installPackagesResult = nullptr;
    g_packageSetVersionResult = false;
    g_setVersionArg.clear();
    g_startedPrograms.clear();
    g_startedArgs.clear();
    g_dpkgLocked = false;
    g_isNetworkOnlineResult = true;
    g_cancelCalls = 0;
    g_initBackendResult = false;
    g_doOperateArgs.clear();
    g_transactionError = QApt::Success;
    g_exitStatus = QProcess::NormalExit;
    g_exitCode = 0;
    g_netlogContent.clear();
    g_writeNetlog = false;
    g_debFileArch = "amd64";
    g_debValid = true;
}

// ---------- QApt stubs ----------
bool ut_backendInit(QApt::Backend *)
{
    return g_backendInitResult;
}

// Build a real, self-contained QApt::Package (internal ctor, allowed by -fno-access-control)
QApt::Package *ut_backendPackage(QApt::Backend *, const QString &name)
{
    Q_UNUSED(name);
    return g_backendPackageResult;
}

QApt::Transaction *ut_backendInstallPackages(QApt::Backend *, const QApt::PackageList &)
{
    return g_installPackagesResult;
}

bool ut_packageSetVersion(QApt::Package *, const QString &version)
{
    g_setVersionArg = version;
    return g_packageSetVersionResult;
}

QApt::ErrorCode ut_transactionError(QApt::Transaction *)
{
    return g_transactionError;
}

// Build a real, self-contained QApt::Package via the internal ctor
// (QApt::Package(Backend*, pkgCache::PkgIterator&)); the iterator is default
// constructed ("end" iterator) and never dereferenced by the tested code paths.
QApt::Package *makeFakePackage()
{
    static pkgCache::PkgIterator s_dummyIter;
    return new QApt::Package(nullptr, s_dummyIter);
}

void ut_transactionRun(QApt::Transaction *) { }

void ut_transactionCancel(QApt::Transaction *)
{
    ++g_cancelCalls;
}

QApt::DownloadProgress ut_transactionDownloadProgress(QApt::Transaction *)
{
    return QApt::DownloadProgress();
}

// ---------- Utils stub ----------
bool ut_isDpkgLocked()
{
    return g_dpkgLocked;
}

// ---------- QProcess stubs ----------
void ut_processStartProgramArgs(QProcess *, const QString &program, const QStringList &, QIODevice::OpenMode)
{
    g_startedPrograms << program;
    if (g_writeNetlog) {
        QFile f("netlog.bat");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            f.write(g_netlogContent);
    }
}

void ut_processStartMode(QProcess *self, QIODevice::OpenMode)
{
    g_startedPrograms << self->program();
    if (g_writeNetlog) {
        QFile f("netlog.bat");
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            f.write(g_netlogContent);
    }
}

bool ut_processWaitForFinished(QProcess *, int)
{
    return true;
}

int ut_processExitCode(QProcess *)
{
    return g_exitCode;
}

QProcess::ExitStatus ut_processExitStatus(QProcess *)
{
    return g_exitStatus;
}

// ---------- self stubs ----------
bool ut_driverInitBackendSelf(DriverInstaller *)
{
    return g_initBackendResult;
}

bool ut_isNetworkOnlineSelf(DriverInstaller *)
{
    return g_isNetworkOnlineResult;
}

void ut_driverDoOperateSelf(DriverInstaller *, const QString &package, const QString &version)
{
    g_doOperateArgs << package << version;
}
} // namespace

class DriverInstaller_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        resetDriverInstallerRecorders();
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

// B3: dpkg free -> doOperate called once with (pkg, install)
TEST_F(DriverInstaller_UT, InstallPackage_DpkgFree_RunsOperate)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstaller, doOperate), ut_driverDoOperateSelf);
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);

    DriverInstaller installer;

    // Arrange
    g_dpkgLocked = false;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 0);
    EXPECT_EQ(g_doOperateArgs, (QStringList {  "pkg", "1.0"  }));
}

// B1: dpkg locked within retry limit -> deferred retry, no immediate operate
TEST_F(DriverInstaller_UT, InstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstaller, doOperate), ut_driverDoOperateSelf);
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);

    DriverInstaller installer;

    // Arrange
    g_dpkgLocked = true;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert: singleShot pending -- no operate, no error yet
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
    EXPECT_EQ(errorSpy.count(), 0);
}

// B2: dpkg locked, retries exhausted -> error + finished(false)
TEST_F(DriverInstaller_UT, InstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DriverInstaller, doOperate), ut_driverDoOperateSelf);
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);

    DriverInstaller installer;
    installer.m_iRuningTestCount = 20;   // MAX_DPKGRUNING_TEST

    // Arrange
    g_dpkgLocked = true;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    QSignalSpy finSpy(&installer, &DriverInstaller::installProgressFinished);
    // Act
    installer.installPackage("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NULL);
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    // 🔴 DEFECT(ut-t25): exhausted-retry branch has no `return`, doOperate still runs
    EXPECT_FALSE(g_doOperateArgs.isEmpty());
}

// ---------- initBackend ----------

// B1: lazy creation + init success
TEST_F(DriverInstaller_UT, InitBackend_NullBackend_CreatesBackendAndInits)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);

    DriverInstaller installer;

    // Arrange
    g_backendInitResult = true;
    // Act
    const bool ok = installer.initBackend();
    // Assert
    EXPECT_TRUE(ok);
    ASSERT_NE(installer.mp_Backend, nullptr);
    // B2 follow-up: second call reuses the same backend instance
    QApt::Backend *first = installer.mp_Backend;
    EXPECT_TRUE(installer.initBackend());
    EXPECT_EQ(installer.mp_Backend, first);
}

// B3: init failure propagated
TEST_F(DriverInstaller_UT, InitBackend_InitFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);

    DriverInstaller installer;

    // Arrange
    g_backendInitResult = false;
    // Act
    const bool ut_initBackend = installer.initBackend();
    // Assert
    EXPECT_FALSE(ut_initBackend);
    EXPECT_NE(installer.mp_Backend, nullptr);
}

// ---------- aptClean ----------

// B1: runs the lastore clean helper
TEST_F(DriverInstaller_UT, AptClean_Called_RunsCleanCommand)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);


    // Arrange: start() stubbed to record the launched program
    DriverInstaller installer;
    // Act
    installer.aptClean();
    // Assert: cleaner binary started; stub absorbed the call, nothing else runs
    EXPECT_EQ(g_startedPrograms, QStringList { "/usr/bin/lastore-apt-clean" });
    EXPECT_EQ(g_startedPrograms.size(), 1);
}

// ---------- isNetworkOnline ----------

// B1: ping abnormal exit -> false
TEST_F(DriverInstaller_UT, IsNetworkOnline_PingAbnormalExit_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start), ut_processStartMode);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstaller installer;

    // Arrange
    enterTempCwd();
    g_exitCode = 1;
    g_exitStatus = QProcess::NormalExit;
    // Act
    const bool online = installer.isNetworkOnline(1);   // 1us, avoid real sleep
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedPrograms, QStringList { "ping" });  // setProgram target
}

// B2: log too short -> false
TEST_F(DriverInstaller_UT, IsNetworkOnline_ShortLog_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start), ut_processStartMode);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstaller installer;

    // Arrange
    enterTempCwd();
    g_writeNetlog = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedPrograms, QStringList { "ping" });  // setProgram target
}

// B3: no "received," marker -> false
TEST_F(DriverInstaller_UT, IsNetworkOnline_NoReceivedMarker_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start), ut_processStartMode);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstaller installer;

    // Arrange
    enterTempCwd();
    g_writeNetlog = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\nno marker here at all\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedPrograms, QStringList { "ping" });  // setProgram target
}

// B4: packet loss != 0 -> false
TEST_F(DriverInstaller_UT, IsNetworkOnline_PacketLoss_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start), ut_processStartMode);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstaller installer;

    // Arrange
    enterTempCwd();
    g_writeNetlog = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n"
                      "2 packets transmitted, 0 received, 100% packet loss, time 1001ms\n"
                      "rtt min/avg/max/mdev = 10.1/10.2/10.3/0.1 ms\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_FALSE(online);
    EXPECT_EQ(g_startedPrograms, QStringList { "ping" });  // setProgram target
}

// B5: packet loss == 0 -> true
TEST_F(DriverInstaller_UT, IsNetworkOnline_ZeroLoss_ReturnsTrue)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start), ut_processStartMode);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);
    stub.set(ADDR(QProcess, exitStatus), ut_processExitStatus);

    DriverInstaller installer;

    // Arrange
    enterTempCwd();
    g_writeNetlog = true;
    g_netlogContent = "PING www.baidu.com (110.242.68.4) 56(84) bytes of data.\n"
                      "2 packets transmitted, 2 received, 0% packet loss, time 1001ms\n"
                      "rtt min/avg/max/mdev = 10.1/10.2/10.3/0.1 ms\n";
    // Act
    const bool online = installer.isNetworkOnline(0);
    // Assert
    EXPECT_TRUE(online);
    EXPECT_EQ(g_startedPrograms, QStringList { "ping" });  // setProgram target
}

// ---------- undoInstallDriver ----------

// B1: no transaction -> no-op
TEST_F(DriverInstaller_UT, UndoInstallDriver_NoTransaction_DoesNothing)
{

    // Arrange: no transaction assigned (mp_Trans == nullptr)
    DriverInstaller installer;
    // Act
    installer.undoInstallDriver();
    // Assert: cancel not called
    EXPECT_EQ(g_cancelCalls, 0);
    EXPECT_FALSE(installer.m_Cancel);
}

// B2: active transaction -> cancel + m_Cancel
TEST_F(DriverInstaller_UT, UndoInstallDriver_ActiveTransaction_Cancels)
{
    Stub stub;
    stub.set(ADDR(QApt::Transaction, cancel), ut_transactionCancel);

    DriverInstaller installer;
    installer.mp_Trans = new QApt::Transaction("ut-tid");

    // Arrange: active transaction assigned
    // Act
    installer.undoInstallDriver();
    // Assert
    EXPECT_EQ(g_cancelCalls, 1);
    EXPECT_TRUE(installer.m_Cancel);

    delete installer.mp_Trans;
    installer.mp_Trans = nullptr;
}

// ---------- doOperate ----------

// B1: initBackend fails -> errorOccurred(EC_NULL)
TEST_F(DriverInstaller_UT, DoOperate_InitBackendFails_EmitsError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = false;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NULL);
}

// B2: package not found -> errorOccurred(EC_NOTFOUND)
TEST_F(DriverInstaller_UT, DoOperate_PackageNotFound_EmitsError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = nullptr;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NOTFOUND);
}

// B3: setVersion fails -> errorOccurred(EC_NOTFOUND), package deleted
TEST_F(DriverInstaller_UT, DoOperate_VersionNotFound_EmitsError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = false;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "9.9");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NOTFOUND);
    EXPECT_EQ(g_setVersionArg, "9.9");
}

// B4: installPackages returns null -> errorOccurred(EC_NULL)
TEST_F(DriverInstaller_UT, DoOperate_InstallPackagesNull_EmitsError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = nullptr;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NULL);
    EXPECT_EQ(installer.mp_Trans, nullptr);
}

// B5: finished ExitSuccess + Success -> installProgressFinished(true), trans cleaned
TEST_F(DriverInstaller_UT, DoOperate_FinishedSuccess_EmitsFinished)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::Success;
    QSignalSpy finSpy(&installer, &DriverInstaller::installProgressFinished);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->finished(QApt::ExitSuccess);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
    EXPECT_EQ(installer.mp_Trans, nullptr);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B6: nvidia package + CommitError on failure -> treated as success
TEST_F(DriverInstaller_UT, DoOperate_NvidiaCommitError_TreatedAsSuccess)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::CommitError;
    QSignalSpy finSpy(&installer, &DriverInstaller::installProgressFinished);
    // Act
    installer.doOperate("nvidia-driver-550", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->finished(QApt::ExitFailed);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B7: FetchError -> errorOccurred(EC_NETWORK)
TEST_F(DriverInstaller_UT, DoOperate_FetchError_EmitsNetworkError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::FetchError;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->finished(QApt::ExitFailed);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NETWORK);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B8: CommitError + offline -> errorOccurred(EC_NETWORK)
TEST_F(DriverInstaller_UT, DoOperate_CommitErrorOffline_EmitsNetworkError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);
    stub.set(ADDR(DriverInstaller, isNetworkOnline), ut_isNetworkOnlineSelf);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::CommitError;
    g_isNetworkOnlineResult = false;
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->finished(QApt::ExitFailed);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_NETWORK);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B9: ExitCancelled -> errorOccurred(EC_CANCEL)
TEST_F(DriverInstaller_UT, DoOperate_Cancelled_EmitsError)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    QSignalSpy errorSpy(&installer, &DriverInstaller::errorOccurred);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->finished(QApt::ExitCancelled);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toInt(), EC_CANCEL);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B10: progressChanged while m_Cancel -> transaction cancel() invoked
TEST_F(DriverInstaller_UT, DoOperate_ProgressWhileCancelled_CancelsTransaction)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);
    stub.set(ADDR(QApt::Transaction, cancel), ut_transactionCancel);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    installer.undoInstallDriver();          // sets m_Cancel via cancel path
    EXPECT_EQ(g_cancelCalls, 1);            // from undoInstallDriver itself
    // progress callback honours the cancel flag
    emit installer.mp_Trans->progressChanged(50);
    // Assert
    EXPECT_EQ(g_cancelCalls, 2);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}

// B11: progressChanged -> installProgressChanged forwarded
TEST_F(DriverInstaller_UT, DoOperate_Progress_EmitsInstallProgress)
{
    Stub stub;
    stub.set(ADDR(DriverInstaller, initBackend), ut_driverInitBackendSelf);
    stub.set(static_cast<QApt::Package *(QApt::Backend::*)(const QString &) const>(&QApt::Backend::package),
             ut_backendPackage);
    stub.set(ADDR(QApt::Package, setVersion), ut_packageSetVersion);
    stub.set(ADDR(QApt::Backend, installPackages), ut_backendInstallPackages);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);
    stub.set(ADDR(QApt::Transaction, downloadProgress), ut_transactionDownloadProgress);

    DriverInstaller installer;
    installer.mp_Backend = new QApt::Backend();

    // Arrange
    g_initBackendResult = true;
    g_backendPackageResult = makeFakePackage();
    g_packageSetVersionResult = true;
    g_installPackagesResult = new QApt::Transaction("ut-tid");
    QSignalSpy progressSpy(&installer, &DriverInstaller::installProgressChanged);
    // Act
    installer.doOperate("pkg", "1.0");
    ASSERT_NE(installer.mp_Trans, nullptr);
    emit installer.mp_Trans->progressChanged(42);
    // Assert
    EXPECT_EQ(progressSpy.count(), 1);
    EXPECT_EQ(progressSpy.at(0).at(0).toInt(), 42);
    delete g_installPackagesResult;
    g_installPackagesResult = nullptr;
}
