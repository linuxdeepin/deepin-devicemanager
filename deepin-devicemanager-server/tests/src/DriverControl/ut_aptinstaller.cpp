// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ut_aptinstaller.cpp
 * @brief Unit tests for AptInstaller
 *        (deepin-devicecontrol/src/drivercontrol/aptinstaller.cpp)
 *
 * Tested methods and covered branches:
 * - ctor: m_process created + signals wired | Ctor_StateInitialized
 * - isValid: returns cached m_bValid | IsValid_ReflectsCachedState
 * - doOperate (private):
 *   B1 install -> "apt install -y <pkg>" | DoOperate_Install_StartsAptInstallArgs
 *   B2 remove  -> "apt remove -y <pkg>"  | DoOperate_Remove_StartsAptRemoveArgs
 * - parseAptOutput (private):
 *   B1 output contains "<n>%" -> progressChanged(n) | ParseAptOutput_PercentMatch_EmitsProgress
 *   B2 no percent -> no emission | ParseAptOutput_NoPercent_NoEmission
 * - installPackage:
 *   B1 dpkg locked, retries < MAX -> deferred retry | InstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, exhausted -> errorOccurred("Dpkg locked, time out") + installFinished(false)
 *      | InstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free + m_bValid false -> skip | InstallPackage_InvalidState_SkipsOperate
 *   B4 dpkg free + m_bValid true -> doOperate(path, true) | InstallPackage_DpkgFree_RunsInstall
 * - uninstallPackage:
 *   B1 dpkg locked, retries < MAX -> deferred | UninstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, exhausted -> errorOccurred + installFinished(false)
 *      | UninstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free -> doOperate(pkg, false) | UninstallPackage_DpkgFree_RunsRemove
 * - isArchMatched:
 *   B1 no "Architecture:" info -> false | IsArchMatched_NoArchInfo_ReturnsFalse
 *   B2 pkg arch "all" -> true | IsArchMatched_AllArch_ReturnsTrue
 *   B3 pkg arch == system arch -> true | IsArchMatched_MatchedArch_ReturnsTrue
 *   B4 pkg arch != system arch -> false | IsArchMatched_UnmatchedArch_ReturnsFalse
 * - isDebValid:
 *   B1 file missing -> false | IsDebValid_MissingFile_ReturnsFalse
 *   B2 suffix not .deb -> false | IsDebValid_NonDebSuffix_ReturnsFalse
 *   B3 dpkg exit 0 -> true | IsDebValid_DpkgOk_ReturnsTrue
 *   B4 dpkg exit != 0 -> false | IsDebValid_DpkgFails_ReturnsFalse
 * - process signals (wired in ctor):
 *   B1 finished(0, NormalExit) -> installFinished(true) | ProcessFinished_Success_EmitsFinished
 *   B2 finished(non-zero) -> installFinished(false) + errorOccurred
 *      | ProcessFinished_Failure_EmitsFinishedAndError
 *   B3 readyReadStandardOutput -> parseAptOutput -> progressChanged
 *      | ReadyReadStandardOutput_ParsesProgress
 *
 * Minimal dependency list: Utils::isDpkgLocked / Utils::isFileLocked stubbed;
 * QProcess start/waitForFinished/readAllStandardOutput/exitCode machine-stubbed
 * (no real process is ever spawned). QSysInfo::currentCpuArchitecture is used real.
 */

#include "../ut_Head.h"
#include "../stub.h"
#include <QProcess>

#include "drivercontrol/aptinstaller.h"
#include "drivercontrol/utils.h"

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QSysInfo>

using namespace testing;

namespace {
// ---------- recorder state ----------
bool g_dpkgLocked = false;
bool g_fileLocked = false;
bool g_installerValid = true;
int g_exitCode = 0;
QByteArray g_processOutput;
QString g_startedProgram;
QStringList g_startedArgs;
bool g_installerDoOperateCalled = false;

void resetAptInstallerRecorders()
{
    g_dpkgLocked = false;
    g_fileLocked = false;
    g_installerValid = true;
    g_exitCode = 0;
    g_processOutput.clear();
    g_startedProgram.clear();
    g_startedArgs.clear();
    g_installerDoOperateCalled = false;
}

// ---------- Utils stubs ----------
bool ut_isDpkgLocked()
{
    return g_dpkgLocked;
}

bool ut_isFileLocked(const QString &, bool)
{
    return g_fileLocked;
}

// ---------- QProcess stubs ----------
void ut_processStartProgramArgs(QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode)
{
    g_startedProgram = program;
    g_startedArgs = args;
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
} // namespace

class AptInstaller_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        resetAptInstallerRecorders();
        m_dir = new QTemporaryDir;
    }

    void TearDown() override
    {
        delete m_dir;
        m_dir = nullptr;
    }

    // create a real file in the temp dir, return its path
    QString makeFile(const QString &name, const QByteArray &content)
    {
        QFile f(m_dir->filePath(name));
        EXPECT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content);
        f.close();
        return m_dir->filePath(name);
    }

    QTemporaryDir *m_dir = nullptr;
};

// ---------- ctor / isValid ----------

TEST_F(AptInstaller_UT, Ctor_BackendReady_StateInitialized)
{

    // Arrange: ctor ran initBackend with stubbed QApt::Backend::init
    AptInstaller installer;
    // Act: query post-ctor state
    const bool ut_isValid = installer.isValid();
    // Assert
    EXPECT_NE(installer.m_process, nullptr);
    EXPECT_TRUE(ut_isValid);
    EXPECT_EQ(installer.m_iRuningTestCount, 0);
}

TEST_F(AptInstaller_UT, IsValid_AfterInit_ReflectsCachedState)
{
    AptInstaller installer;

    // Arrange: cached flag drives the getter
    installer.m_bValid = false;
    // Act
    const bool ut_isValid = installer.isValid();
    // Assert
    EXPECT_FALSE(ut_isValid);
    installer.m_bValid = true;
    EXPECT_TRUE(installer.isValid());
}

// ---------- doOperate ----------

// B1: install args
TEST_F(AptInstaller_UT, DoOperate_Install_StartsAptInstallArgs)
    // Arrange
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Act
    installer.doOperate("printer-driver-foo", true);
    // Assert
    EXPECT_EQ(g_startedProgram, QString("apt"));
    EXPECT_EQ(g_startedArgs, (QStringList {  "install", "-y", "printer-driver-foo"  }));
}

// B2: remove args
TEST_F(AptInstaller_UT, DoOperate_Remove_StartsAptRemoveArgs)
    // Arrange
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Act
    installer.doOperate("printer-driver-foo", false);
    // Assert
    EXPECT_EQ(g_startedProgram, QString("apt"));
    EXPECT_EQ(g_startedArgs, (QStringList {  "remove", "-y", "printer-driver-foo"  }));
}

// ---------- parseAptOutput ----------

// B1: percent found -> progressChanged emitted
TEST_F(AptInstaller_UT, ParseAptOutput_PercentMatch_EmitsProgress)
{
    AptInstaller installer;

    // Arrange
    QSignalSpy progressSpy(&installer, &AptInstaller::progressChanged);
    // Act
    installer.parseAptOutput("Unpacking printer-driver-foo ... 37%");
    // Assert
    EXPECT_EQ(progressSpy.count(), 1);
    EXPECT_EQ(progressSpy.at(0).at(0).toInt(), 37);
}

// B2: no percent -> no emission
TEST_F(AptInstaller_UT, ParseAptOutput_NoPercent_NoEmission)
{
    AptInstaller installer;

    // Arrange
    QSignalSpy progressSpy(&installer, &AptInstaller::progressChanged);
    // Act
    installer.parseAptOutput("Reading package lists... Done");
    // Assert
    EXPECT_EQ(progressSpy.count(), 0);
    EXPECT_TRUE(progressSpy.isEmpty());
}

// ---------- installPackage ----------

// B4: dpkg free + valid -> starts install
TEST_F(AptInstaller_UT, InstallPackage_DpkgFree_RunsInstall)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Arrange
    g_dpkgLocked = false;
    // Act
    installer.installPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(g_startedProgram, QString("apt"));
    EXPECT_EQ(g_startedArgs, (QStringList {  "install", "-y", "printer-driver-foo"  }));
}

// B3: invalid state -> skip
TEST_F(AptInstaller_UT, InstallPackage_InvalidState_SkipsOperate)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Arrange
    installer.m_bValid = false;
    // Act
    installer.installPackage("printer-driver-foo");
    // Assert
    EXPECT_TRUE(g_startedProgram.isEmpty());
    EXPECT_TRUE(g_startedArgs.isEmpty());
}

// B1: dpkg locked within retry limit -> deferred
TEST_F(AptInstaller_UT, InstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Arrange
    g_dpkgLocked = true;
    // Act
    installer.installPackage("printer-driver-foo");
    // Assert: singleShot pending, nothing started yet
    EXPECT_TRUE(g_startedProgram.isEmpty());
    EXPECT_TRUE(g_startedArgs.isEmpty());
}

// B2: dpkg locked, retries exhausted -> error + finished(false)
TEST_F(AptInstaller_UT, InstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;
    installer.m_iRuningTestCount = 20;   // MAX_DPKGRUNING_TEST

    // Arrange
    g_dpkgLocked = true;
    QSignalSpy errorSpy(&installer, &AptInstaller::errorOccurred);
    QSignalSpy finSpy(&installer, &AptInstaller::installFinished);
    // Act
    installer.installPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Dpkg locked, time out"));
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_TRUE(g_startedProgram.isEmpty());
}

// ---------- uninstallPackage ----------

// B3: dpkg free -> starts remove
TEST_F(AptInstaller_UT, UninstallPackage_DpkgFree_RunsRemove)
{
    Stub stub;
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Arrange
    g_fileLocked = false;
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(g_startedProgram, QString("apt"));
    EXPECT_EQ(g_startedArgs, (QStringList {  "remove", "-y", "printer-driver-foo"  }));
}

// B1: dpkg locked within retry limit -> deferred
TEST_F(AptInstaller_UT, UninstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;

    // Arrange
    g_fileLocked = true;
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_TRUE(g_startedProgram.isEmpty());
    EXPECT_TRUE(g_startedArgs.isEmpty());
}

// B2: dpkg locked, retries exhausted -> error + finished(false)
TEST_F(AptInstaller_UT, UninstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);

    AptInstaller installer;
    installer.m_iRuningTestCount = 20;

    // Arrange
    g_fileLocked = true;
    QSignalSpy errorSpy(&installer, &AptInstaller::errorOccurred);
    QSignalSpy finSpy(&installer, &AptInstaller::installFinished);
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Dpkg locked, time out"));
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_TRUE(g_startedProgram.isEmpty());
}

// ---------- isArchMatched ----------

// B1: no architecture info -> false
TEST_F(AptInstaller_UT, IsArchMatched_NoArchInfo_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    AptInstaller installer;

    // Arrange
    g_processOutput = " new Debian package, version 2.0.\n";
    // Act
    // Assert
    EXPECT_FALSE(installer.isArchMatched(m_dir->filePath("ut.deb")));
    EXPECT_EQ(g_startedProgram, QString("dpkg"));
}

// B2: arch "all" -> true
TEST_F(AptInstaller_UT, IsArchMatched_AllArch_ReturnsTrue)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    AptInstaller installer;

    // Arrange
    g_processOutput = "Package: foo\nArchitecture: all\n";
    // Act
    // Assert
    EXPECT_TRUE(installer.isArchMatched(m_dir->filePath("ut.deb")));
    EXPECT_EQ(g_startedProgram, QString("dpkg"));
}

// B3: arch == system arch -> true
TEST_F(AptInstaller_UT, IsArchMatched_MatchedArch_ReturnsTrue)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    AptInstaller installer;

    // Arrange
    g_processOutput = QString("Package: foo\nArchitecture: %1\n")
                              .arg(QSysInfo::currentCpuArchitecture()).toUtf8();
    // Act
    // Assert
    EXPECT_TRUE(installer.isArchMatched(m_dir->filePath("ut.deb")));
    EXPECT_EQ(g_startedProgram, QString("dpkg"));
}

// B4: arch mismatch -> false
TEST_F(AptInstaller_UT, IsArchMatched_UnmatchedArch_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    AptInstaller installer;

    // Arrange
    g_processOutput = "Package: foo\nArchitecture: arm64\n";
    // Act
    // Assert
    EXPECT_FALSE(installer.isArchMatched(m_dir->filePath("ut.deb")));
    EXPECT_EQ(g_startedProgram, QString("dpkg"));
}

// ---------- isDebValid ----------

// B1: file missing -> false
TEST_F(AptInstaller_UT, IsDebValid_MissingFile_ReturnsFalse)
    // Arrange
{
    AptInstaller installer;

    // Act: rejected before any dpkg invocation (no QProcess started)
    // Assert
    EXPECT_FALSE(installer.isDebValid(m_dir->filePath("missing.deb")));
    EXPECT_TRUE(g_startedProgram.isEmpty());
}

// B2: suffix not .deb -> false
TEST_F(AptInstaller_UT, IsDebValid_NonDebSuffix_ReturnsFalse)
{
    AptInstaller installer;

    // Arrange: existing file without .deb suffix
    const QString path = makeFile("plain.txt", "x");
    // Act
    const bool ut_isDebValid = installer.isDebValid(path);
    // Assert
    EXPECT_FALSE(ut_isDebValid);
    EXPECT_TRUE(g_startedProgram.isEmpty());  // rejected on suffix before dpkg runs
}

// B3: dpkg exit 0 -> true
TEST_F(AptInstaller_UT, IsDebValid_DpkgOk_ReturnsTrue)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);

    AptInstaller installer;

    // Arrange
    const QString path = makeFile("ut.deb", "x");
    g_exitCode = 0;
    // Act
    const bool ut_isDebValid = installer.isDebValid(path);
    // Assert
    EXPECT_TRUE(ut_isDebValid);
    EXPECT_EQ(g_startedProgram, QString("dpkg"));  // dpkg -I queried
}

// B4: dpkg exit != 0 -> false
TEST_F(AptInstaller_UT, IsDebValid_DpkgFails_ReturnsFalse)
{
    Stub stub;
    stub.set(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
             ut_processStartProgramArgs);
    stub.set(ADDR(QProcess, waitForFinished), ut_processWaitForFinished);
    stub.set(ADDR(QProcess, exitCode), ut_processExitCode);

    AptInstaller installer;

    // Arrange
    const QString path = makeFile("ut.deb", "x");
    g_exitCode = 1;
    // Act
    const bool ut_isDebValid = installer.isDebValid(path);
    // Assert
    EXPECT_FALSE(ut_isDebValid);
    EXPECT_EQ(g_startedProgram, QString("dpkg"));  // dpkg -I queried before failing
}

// ---------- process signals (ctor wiring) ----------

// B1: finished(0, NormalExit) -> installFinished(true)
TEST_F(AptInstaller_UT, ProcessFinished_Success_EmitsFinished)
{
    AptInstaller installer;

    // Arrange
    QSignalSpy finSpy(&installer, &AptInstaller::installFinished);
    QSignalSpy errorSpy(&installer, &AptInstaller::errorOccurred);
    // Act
    emit installer.m_process->finished(0, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
    EXPECT_EQ(errorSpy.count(), 0);
}

// B2: finished(non-zero) -> installFinished(false) + errorOccurred
TEST_F(AptInstaller_UT, ProcessFinished_Failure_EmitsFinishedAndError)
{
    AptInstaller installer;

    // Arrange
    QSignalSpy finSpy(&installer, &AptInstaller::installFinished);
    QSignalSpy errorSpy(&installer, &AptInstaller::errorOccurred);
    // Act
    emit installer.m_process->finished(100, QProcess::NormalExit);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Process failed with exit code: 100"));
}

// B3: readyReadStandardOutput -> parseAptOutput -> progressChanged
TEST_F(AptInstaller_UT, ReadyReadStdout_PercentLine_EmitsProgress)
{
    Stub stub;
    stub.set(ADDR(QProcess, readAllStandardOutput), ut_processReadAllStandardOutput);

    AptInstaller installer;

    // Arrange
    QSignalSpy progressSpy(&installer, &AptInstaller::progressChanged);
    g_processOutput = "Unpacking foo ... 25%\n";
    // Act
    emit installer.m_process->readyReadStandardOutput(QProcess::QPrivateSignal());
    // Assert
    EXPECT_EQ(progressSpy.count(), 1);
    EXPECT_EQ(progressSpy.at(0).at(0).toInt(), 25);
}
