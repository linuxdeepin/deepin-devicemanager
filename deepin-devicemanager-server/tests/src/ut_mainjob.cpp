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
 *
 * External dependencies stubbed (dependency-tracer §3 matrix):
 *   ThreadPool::loadDeviceInfo / updateDeviceInfo (would queue real QProcess tasks)
 *   DetectThread::setWorkingFlag (would start a real QThread)
 *   MonitorUsb::setWorkingFlag / curHwinfoUsbInfo (udev + hwinfo subprocess)
 *   ControlInterface::setMonitorWorkingDBFlag / monitorWorkingDBFlag (D-Bus)
 *   QThread::msleep (1s sleep inside DETECT path)
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

#include "stub.h"

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
        m_mainJob = new MainJob("org.deepin.DeviceInfo");
    }
    void TearDown() override
    {
        delete m_mainJob;
        m_mainJob = nullptr;
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
 */
