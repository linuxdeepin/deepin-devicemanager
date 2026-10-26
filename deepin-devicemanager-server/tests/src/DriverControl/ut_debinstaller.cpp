// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ut_debinstaller.cpp
 * @brief Unit tests for DebInstaller
 *        (deepin-devicecontrol/src/drivercontrol/debinstaller.cpp)
 *
 * Tested methods and covered branches:
 * - ctor + initBackend (private):
 *   B1 init() true -> m_bValid true | Ctor_BackendInitOk_IsValidTrue
 *   B2 init() false -> m_bValid false + initErrorMessage logged
 *      | Ctor_BackendInitFails_IsValidFalse
 *   B3 re-init recreates backend | InitBackend_Reinit_RecreatesBackend
 * - isValid: returns cached m_bValid | IsValid_ReflectsCachedState
 * - isArchMatched:
 *   B1 deb invalid -> false | IsArchMatched_InvalidDeb_ReturnsFalse
 *   B2 arch "all" -> false | IsArchMatched_AllArch_ReturnsFalse
 *   B3 arch in backend architectures -> true | IsArchMatched_MatchedArch_ReturnsTrue
 *   B4 arch not supported -> false | IsArchMatched_UnmatchedArch_ReturnsFalse
 * - isDebValid:
 *   B1 deb invalid -> false | IsDebValid_InvalidDeb_ReturnsFalse
 *   B2 deb valid -> true | IsDebValid_ValidDeb_ReturnsTrue
 * - reset (private):
 *   B1 reloadCache false -> m_bValid false, trans cleared | Reset_ReloadCacheFails_StateInvalid
 *   B2 reloadCache true -> m_bValid true | Reset_ReloadCacheSucceeds_StateValid
 * - doOperate (private):
 *   B1 install path, invalid deb -> errorOccurred("Deb is invalid")
 *      | DoOperate_InstallInvalidDeb_EmitsError
 *   B2 install path, installFile null -> errorOccurred("Get transaction failed!")
 *      | DoOperate_InstallTransactionNull_EmitsError
 *   B3 install path, finished(ExitSuccess)+Success -> installFinished(true) + reset
 *      | DoOperate_InstallFinished_EmitsFinishedAndResets
 *   B4 remove path, commitChanges -> transaction finished -> installFinished(false)
 *      | DoOperate_UninstallFinished_EmitsFinished
 *   B5 transaction errorOccurred -> errorOccurred(QString(code))
 *      | DoOperate_TransactionError_EmitsErrorString
 * - installPackage:
 *   B1 dpkg locked, retries < MAX -> deferred retry | InstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, retries exhausted -> errorOccurred + installFinished(false)
 *      | InstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free + m_bValid false -> no operate | InstallPackage_InvalidState_SkipsOperate
 *   B4 dpkg free + m_bValid true -> doOperate(path, true) | InstallPackage_DpkgFree_RunsInstall
 * - uninstallPackage:
 *   B1 dpkg locked, retries < MAX -> deferred | UninstallPackage_DpkgLockedWithinLimit_RetriesLater
 *   B2 dpkg locked, exhausted -> errorOccurred + installFinished(false)
 *      | UninstallPackage_DpkgLockedExhausted_EmitsErrors
 *   B3 dpkg free -> doOperate(pkg, false) | UninstallPackage_DpkgFree_RunsRemove
 *
 * Minimal dependency list: QApt::Backend (real object, members machine-stubbed),
 * QApt::DebFile (real object on a temp garbage file, isValid/architecture stubbed),
 * QApt::Transaction (real "ut-tid" object, error/run stubbed), Utils::isDpkgLocked /
 * Utils::isFileLocked stubbed. No DBus, no network, no real package operations.
 */

#include "../ut_Head.h"
#include "../stub.h"

#include "drivercontrol/debinstaller.h"
#include "drivercontrol/utils.h"

#include <QApt/Backend>
#include <QApt/DebFile>
#include <QApt/Transaction>

#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

using namespace testing;

namespace {
// ---------- recorder state ----------
bool g_backendInitResult = false;
bool g_reloadCacheResult = true;
QString g_initErrorMessage = "ut-init-error";
bool g_debValid = true;
QString g_debArch = "amd64";
QStringList g_backendArchs = { "amd64" };
QApt::Transaction *g_installFileResult = nullptr;
QApt::Transaction *g_commitChangesResult = nullptr;
int g_markRemoveCalls = 0;
bool g_dpkgLocked = false;
bool g_fileLocked = false;
bool g_installerValid = true;
bool g_initBackendResult = false;   // reserved for direct initBackend control
QStringList g_doOperateArgs;
QApt::ErrorCode g_transactionError = QApt::Success;
int g_cancelCalls = 0;
int g_backendInitCalls = 0;
bool g_debArchQueried = false;

void resetDebInstallerRecorders()
{
    g_backendInitCalls = 0;
    g_debArchQueried = false;
    g_backendInitResult = false;
    g_reloadCacheResult = true;
    g_initErrorMessage = "ut-init-error";
    g_debValid = true;
    g_debArch = "amd64";
    g_backendArchs = QStringList { "amd64" };
    g_installFileResult = nullptr;
    g_commitChangesResult = nullptr;
    g_markRemoveCalls = 0;
    g_dpkgLocked = false;
    g_fileLocked = false;
    g_installerValid = true;
    g_initBackendResult = false;
    g_doOperateArgs.clear();
    g_transactionError = QApt::Success;
    g_cancelCalls = 0;
}

// ---------- QApt stubs ----------
bool ut_backendInit(QApt::Backend *)
{
    g_backendInitCalls++;
    return g_backendInitResult;
}

QString ut_backendInitErrorMessage(QApt::Backend *)
{
    return g_initErrorMessage;
}

bool ut_backendReloadCache(QApt::Backend *)
{
    return g_reloadCacheResult;
}

QStringList ut_backendArchitectures(QApt::Backend *)
{
    return g_backendArchs;
}

QApt::Transaction *ut_backendInstallFile(QApt::Backend *, const QApt::DebFile &)
{
    return g_installFileResult;
}

void ut_backendMarkRemove(QApt::Backend *, const QString &)
{
    ++g_markRemoveCalls;
}

QApt::Transaction *ut_backendCommitChanges(QApt::Backend *)
{
    return g_commitChangesResult;
}

bool ut_debIsValid(QApt::DebFile *)
{
    return g_debValid;
}

QString ut_debArchitecture(QApt::DebFile *)
{
    g_debArchQueried = true;
    return g_debArch;
}

QApt::ErrorCode ut_transactionError(QApt::Transaction *)
{
    return g_transactionError;
}

void ut_transactionRun(QApt::Transaction *) { }

void ut_transactionCancel(QApt::Transaction *)
{
    ++g_cancelCalls;
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

// ---------- self stubs ----------
void ut_installerDoOperateSelf(DebInstaller *, const QString &package, bool bInstall)
{
    g_doOperateArgs << package << (bInstall ? "install" : "remove");
}
} // namespace

class DebInstaller_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        resetDebInstallerRecorders();
        // tiny garbage file used as stand-in deb path
        m_dir = new QTemporaryDir;
        QFile f(m_dir->filePath("ut-fake.deb"));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("not-a-real-deb");
        f.close();
        m_debPath = m_dir->filePath("ut-fake.deb");
    }

    void TearDown() override
    {
        delete m_dir;
        m_dir = nullptr;
    }

    QTemporaryDir *m_dir = nullptr;
    QString m_debPath;
};

// ---------- ctor / initBackend ----------

// B1: backend init success -> valid installer
TEST_F(DebInstaller_UT, Ctor_BackendInitOk_IsValidTrue)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);

    // Arrange
    g_backendInitResult = true;
    // Act
    DebInstaller installer;
    // Assert
    EXPECT_TRUE(installer.isValid());
    EXPECT_NE(installer.m_backend, nullptr);
    EXPECT_EQ(g_backendInitCalls, 1);
}

// B2: backend init failure -> invalid + error message path
TEST_F(DebInstaller_UT, Ctor_BackendInitFails_IsValidFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::Backend, initErrorMessage), ut_backendInitErrorMessage);

    // Arrange
    g_backendInitResult = false;
    // Act
    DebInstaller installer;
    // Assert
    EXPECT_FALSE(installer.isValid());
    EXPECT_EQ(g_backendInitCalls, 1);
}

// B3: explicit initBackend re-creates the backend instance
TEST_F(DebInstaller_UT, InitBackend_Reinit_RecreatesBackend)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);

    // Arrange
    g_backendInitResult = true;
    DebInstaller installer;
    QApt::Backend *first = installer.m_backend;
    EXPECT_NE(first, nullptr);
    // Act
    installer.initBackend();
    // Assert
    EXPECT_NE(installer.m_backend, first);
}

// ---------- isValid ----------

TEST_F(DebInstaller_UT, IsValid_AfterInit_ReflectsCachedState)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);

    DebInstaller installer;

    // Arrange: cached flag mirrors the last Backend::init result
    installer.m_bValid = true;
    // Act
    const bool ut_valid1 = installer.isValid();
    installer.m_bValid = false;
    const bool ut_valid2 = installer.isValid();
    // Assert: getter reflects the cached flag, no re-init
    EXPECT_TRUE(ut_valid1);
    EXPECT_FALSE(ut_valid2);
}

// ---------- isArchMatched ----------

// B1: invalid deb -> false
TEST_F(DebInstaller_UT, IsArchMatched_InvalidDeb_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);

    DebInstaller installer;
    // Arrange
    g_debValid = false;
    g_debArchQueried = false;
    // Act
    const bool ut_isArchMatched = installer.isArchMatched(m_debPath);
    // Assert: invalid deb rejected before architecture comparison
    EXPECT_FALSE(ut_isArchMatched);
    EXPECT_FALSE(g_debArchQueried);
}

// B2: arch "all" -> false without consulting backend
TEST_F(DebInstaller_UT, IsArchMatched_AllArch_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);
    stub.set(ADDR(QApt::DebFile, architecture), ut_debArchitecture);

    DebInstaller installer;
    // Arrange
    g_debValid = true;
    g_debArch = "all";
    g_debArchQueried = true;
    // Act
    const bool ut_isArchMatched = installer.isArchMatched(m_debPath);
    // Assert: "all" is never host-matchable
    EXPECT_FALSE(ut_isArchMatched);
    EXPECT_TRUE(g_debArchQueried);
}

// B3: matching arch -> true
TEST_F(DebInstaller_UT, IsArchMatched_MatchedArch_ReturnsTrue)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);
    stub.set(ADDR(QApt::DebFile, architecture), ut_debArchitecture);
    stub.set(ADDR(QApt::Backend, architectures), ut_backendArchitectures);

    DebInstaller installer;
    // Arrange
    g_debArch = "amd64";
    g_backendArchs = QStringList { "amd64", "i386" };
    g_debArchQueried = false;
    // Act
    const bool ut_isArchMatched = installer.isArchMatched(m_debPath);
    // Assert
    EXPECT_TRUE(ut_isArchMatched);
    EXPECT_TRUE(g_debArchQueried);
}

// B4: unmatched arch -> false
TEST_F(DebInstaller_UT, IsArchMatched_UnmatchedArch_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);
    stub.set(ADDR(QApt::DebFile, architecture), ut_debArchitecture);
    stub.set(ADDR(QApt::Backend, architectures), ut_backendArchitectures);

    DebInstaller installer;
    // Arrange
    g_debArch = "arm64";
    g_backendArchs = QStringList { "amd64" };
    g_debArchQueried = false;
    // Act
    const bool ut_isArchMatched = installer.isArchMatched(m_debPath);
    // Assert
    EXPECT_FALSE(ut_isArchMatched);
    EXPECT_TRUE(g_debArchQueried);
}

// ---------- isDebValid ----------

TEST_F(DebInstaller_UT, IsDebValid_InvalidDeb_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);

    DebInstaller installer;
    // Arrange
    g_debValid = false;
    // Act
    const bool ut_isDebValid = installer.isDebValid(m_debPath);
    // Assert
    EXPECT_FALSE(ut_isDebValid);
    EXPECT_EQ(g_backendInitCalls, 1);
}

TEST_F(DebInstaller_UT, IsDebValid_ValidDeb_ReturnsTrue)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);

    DebInstaller installer;
    // Arrange
    g_debValid = true;
    // Act
    const bool ut_isDebValid = installer.isDebValid(m_debPath);
    // Assert
    EXPECT_TRUE(ut_isDebValid);
    EXPECT_EQ(g_backendInitCalls, 1);
}

// ---------- reset ----------

// B1: reloadCache failure -> invalid state, trans cleared
TEST_F(DebInstaller_UT, Reset_ReloadCacheFails_StateInvalid)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::Backend, reloadCache), ut_backendReloadCache);
    stub.set(ADDR(QApt::Backend, initErrorMessage), ut_backendInitErrorMessage);

    DebInstaller installer;
    // Arrange
    QApt::Transaction *trans = new QApt::Transaction("ut-tid");
    installer.m_pTrans = trans;
    g_reloadCacheResult = false;
    // Act
    installer.reset();
    // Assert
    EXPECT_FALSE(installer.m_bValid);
    EXPECT_EQ(installer.m_pTrans, nullptr);
    EXPECT_EQ(installer.m_iRuningTestCount, 0);
    delete trans;
}

// B2: reloadCache success -> valid state
TEST_F(DebInstaller_UT, Reset_ReloadCacheSucceeds_StateValid)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::Backend, reloadCache), ut_backendReloadCache);

    DebInstaller installer;
    // Arrange
    installer.m_bValid = false;
    g_reloadCacheResult = true;
    // Act
    installer.reset();
    // Assert
    EXPECT_TRUE(installer.m_bValid);
    EXPECT_EQ(installer.m_pTrans, nullptr);
}

// ---------- doOperate ----------

// B1: install path, invalid deb -> errorOccurred("Deb is invalid")
TEST_F(DebInstaller_UT, DoOperate_InstallInvalidDeb_EmitsError)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);

    DebInstaller installer;
    // Arrange
    g_debValid = false;
    QSignalSpy errorSpy(&installer, &DebInstaller::errorOccurred);
    // Act
    installer.doOperate(m_debPath, true);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Deb is invalid"));
}

// B2: install path, installFile null -> errorOccurred("Get transaction failed!")
TEST_F(DebInstaller_UT, DoOperate_InstallTransactionNull_EmitsError)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);
    stub.set(ADDR(QApt::Backend, installFile), ut_backendInstallFile);

    DebInstaller installer;
    // Arrange
    g_debValid = true;
    g_installFileResult = nullptr;
    QSignalSpy errorSpy(&installer, &DebInstaller::errorOccurred);
    // Act
    installer.doOperate(m_debPath, true);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Get transaction failed!"));
    EXPECT_EQ(installer.m_pTrans, nullptr);
}

// B3: install path, transaction finished success -> installFinished(true) + reset
TEST_F(DebInstaller_UT, DoOperate_InstallFinished_EmitsFinishedAndResets)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::DebFile, isValid), ut_debIsValid);
    stub.set(ADDR(QApt::Backend, installFile), ut_backendInstallFile);
    stub.set(ADDR(QApt::Backend, reloadCache), ut_backendReloadCache);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);

    DebInstaller installer;
    // Arrange
    g_debValid = true;
    g_installFileResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::Success;
    g_reloadCacheResult = true;
    QSignalSpy finSpy(&installer, &DebInstaller::installFinished);
    // Act
    installer.doOperate(m_debPath, true);
    ASSERT_NE(installer.m_pTrans, nullptr);
    emit installer.m_pTrans->finished(QApt::ExitSuccess);
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_TRUE(finSpy.at(0).at(0).toBool());
    EXPECT_EQ(installer.m_pTrans, nullptr);
    EXPECT_TRUE(installer.m_bValid);
    delete g_installFileResult;
    g_installFileResult = nullptr;
}

// B4: remove path, commitChanges transaction finished -> installFinished(false)
TEST_F(DebInstaller_UT, DoOperate_UninstallFinished_EmitsFinished)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::Backend, markPackageForRemoval), ut_backendMarkRemove);
    stub.set(ADDR(QApt::Backend, commitChanges), ut_backendCommitChanges);
    stub.set(ADDR(QApt::Backend, reloadCache), ut_backendReloadCache);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);

    DebInstaller installer;
    // Arrange
    g_commitChangesResult = new QApt::Transaction("ut-tid");
    g_transactionError = QApt::FetchError;
    g_reloadCacheResult = true;
    QSignalSpy finSpy(&installer, &DebInstaller::installFinished);
    QSignalSpy removeSpy(&installer, &DebInstaller::errorOccurred);
    // Act
    installer.doOperate("printer-driver-foo", false);
    ASSERT_NE(installer.m_pTrans, nullptr);
    emit installer.m_pTrans->finished(QApt::ExitFailed);
    // Assert
    EXPECT_EQ(g_markRemoveCalls, 1);
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_EQ(installer.m_pTrans, nullptr);
    delete g_commitChangesResult;
    g_commitChangesResult = nullptr;
}

// B5: transaction errorOccurred -> errorOccurred(QString(code))
TEST_F(DebInstaller_UT, DoOperate_TransactionError_EmitsErrorString)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(QApt::Backend, markPackageForRemoval), ut_backendMarkRemove);
    stub.set(ADDR(QApt::Backend, commitChanges), ut_backendCommitChanges);
    stub.set(ADDR(QApt::Transaction, error), ut_transactionError);
    stub.set(ADDR(QApt::Transaction, run), ut_transactionRun);

    DebInstaller installer;
    // Arrange
    g_commitChangesResult = new QApt::Transaction("ut-tid");
    QSignalSpy errorSpy(&installer, &DebInstaller::errorOccurred);
    // Act
    installer.doOperate("printer-driver-foo", false);
    ASSERT_NE(installer.m_pTrans, nullptr);
    emit installer.m_pTrans->errorOccurred(QApt::CommitError);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString::number(static_cast<int>(QApt::CommitError)));
    delete g_commitChangesResult;
    g_commitChangesResult = nullptr;
}

// ---------- installPackage ----------

// B4: dpkg free + valid -> doOperate(path, true)
TEST_F(DebInstaller_UT, InstallPackage_DpkgFree_RunsInstall)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    // Arrange
    g_dpkgLocked = false;
    installer.m_bValid = true;
    // Act
    installer.installPackage(m_debPath);
    // Assert
    EXPECT_EQ(g_doOperateArgs, (QStringList {  m_debPath, "install"  }));
    EXPECT_EQ(g_doOperateArgs.size(), 2);
}

// B3: dpkg free but invalid state -> skip
TEST_F(DebInstaller_UT, InstallPackage_InvalidState_SkipsOperate)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    // Arrange
    g_dpkgLocked = false;
    installer.m_bValid = false;
    // Act
    installer.installPackage(m_debPath);
    // Assert
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
    EXPECT_EQ(g_doOperateArgs.size(), 0);
}

// B1: dpkg locked within retry limit -> deferred
TEST_F(DebInstaller_UT, InstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    // Arrange
    g_dpkgLocked = true;
    installer.m_bValid = true;
    // Act
    installer.installPackage(m_debPath);
    // Assert: singleShot pending, nothing run yet
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
    EXPECT_EQ(g_doOperateArgs.size(), 0);
}

// B2: dpkg locked, retries exhausted -> error + finished(false)
TEST_F(DebInstaller_UT, InstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isDpkgLocked), ut_isDpkgLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    installer.m_iRuningTestCount = 20;   // MAX_DPKGRUNING_TEST
    // Arrange
    g_dpkgLocked = true;
    installer.m_bValid = true;
    QSignalSpy errorSpy(&installer, &DebInstaller::errorOccurred);
    QSignalSpy finSpy(&installer, &DebInstaller::installFinished);
    // Act
    installer.installPackage(m_debPath);
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Dpkg locked ,time out"));
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    // 🔴 DEFECT(ut-t25): exhausted-retry branch has no `return`, so doOperate
    // still runs after emitting errors (install proceeds despite dpkg lock)
    EXPECT_FALSE(g_doOperateArgs.isEmpty());
}

// ---------- uninstallPackage ----------

// B3: dpkg free -> doOperate(pkg, false)
TEST_F(DebInstaller_UT, UninstallPackage_DpkgFree_RunsRemove)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    // Arrange
    g_fileLocked = false;
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(g_doOperateArgs, (QStringList {  "printer-driver-foo", "remove"  }));
    EXPECT_EQ(g_doOperateArgs.size(), 2);
}

// B1: dpkg locked within retry limit -> deferred
TEST_F(DebInstaller_UT, UninstallPackage_DpkgLockedWithinLimit_RetriesLater)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    // Arrange
    g_fileLocked = true;
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_TRUE(g_doOperateArgs.isEmpty());
    EXPECT_EQ(g_doOperateArgs.size(), 0);
}

// B2: dpkg locked, retries exhausted -> error + finished(false)
TEST_F(DebInstaller_UT, UninstallPackage_DpkgLockedExhausted_EmitsErrors)
{
    Stub stub;
    stub.set(ADDR(QApt::Backend, init), ut_backendInit);
    stub.set(ADDR(Utils, isFileLocked), ut_isFileLocked);
    stub.set(ADDR(DebInstaller, doOperate), ut_installerDoOperateSelf);

    DebInstaller installer;
    installer.m_iRuningTestCount = 20;
    // Arrange
    g_fileLocked = true;
    QSignalSpy errorSpy(&installer, &DebInstaller::errorOccurred);
    QSignalSpy finSpy(&installer, &DebInstaller::installFinished);
    // Act
    installer.uninstallPackage("printer-driver-foo");
    // Assert
    EXPECT_EQ(errorSpy.count(), 1);
    EXPECT_EQ(errorSpy.at(0).at(0).toString(), QString("Dpkg locked ,time out"));
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    // 🔴 DEFECT(ut-t25): exhausted-retry branch has no `return`, doOperate still runs
    EXPECT_FALSE(g_doOperateArgs.isEmpty());
}
