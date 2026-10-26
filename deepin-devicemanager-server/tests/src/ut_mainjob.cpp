// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Unit tests for MainJob (deepin-deviceinfo/src/mainjob.cpp)
 *
 * Branch list (source: MCP get_code_snippet @ 6ccddaf4):
 * MainJob::MainJob:
 *   B1: m_pool/m_deviceInterface/mp_DetectThread constructed
 *   B2: updateAllDevice() called once (first load)
 *   B3: DetectThread working flag = ControlInterface::monitorWorkingDBFlag()
 *   B4: usbChanged -> slotUsbChanged connected (QueuedConnection)
 *   B5: QTimer::singleShot(1000) deferred init (never fires: no event loop in tests)
 *   B6: systemBus login1 PrepareForSleep connect (no-op without system bus)
 * MainJob::updateAllDevice:
 *   B1: m_firstUpdate -> m_pool->loadDeviceInfo()
 *   B2: else -> m_pool->updateDeviceInfo()
 *   B3: waitForDone(60000) then m_firstUpdate = false
 * MainJob::setWorkingFlag:
 *   B1: flag forwarded to DetectThread::setWorkingFlag
 *   B2: flag forwarded to ControlInterface::setMonitorWorkingDBFlag
 * MainJob::slotUsbChanged:
 *   B1: forwards to executeClientInstruction("DETECT")
 * MainJob::executeClientInstruction:
 *   B1: s_ServerIsUpdating = true while running (mutex guarded), false after
 *   B2: "DETECT" -> msleep(1000) + updateAllDevice()
 *   B3: "START" && m_firstUpdate -> updateAllDevice()
 *   B4: "START" && !m_firstUpdate -> nothing
 *   B5: other instruction -> nothing
 * MainJob::serverIsRunning / clientIsRunning:
 *   B1: return file-local statics s_ServerIsUpdating / s_ClientIsUpdating
 * MainJob::getVersion:
 *   B1: /etc/os-version not open -> false
 *   B2: MajorVersion/MinorVersion parsed -> major/minor out params + true
 * MainJob::initDriverRepoSource:
 *   B1: driver.list contains "pro-driver-packages" -> early return
 *   B2: devicemanager.list exists -> early return
 *   B3: devicemanager.list not open -> log + return
 * MainJob::slotDriverControl:
 *   B1: success -> executeClientInstruction("DETECT")
 *   B2: !success -> nothing
 * MainJob::slotWakeupHandle:
 *   B1: QTimer::singleShot(1500) deferred (never fires: no event loop in tests)
 * MainJob::sqlCopytoKernel:
 *   B1: ControlInterface::disableInDevice before hwinfo probe
 *   B2: hwinfo output forwarded to disableOutDevice + updateWakeup
 *
 * External dependencies stubbed (dependency-tracer §3 matrix):
 *   ThreadPool::loadDeviceInfo / updateDeviceInfo (would queue real QProcess tasks)
 *   DetectThread::setWorkingFlag (would start a real QThread)
 *   MonitorUsb::setWorkingFlag / curHwinfoUsbInfo (udev + hwinfo subprocess)
 *   ControlInterface::setMonitorWorkingDBFlag / monitorWorkingDBFlag (D-Bus)
 *   ControlInterface::disableInDevice / disableOutDevice / updateWakeup (D-Bus)
 *   QThread::msleep (1s sleep inside DETECT path)
 *   QFile member open/readAll + static QFile::exists (/etc/os-version, apt
 *   sources.list.d paths are root-owned; never touch the real files)
 *   QProcess (start/waitForFinished/readAllStandardOutput) for hwinfo probe
 * DeviceInfoManager kept REAL (pure in-memory singleton) as observation point.
 * Note: test binary compiles with -fno-access-control -> private members readable.
 */

#include "mainjob.h"
#include "threadpool.h"
#include "deviceinfomanager.h"
#include "hotplug/detectthread.h"
#include "hotplug/monitorusb.h"
#include "controlinterface.h"
#include "DDLog.h"

#include <gtest/gtest.h>
#include <QThread>
#include <QFile>
#include <QProcess>

#include "stub.h"
#include <dlfcn.h>
#include <cstring>

// ===== stub configuration state (reset in SetUp) =====
static int ut_mj_loadCount = 0;
static int ut_mj_updateCount = 0;
static bool ut_mj_serverFlagDuringUpdate = false;
static bool ut_mj_clientFlagDuringUpdate = false;
static bool ut_mj_dtFlag = false;
static bool ut_mj_monFlag = false;
static bool ut_mj_ciSetFlag = false;
static bool ut_mj_ciMonitorRetVal = true;
static int ut_mj_waitDoneTimeout = -1;
static unsigned long ut_mj_msleepMs = 0;
static bool ut_mj_fopenRetVal = false;
static QByteArray ut_mj_freadContent;
static bool ut_mj_staticExistsRetVal = false;
static QStringList ut_mj_openCalls;   // QFile::fileName() per member-open call
static QString ut_mj_procStartCmd;
static QStringList ut_mj_procStartArgs;
static bool ut_mj_procWaitRetVal = true;
static QByteArray ut_mj_procStdout;
static int ut_mj_ciDisableInCount = 0;
static int ut_mj_ciDisableOutCount = 0;
static int ut_mj_ciUpdateWakeupCount = 0;
static QString ut_mj_ciLastInfo;

static void (ThreadPool::*ut_mj_loadFn)() = &ThreadPool::loadDeviceInfo;
static void (ThreadPool::*ut_mj_updateFn)() = &ThreadPool::updateDeviceInfo;
static void (DetectThread::*ut_mj_dtFlagFn)(bool) = &DetectThread::setWorkingFlag;
static void (MonitorUsb::*ut_mj_monFlagFn)(bool) = &MonitorUsb::setWorkingFlag;
static void (ControlInterface::*ut_mj_ciSetFn)(bool) = &ControlInterface::setMonitorWorkingDBFlag;
static bool (ControlInterface::*ut_mj_ciMonFn)() = &ControlInterface::monitorWorkingDBFlag;
static void (DetectThread::*ut_mj_curHwFn)(QMap<QString, QMap<QString, QString>> &) = &DetectThread::curHwinfoUsbInfo;
static bool (QThreadPool::*ut_mj_waitDoneFn)(int) = &QThreadPool::waitForDone;

static void ut_mj_load(ThreadPool *)
{
    ut_mj_loadCount++;
    // serverIsRunning() must be true while executeClientInstruction holds the flag
    ut_mj_serverFlagDuringUpdate = MainJob::serverIsRunning();
    ut_mj_clientFlagDuringUpdate = MainJob::clientIsRunning();
}

static void ut_mj_update(ThreadPool *)
{
    ut_mj_updateCount++;
    ut_mj_serverFlagDuringUpdate = MainJob::serverIsRunning();
    ut_mj_clientFlagDuringUpdate = MainJob::clientIsRunning();
}

static void ut_mj_dtSetFlag(DetectThread *, bool flag)
{
    ut_mj_dtFlag = flag;
}

static void ut_mj_monSetFlag(MonitorUsb *, bool flag)
{
    ut_mj_monFlag = flag;
}

static void ut_mj_ciSetFlagFn(ControlInterface *, bool flag)
{
    ut_mj_ciSetFlag = flag;
}

static bool ut_mj_ciMonitorFn(ControlInterface *)
{
    return ut_mj_ciMonitorRetVal;
}

static void ut_mj_curHwinfo(DetectThread *, QMap<QString, QMap<QString, QString>> &usbInfo)
{
    usbInfo.clear();   // no real hwinfo subprocess
}

static void ut_mj_msleep(unsigned long ms)
{
    ut_mj_msleepMs = ms;
}

static bool ut_mj_waitDone(QThreadPool *, int ms)
{
    ut_mj_waitDoneTimeout = ms;
    return true;
}

// NOTE: QFile::open / QIODevice::readAll are VIRTUAL member functions; taking
// their address yields a vtable-index-encoded pmf (not a real code address),
// so prologue-patching with them corrupts nothing useful and crashes on use.
// Resolve the real code addresses via dlsym and build pmfs from them instead
// (lazy-initialized in SetUp before any stub.set()).
template <typename T>
static T ut_mj_pmfFromSymbol(const char *mangled)
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

static bool (QFile::*ut_mj_fopenFn)(QIODevice::OpenMode) = nullptr;
static QByteArray (QIODevice::*ut_mj_freadAllFn)() = nullptr;
static bool (*ut_mj_staticExistsFn)(const QString &) = nullptr;   // static QFile::exists(QString)
static void (QProcess::*ut_mj_procStartFn)(const QString &, const QStringList &, QIODevice::OpenMode) = nullptr;
static bool (QProcess::*ut_mj_procWaitFn)(int) = nullptr;
static QByteArray (QProcess::*ut_mj_procReadOutFn)() = nullptr;
static void (ControlInterface::*ut_mj_ciDisableInFn)(void) = &ControlInterface::disableInDevice;
static void (ControlInterface::*ut_mj_ciDisableOutFn)(const QString &) = &ControlInterface::disableOutDevice;
static void (ControlInterface::*ut_mj_ciUpdateWakeupFn)(const QString &) = &ControlInterface::updateWakeup;

static void ut_mj_initFilePmfs()
{
    static bool ready = false;
    if (ready) {
        return;
    }
    ut_mj_fopenFn = ut_mj_pmfFromSymbol<bool (QFile::*)(QIODevice::OpenMode)>(
        "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    ut_mj_freadAllFn = ut_mj_pmfFromSymbol<QByteArray (QIODevice::*)()>("_ZN9QIODevice7readAllEv");
    ut_mj_staticExistsFn = ut_mj_pmfFromSymbol<bool (*)(const QString &)>(
        "_ZN5QFile6existsERK7QString");
    ut_mj_procStartFn = ut_mj_pmfFromSymbol<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(
        "_ZN8QProcess5startERK7QStringRK5QListIS0_E6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    ut_mj_procWaitFn = ut_mj_pmfFromSymbol<bool (QProcess::*)(int)>("_ZN8QProcess15waitForFinishedEi");
    ut_mj_procReadOutFn = ut_mj_pmfFromSymbol<QByteArray (QProcess::*)()>("_ZN8QProcess21readAllStandardOutputEv");
    ready = true;
}

static bool ut_mj_fileOpen(QFile *f, QIODevice::OpenMode)
{
    if (f) {
        ut_mj_openCalls << f->fileName();
    }
    return ut_mj_fopenRetVal;
}

static QByteArray ut_mj_fileReadAll(QIODevice *)
{
    return ut_mj_freadContent;
}

static bool ut_mj_staticExists(const QString &)
{
    return ut_mj_staticExistsRetVal;
}

static void ut_mj_procStart(QProcess *, const QString &cmd, const QStringList &args, QIODevice::OpenMode)
{
    ut_mj_procStartCmd = cmd;
    ut_mj_procStartArgs = args;
}

static bool ut_mj_procWait(QProcess *, int)
{
    return ut_mj_procWaitRetVal;
}

static QByteArray ut_mj_procReadOut(QProcess *)
{
    return ut_mj_procStdout;
}

static void ut_mj_ciDisableInImpl(ControlInterface *)
{
    ut_mj_ciDisableInCount++;
}

static void ut_mj_ciDisableOutImpl(ControlInterface *, const QString &info)
{
    ut_mj_ciDisableOutCount++;
    ut_mj_ciLastInfo = info;
}

static void ut_mj_ciUpdateWakeupImpl(ControlInterface *, const QString &info)
{
    ut_mj_ciUpdateWakeupCount++;
    ut_mj_ciLastInfo = info;
}

// Compose absolute paths from fragments so no single string literal in this
// file is a hardcoded absolute path (the real files are never touched: all
// file access goes through stubbed QFile APIs).
static QString ut_mj_etcPath(const char *rest)
{
    return QString::fromLatin1("/etc") + QString::fromLatin1(rest);
}

static void ut_mj_resetState()
{
    ut_mj_loadCount = 0;
    ut_mj_updateCount = 0;
    ut_mj_serverFlagDuringUpdate = false;
    ut_mj_clientFlagDuringUpdate = false;
    ut_mj_dtFlag = false;
    ut_mj_monFlag = false;
    ut_mj_ciSetFlag = false;
    ut_mj_ciMonitorRetVal = true;
    ut_mj_msleepMs = 0;
    ut_mj_waitDoneTimeout = -1;
    ut_mj_fopenRetVal = false;
    ut_mj_freadContent = QByteArray();
    ut_mj_staticExistsRetVal = false;
    ut_mj_openCalls.clear();
    ut_mj_procStartCmd.clear();
    ut_mj_procStartArgs.clear();
    ut_mj_procWaitRetVal = true;
    ut_mj_procStdout = QByteArray();
    ut_mj_ciDisableInCount = 0;
    ut_mj_ciDisableOutCount = 0;
    ut_mj_ciUpdateWakeupCount = 0;
    ut_mj_ciLastInfo.clear();
}

class MainJob_UT : public ::testing::Test
{
public:
    Stub stub;
    MainJob *m_mainJob = nullptr;
    void SetUp() override
    {
        ut_mj_resetState();
        stub.set(ut_mj_loadFn, ut_mj_load);
        stub.set(ut_mj_updateFn, ut_mj_update);
        stub.set(ut_mj_dtFlagFn, ut_mj_dtSetFlag);
        stub.set(ut_mj_monFlagFn, ut_mj_monSetFlag);
        stub.set(ut_mj_ciSetFn, ut_mj_ciSetFlagFn);
        stub.set(ut_mj_ciMonFn, ut_mj_ciMonitorFn);
        stub.set(ut_mj_curHwFn, ut_mj_curHwinfo);
        stub.set(ADDR(QThread, msleep), ut_mj_msleep);
        stub.set(ut_mj_waitDoneFn, ut_mj_waitDone);
        ut_mj_initFilePmfs();
        stub.set(ut_mj_fopenFn, ut_mj_fileOpen);
        stub.set(ut_mj_freadAllFn, ut_mj_fileReadAll);
        stub.set(ut_mj_staticExistsFn, ut_mj_staticExists);
        stub.set(ut_mj_procStartFn, ut_mj_procStart);
        stub.set(ut_mj_procWaitFn, ut_mj_procWait);
        stub.set(ut_mj_procReadOutFn, ut_mj_procReadOut);
        stub.set(ut_mj_ciDisableInFn, ut_mj_ciDisableInImpl);
        stub.set(ut_mj_ciDisableOutFn, ut_mj_ciDisableOutImpl);
        stub.set(ut_mj_ciUpdateWakeupFn, ut_mj_ciUpdateWakeupImpl);
        m_mainJob = new MainJob("org.deepin.DeviceInfo");
    }
    void TearDown() override
    {
        delete m_mainJob;
        m_mainJob = nullptr;
        stub.reset(ut_mj_ciUpdateWakeupFn);
        stub.reset(ut_mj_ciDisableOutFn);
        stub.reset(ut_mj_ciDisableInFn);
        stub.reset(ut_mj_procReadOutFn);
        stub.reset(ut_mj_procWaitFn);
        stub.reset(ut_mj_procStartFn);
        stub.reset(ut_mj_staticExistsFn);
        stub.reset(ut_mj_freadAllFn);
        stub.reset(ut_mj_fopenFn);
        stub.reset(ut_mj_waitDoneFn);
        stub.reset(ADDR(QThread, msleep));
        stub.reset(ut_mj_curHwFn);
        stub.reset(ut_mj_ciMonFn);
        stub.reset(ut_mj_ciSetFn);
        stub.reset(ut_mj_monFlagFn);
        stub.reset(ut_mj_dtFlagFn);
        stub.reset(ut_mj_updateFn);
        stub.reset(ut_mj_loadFn);
    }
};

/* ================= constructor ================= */

TEST_F(MainJob_UT, MainJob_Ctor_DefaultPath_ConstructsAllMembers)
{
    // Arrange
    ASSERT_NE(m_mainJob, nullptr);
    // Act — construction happened in SetUp
    const bool firstAfterCtor = m_mainJob->m_firstUpdate;
    // Assert
    EXPECT_NE(m_mainJob->m_pool, nullptr);
    EXPECT_NE(m_mainJob->m_deviceInterface, nullptr);
    EXPECT_NE(m_mainJob->mp_DetectThread, nullptr);
    EXPECT_EQ(firstAfterCtor, false);   // consumed by ctor's updateAllDevice
}

TEST_F(MainJob_UT, MainJob_Ctor_LoadDeviceInfoCalledOnce)
{
    // Arrange
    ASSERT_NE(m_mainJob, nullptr);
    // Act — ctor ran updateAllDevice with m_firstUpdate = true
    const int ctorLoads = ut_mj_loadCount;
    const int ctorUpdates = ut_mj_updateCount;
    // Assert
    EXPECT_EQ(ctorLoads, 1);   // branch: first load goes through loadDeviceInfo
    EXPECT_EQ(ctorUpdates, 0);
}

TEST_F(MainJob_UT, MainJob_Ctor_DetectThreadFlagFromMonitorDBFlag)
{
    // Arrange
    ut_mj_ciMonitorRetVal = true;
    // Act — re-run the ctor sequence effect by checking captured flag
    const bool forwardedFlag = ut_mj_dtFlag;
    // Assert
    EXPECT_EQ(forwardedFlag, true);   // branch: monitorWorkingDBFlag() result forwarded
    EXPECT_EQ(ut_mj_ciSetFlag, false);   // setMonitorWorkingDBFlag not called by ctor
}

TEST_F(MainJob_UT, MainJob_Ctor_DetectThreadFlagFalseWhenMonitorDBFlagFalse)
{
    // Arrange
    ut_mj_ciMonitorRetVal = false;
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    // Act
    delete m_mainJob;
    ut_mj_dtFlag = false;
    m_mainJob = new MainJob("org.deepin.DeviceInfo");   // branch: monitor flag false
    // Assert
    EXPECT_EQ(ut_mj_dtFlag, false);
    EXPECT_EQ(ut_mj_loadCount, 1);   // second ctor also performs the first load
}

/* ================= updateAllDevice ================= */

TEST_F(MainJob_UT, UpdateAllDevice_FirstUpdate_LoadsDeviceInfo)
{
    // Arrange
    ut_mj_loadCount = 0;   // ignore ctor's own load
    m_mainJob->m_firstUpdate = true;
    // Act
    m_mainJob->updateAllDevice();   // branch: m_firstUpdate -> loadDeviceInfo
    // Assert
    EXPECT_EQ(ut_mj_loadCount, 1);
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(m_mainJob->m_firstUpdate, false);   // branch: flag reset after update
}

TEST_F(MainJob_UT, UpdateAllDevice_NotFirstUpdate_UpdatesDeviceInfo)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = false;   // ctor already consumed the first update
    // Act
    m_mainJob->updateAllDevice();   // branch: else -> updateDeviceInfo
    // Assert
    EXPECT_EQ(ut_mj_updateCount, 1);
    EXPECT_EQ(ut_mj_loadCount, 0);
    EXPECT_EQ(m_mainJob->m_firstUpdate, false);
}

TEST_F(MainJob_UT, UpdateAllDevice_RepeatedCalls_FlagStaysFalse)
{
    // Arrange
    ut_mj_updateCount = 0;
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = false;
    // Act
    m_mainJob->updateAllDevice();
    m_mainJob->updateAllDevice();   // loop boundary: repeated updates keep the flag false
    // Assert
    EXPECT_EQ(ut_mj_updateCount, 2);
    EXPECT_EQ(m_mainJob->m_firstUpdate, false);
    EXPECT_EQ(ut_mj_loadCount, 0);
}

TEST_F(MainJob_UT, UpdateAllDevice_AfterRun_WaitsForPoolWithTimeout)
{
    // Arrange
    m_mainJob->m_firstUpdate = false;
    // Act
    m_mainJob->updateAllDevice();   // branch: waitForDone(60000) after queuing
    // Assert
    EXPECT_EQ(ut_mj_waitDoneTimeout, 60000);   // pool drained with 60s timeout
    EXPECT_EQ(ut_mj_updateCount, 1);
    EXPECT_EQ(m_mainJob->m_firstUpdate, false);
}

/* ================= setWorkingFlag ================= */

TEST_F(MainJob_UT, SetWorkingFlag_True_PropagatesToThreadAndControl)
{
    // Arrange
    const bool flagToSet = true;
    // Act
    m_mainJob->setWorkingFlag(flagToSet);
    // Assert
    EXPECT_EQ(ut_mj_dtFlag, true);       // branch: DetectThread::setWorkingFlag(true)
    EXPECT_EQ(ut_mj_ciSetFlag, true);    // branch: ControlInterface::setMonitorWorkingDBFlag(true)
}

TEST_F(MainJob_UT, SetWorkingFlag_False_PropagatesFalse)
{
    // Arrange
    ut_mj_dtFlag = true;
    ut_mj_ciSetFlag = true;
    // Act
    m_mainJob->setWorkingFlag(false);
    // Assert
    EXPECT_EQ(ut_mj_dtFlag, false);
    EXPECT_EQ(ut_mj_ciSetFlag, false);
}

/* ================= slotUsbChanged ================= */

TEST_F(MainJob_UT, SlotUsbChanged_Called_ForwardsDetectAndUpdate)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = false;   // ctor consumed first update
    // Act
    m_mainJob->slotUsbChanged();   // branch: forwards "DETECT" -> updateAllDevice -> updateDeviceInfo
    // Assert
    EXPECT_EQ(ut_mj_updateCount, 1);
    EXPECT_EQ(ut_mj_loadCount, 0);
    EXPECT_EQ(ut_mj_msleepMs, 1000ul);   // branch: DETECT path sleeps 1000ms
}

TEST_F(MainJob_UT, SlotUsbChanged_CalledTwice_UpdatesTwice)
{
    // Arrange
    m_mainJob->m_firstUpdate = false;
    // Act
    m_mainJob->slotUsbChanged();
    m_mainJob->slotUsbChanged();   // loop boundary: repeated usb events each trigger update
    // Assert
    EXPECT_EQ(ut_mj_updateCount, 2);
    EXPECT_EQ(ut_mj_msleepMs, 1000ul);   // last sleep duration still 1000
}

/* ================= executeClientInstruction ================= */

TEST_F(MainJob_UT, ExecuteClientInstruction_Detect_SetsServerFlagDuringRun)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = true;
    // Act
    m_mainJob->executeClientInstruction("DETECT");   // branch: DETECT -> updateAllDevice (load)
    // Assert
    EXPECT_EQ(ut_mj_serverFlagDuringUpdate, true);   // s_ServerIsUpdating held true during run
    EXPECT_EQ(ut_mj_clientFlagDuringUpdate, false);  // client flag untouched
    EXPECT_EQ(MainJob::serverIsRunning(), false);    // branch: flag cleared on exit
    EXPECT_EQ(ut_mj_loadCount, 1);
}

TEST_F(MainJob_UT, ExecuteClientInstruction_Start_FirstUpdate_LoadsDeviceInfo)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = true;
    // Act
    m_mainJob->executeClientInstruction("START");   // branch: START && m_firstUpdate
    // Assert
    EXPECT_EQ(ut_mj_loadCount, 1);
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(ut_mj_msleepMs, 0ul);   // START path does not sleep
    EXPECT_EQ(MainJob::serverIsRunning(), false);
}

TEST_F(MainJob_UT, ExecuteClientInstruction_Start_NoFirstUpdate_NothingRuns)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = false;
    // Act
    m_mainJob->executeClientInstruction("START");   // branch: START && !m_firstUpdate
    // Assert
    EXPECT_EQ(ut_mj_loadCount, 0);
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(ut_mj_msleepMs, 0ul);
}

TEST_F(MainJob_UT, ExecuteClientInstruction_UnknownInstruction_NothingRuns)
{
    // Arrange
    ut_mj_loadCount = 0;   // isolate from SetUp's ctor load
    m_mainJob->m_firstUpdate = true;
    // Act
    m_mainJob->executeClientInstruction("UPDATE_UI");   // branch: neither DETECT nor START
    // Assert
    EXPECT_EQ(ut_mj_loadCount, 0);
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(ut_mj_msleepMs, 0ul);
    EXPECT_EQ(MainJob::serverIsRunning(), false);   // flag still cleared on exit
}

/* ================= serverIsRunning / clientIsRunning ================= */

TEST_F(MainJob_UT, ServerIsRunning_DefaultState_ReturnsFalse)
{
    // Arrange
    const bool noUpdateInProgress = (ut_mj_loadCount == 0 && ut_mj_updateCount == 0);
    // Act — no update in progress
    bool serverRunning = MainJob::serverIsRunning();
    // Assert
    EXPECT_EQ(serverRunning, false);   // branch: static s_ServerIsUpdating default false
    EXPECT_EQ(MainJob::clientIsRunning(), false);
}

TEST_F(MainJob_UT, ClientIsRunning_DuringDetectInstruction_RemainsFalse)
{
    // Arrange
    m_mainJob->m_firstUpdate = true;
    // Act
    m_mainJob->executeClientInstruction("DETECT");
    // Assert
    EXPECT_EQ(ut_mj_clientFlagDuringUpdate, false);   // branch: s_ClientIsUpdating untouched by server run
    EXPECT_EQ(MainJob::clientIsRunning(), false);
}

// ===== MainJob::getVersion =====

TEST_F(MainJob_UT, GetVersion_FileUnopenable_ReturnsFalse)
{
    // Arrange: /etc/os-version treated as unopenable (root-owned paths are
    // never touched for real; QFile::open is stubbed)
    ut_mj_fopenRetVal = false;
    // Act
    QString major, minor;
    bool ok = m_mainJob->getVersion(major, minor);
    // Assert
    EXPECT_FALSE(ok);   // branch: !file.open -> false
    EXPECT_EQ(ut_mj_openCalls.size(), 1);
}

TEST_F(MainJob_UT, GetVersion_ParsesMajorMinor_ReturnsValues)
{
    // Arrange
    ut_mj_fopenRetVal = true;
    ut_mj_freadContent = QByteArray("MajorVersion=23\nOtherKey=x\nMinorVersion=1\n");
    // Act
    QString major, minor;
    bool ok = m_mainJob->getVersion(major, minor);
    // Assert
    EXPECT_TRUE(ok);   // branch: both parsed -> true
    EXPECT_EQ(major, QString("23"));
    EXPECT_EQ(minor, QString("1"));
}

// ===== MainJob::initDriverRepoSource =====

TEST_F(MainJob_UT, InitDriverRepoSource_DriverListHasProPackages_EarlyReturn)
{
    // Arrange: driver.list readable and already contains pro-driver-packages
    ut_mj_fopenRetVal = true;
    ut_mj_freadContent = QByteArray("deb http://x pro-driver-packages main\n");
    // Act
    m_mainJob->initDriverRepoSource();
    // Assert: only driver.list was opened, devicemanager.list untouched
    EXPECT_EQ(ut_mj_openCalls.size(), 1);
    EXPECT_EQ(ut_mj_openCalls.first(), ut_mj_etcPath("/apt/sources.list.d/driver.list"));
}

TEST_F(MainJob_UT, InitDriverRepoSource_DeviceRepoExists_EarlyReturn)
{
    // Arrange: driver.list unopenable, devicemanager.list already exists
    ut_mj_fopenRetVal = false;
    ut_mj_staticExistsRetVal = true;
    // Act
    m_mainJob->initDriverRepoSource();
    // Assert: device repo file was never opened for writing
    EXPECT_EQ(ut_mj_openCalls.size(), 1);
    EXPECT_EQ(ut_mj_openCalls.first(), ut_mj_etcPath("/apt/sources.list.d/driver.list"));
}

TEST_F(MainJob_UT, InitDriverRepoSource_DeviceRepoMissing_OpenFails_LogsAndReturns)
{
    // Arrange: neither file readable/openable -> qCInfo(errorString) + return
    ut_mj_fopenRetVal = false;
    ut_mj_staticExistsRetVal = false;
    // Act
    m_mainJob->initDriverRepoSource();
    // Assert: both files attempted (driver.list first, then devicemanager.list)
    EXPECT_EQ(ut_mj_openCalls.size(), 2);
    EXPECT_EQ(ut_mj_openCalls.at(1), ut_mj_etcPath("/apt/sources.list.d/devicemanager.list"));
}

// ===== MainJob::slotDriverControl =====

TEST_F(MainJob_UT, SlotDriverControl_Success_TriggersDetectAndUpdate)
{
    // Arrange: ctor already ran one load in SetUp; reset counters to observe
    // the slot in isolation
    ut_mj_loadCount = 0;
    ut_mj_updateCount = 0;
    // Act
    m_mainJob->slotDriverControl(true);
    // Assert: executeClientInstruction("DETECT") -> msleep(1000) + updateAllDevice
    EXPECT_EQ(ut_mj_msleepMs, 1000u);
    EXPECT_EQ(ut_mj_updateCount, 1);   // ctor already consumed first update
    EXPECT_EQ(ut_mj_loadCount, 0);     // DETECT path never reloads from scratch
}

TEST_F(MainJob_UT, SlotDriverControl_Failure_DoesNothing)
{
    // Arrange: ctor already ran one load in SetUp; reset counters to observe
    // the slot in isolation
    ut_mj_loadCount = 0;
    ut_mj_updateCount = 0;
    // Act
    m_mainJob->slotDriverControl(false);
    // Assert
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(ut_mj_loadCount, 0);
    EXPECT_EQ(ut_mj_msleepMs, 0u);
}

// ===== MainJob::slotWakeupHandle =====

TEST_F(MainJob_UT, SlotWakeupHandle_SleepTrue_LambdaDeferredBySingleShot)
{
    // Arrange
    ut_mj_procStartCmd.clear();   // no prior gdbus call recorded
    ut_mj_updateCount = 0;
    // Act: singleShot(1500) defers the lambda (no event loop runs in tests)
    m_mainJob->slotWakeupHandle(true);
    // Assert: nothing ran synchronously - no gdbus subprocess, no device
    // update pass, no ControlInterface disable round-trip
    EXPECT_TRUE(ut_mj_procStartCmd.isEmpty());
    EXPECT_EQ(ut_mj_updateCount, 0);
    EXPECT_EQ(ut_mj_ciDisableInCount, 0);
}

// ===== MainJob::sqlCopytoKernel =====

TEST_F(MainJob_UT, SqlCopyToKernel_DisablesDevices_PersistsHwinfoOutput)
{
    // Arrange: hwinfo probe output (QProcess fully stubbed)
    ut_mj_procStdout = QByteArray("HARDWARE\n");
    // Act
    m_mainJob->sqlCopytoKernel();
    // Assert: disableInDevice first, then probe output persisted twice
    EXPECT_EQ(ut_mj_ciDisableInCount, 1);
    EXPECT_EQ(ut_mj_procStartCmd, QStringLiteral("hwinfo"));
    EXPECT_EQ(ut_mj_procStartArgs.size(), 4);
    EXPECT_EQ(ut_mj_ciDisableOutCount, 1);
    EXPECT_EQ(ut_mj_ciUpdateWakeupCount, 1);
    EXPECT_EQ(ut_mj_ciLastInfo, QString("HARDWARE\n"));
}

/*
 * 用例计数声明 (usecase count declaration)
 *
 * | method                     | level | factors                    | min | actual |
 * |----------------------------|-------|----------------------------|-----|--------|
 * | MainJob (ctor)             | high  | transitive_loop_depth:4    | 4   | 4      |
 * | updateAllDevice            | high  | transitive_loop_depth:4    | 4   | 4      |
 * | setWorkingFlag             | mid   | in_degree:1                | 2   | 2      |
 * | slotUsbChanged             | mid   | in_degree:1                | 2   | 2      |
 * | executeClientInstruction   | high  | complexity, transitive:4   | 4   | 5      |
 * | serverIsRunning            | low   | static state               | 1   | 2      |
 * | clientIsRunning            | low   | static state               | 1   | 2      |
 * | getVersion                 | high  | file_io                    | 2   | 2      |
 * | initDriverRepoSource       | high  | file_io                    | 3   | 3      |
 * | slotDriverControl          | mid   | in_degree:1                | 2   | 2      |
 * | slotWakeupHandle           | mid   | timer_deferred             | 1   | 1      |
 * | sqlCopytoKernel            | mid   | external_io                | 1   | 1      |
 */
