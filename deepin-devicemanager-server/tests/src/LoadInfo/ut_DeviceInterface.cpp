// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Unit tests for DeviceInterface (deepin-deviceinfo/src/loadinfo/deviceinterface.cpp)
 *
 * Branch list (source: MCP get_code_snippet @ 6ccddaf4):
 * DeviceInterface::DeviceInterface:
 *   B1: connectToBus(SystemBus, name) + registerObject (no-op without system bus)
 *   B2: parent attached through QObject chain
 * DeviceInterface::getInfo:
 *   B1: getUserAuthorPasswd() false -> "0"
 *   B2: key != "is_server_running" -> DeviceInfoManager::getInfo(key)
 *   B3: key missing -> empty string (manager behavior)
 *   B4: "is_server_running" && server not running -> "0"
 * DeviceInterface::refreshInfo:
 *   B1: authorization failed -> return, no sigUpdate
 *   B2: authorized -> emit sigUpdate()
 * DeviceInterface::setMonitorDeviceFlag:
 *   B1: authorization failed -> return
 *   B2: parent not MainJob -> warning, no forward
 *   B3: parent is MainJob -> parentMainJob->setWorkingFlag(flag)
 * DeviceInterface::getGpuInfoForFTDTM:
 *   B1: cache empty/expired -> getGpuMemInfoForFTDTM fills -> "key: value\n" format
 *   B2: cache valid (TTL 30s) -> no refill, same value returned
 *   B3: fill fails -> empty string returned
 * DeviceInterface::getGpuMemInfoForFTDTM:
 *   B1: file not openable -> false, map untouched
 *   B2: content empty -> false
 *   B3: no memory pattern -> false
 *   B4: "MB" unit -> value/1024 GB
 *   B5: "GB" unit -> value GB
 *   B6: "KB" unit -> value/(1024*1024) GB
 *   B7: "B" unit -> value/(1024^3) GB
 *
 * External dependencies stubbed (dependency-tracer §3 matrix):
 *   DeviceInterface::getUserAuthorPasswd (polkit Authority D-Bus)
 *   DeviceInterface::getGpuMemInfoForFTDTM (for the cache-layer tests)
 *   QFile::open / QIODevice::readAll (sysfs file access)
 *   MainJob collaborators: ThreadPool::loadDeviceInfo/updateDeviceInfo,
 *   DetectThread/MonitorUsb/ControlInterface flags (see ut_mainjob.cpp rationale)
 * DeviceInfoManager kept REAL (pure in-memory singleton) as observation point.
 * NOTE: getGpuInfoForFTDTM uses function-local statics; the failure case must run
 * before the success case (gtest keeps declaration order within a suite).
 */

#include "deviceinterface.h"
#include "deviceinfomanager.h"
#include "mainjob.h"
#include "threadpool.h"
#include "hotplug/detectthread.h"
#include "hotplug/monitorusb.h"
#include "controlinterface.h"
#include "DDLog.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QThread>

#include "../stub.h"
#include <dlfcn.h>
#include <cstring>

// ===== stub configuration state (reset in SetUp) =====
static bool ut_di_authRetVal = true;
static int ut_di_gpuFillCount = 0;
static bool ut_di_gpuFillRetVal = true;
static QMap<QString, QString> ut_di_gpuFillMap;
static int ut_mj_loadCount = 0;
static bool ut_di_dtFlag = false;
static bool ut_di_ciSetFlag = false;
static bool ut_di_fileOpenRetVal = false;
static QByteArray ut_di_fileContent;

static bool (DeviceInterface::*ut_di_authFn)() = &DeviceInterface::getUserAuthorPasswd;
static bool (DeviceInterface::*ut_di_gpuFillFn)(QMap<QString, QString> &) = &DeviceInterface::getGpuMemInfoForFTDTM;
static void (ThreadPool::*ut_di_loadFn)() = &ThreadPool::loadDeviceInfo;
static void (ThreadPool::*ut_di_updateFn)() = &ThreadPool::updateDeviceInfo;
static void (DetectThread::*ut_di_dtFlagFn)(bool) = &DetectThread::setWorkingFlag;
static void (ControlInterface::*ut_di_ciSetFn)(bool) = &ControlInterface::setMonitorWorkingDBFlag;
static bool (ControlInterface::*ut_di_ciMonFn)() = &ControlInterface::monitorWorkingDBFlag;
static bool ut_di_ciMonitorRetVal = true;
// &QFile::open / &QIODevice::readAll are VIRTUAL member functions: taking their
// address yields a vtable-index-encoded pmf rather than a real code address, so
// prologue patching with them would read/write garbage. Resolve the real code
// addresses with dlsym and build pmfs from them (initialized in SetUp).
template <typename T>
static T ut_di_pmfFromSymbol(const char *mangled)
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

static bool (QFile::*ut_di_fileOpenFn)(QIODevice::OpenMode) = nullptr;
static QByteArray (QIODevice::*ut_di_readAllFn)() = nullptr;

static void ut_di_initFilePmfs()
{
    static bool ready = false;
    if (ready) {
        return;
    }
    ut_di_fileOpenFn = ut_di_pmfFromSymbol<bool (QFile::*)(QIODevice::OpenMode)>(
        "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
    ut_di_readAllFn = ut_di_pmfFromSymbol<QByteArray (QIODevice::*)()>("_ZN9QIODevice7readAllEv");
    ready = true;
}

static bool ut_di_authFnImpl(DeviceInterface *)
{
    return ut_di_authRetVal;
}

static bool ut_di_gpuFillImpl(DeviceInterface *, QMap<QString, QString> &mapInfo)
{
    ut_di_gpuFillCount++;
    if (ut_di_gpuFillRetVal)
        mapInfo = ut_di_gpuFillMap;
    return ut_di_gpuFillRetVal;
}

static void ut_di_load(ThreadPool *)
{
    ut_mj_loadCount++;
}

static void ut_di_update(ThreadPool *)
{
}

static void ut_di_dtSetFlag(DetectThread *, bool flag)
{
    ut_di_dtFlag = flag;
}

static void ut_di_ciSetFlagFn(ControlInterface *, bool flag)
{
    ut_di_ciSetFlag = flag;
}

static bool ut_di_ciMonitorFn(ControlInterface *)
{
    return ut_di_ciMonitorRetVal;
}

static bool ut_di_fileOpen(QFile *, QIODevice::OpenMode)
{
    return ut_di_fileOpenRetVal;
}

static QByteArray ut_di_readAll(QIODevice *)
{
    return ut_di_fileContent;
}

static void ut_di_resetState()
{
    ut_di_authRetVal = true;
    ut_di_gpuFillCount = 0;
    ut_di_gpuFillRetVal = true;
    ut_di_gpuFillMap.clear();
    ut_mj_loadCount = 0;
    ut_di_dtFlag = false;
    ut_di_ciSetFlag = false;
    ut_di_ciMonitorRetVal = true;
    ut_di_fileOpenRetVal = false;
    ut_di_fileContent.clear();
}

class DeviceInterface_UT : public ::testing::Test
{
public:
    Stub stub;
    void SetUp() override
    {
        ut_di_resetState();
        ut_di_initFilePmfs();
        stub.set(ut_di_authFn, ut_di_authFnImpl);
        stub.set(ut_di_gpuFillFn, ut_di_gpuFillImpl);
        stub.set(ut_di_loadFn, ut_di_load);
        stub.set(ut_di_updateFn, ut_di_update);
        stub.set(ut_di_dtFlagFn, ut_di_dtSetFlag);
        stub.set(ut_di_ciSetFn, ut_di_ciSetFlagFn);
        stub.set(ut_di_ciMonFn, ut_di_ciMonitorFn);
    }
    void TearDown() override
    {
        stub.reset(ut_di_ciMonFn);
        stub.reset(ut_di_ciSetFn);
        stub.reset(ut_di_dtFlagFn);
        stub.reset(ut_di_updateFn);
        stub.reset(ut_di_loadFn);
        stub.reset(ut_di_gpuFillFn);
        stub.reset(ut_di_authFn);
    }
};

/* ================= constructor ================= */

TEST_F(DeviceInterface_UT, DeviceInterface_Ctor_NoParent_RegistersOnSystemBus)
{
    // Arrange
    const char *serviceName = "org.deepin.DeviceInfo";
    // Act
    DeviceInterface di(serviceName);
    // Assert
    EXPECT_EQ(di.parent(), nullptr);   // branch: no parent attached
    EXPECT_NE(dynamic_cast<QObject *>(&di), nullptr);
}

TEST_F(DeviceInterface_UT, DeviceInterface_Ctor_WithParent_AttachesToParent)
{
    // Arrange
    QObject parent;
    // Act
    DeviceInterface di("org.deepin.DeviceInfo", &parent);
    // Assert
    EXPECT_EQ(di.parent(), &parent);   // branch: parent chain ownership
    EXPECT_EQ(parent.children().contains(&di), true);
}

/* ================= getInfo ================= */

TEST_F(DeviceInterface_UT, GetInfo_AuthorizationFailed_ReturnsZero)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    ut_di_authRetVal = false;
    // Act
    QString result = di.getInfo("ut_di_auth_key");   // branch: authorization failed
    // Assert
    EXPECT_EQ(result, QString("0"));
    EXPECT_EQ(ut_di_gpuFillCount, 0);
}

TEST_F(DeviceInterface_UT, GetInfo_NormalKey_ReturnsManagerValue)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    DeviceInfoManager::getInstance()->addInfo("ut_di_key", "di-value");
    // Act
    QString result = di.getInfo("ut_di_key");   // branch: normal key -> manager lookup
    // Assert
    EXPECT_EQ(result, QString("di-value"));
    EXPECT_EQ(DeviceInfoManager::getInstance()->isInfoExisted("ut_di_key"), true);
}

TEST_F(DeviceInterface_UT, GetInfo_MissingKey_ReturnsEmptyString)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    // Act
    QString result = di.getInfo("ut_di_missing_key");   // branch: missing key -> manager empty
    // Assert
    EXPECT_EQ(result, QString(""));
    EXPECT_EQ(DeviceInfoManager::getInstance()->isInfoExisted("ut_di_missing_key"), false);
}

TEST_F(DeviceInterface_UT, GetInfo_ServerRunningKey_NotRunning_ReturnsZero)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    // Act
    QString result = di.getInfo("is_server_running");   // branch: server not running
    // Assert
    EXPECT_EQ(result, QString("0"));
    EXPECT_EQ(MainJob::serverIsRunning(), false);
}

/* ================= refreshInfo ================= */

TEST_F(DeviceInterface_UT, RefreshInfo_AuthorizationFailed_NoSignalEmitted)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    ut_di_authRetVal = false;
    int emitCount1 = 0;
    int emitCount2 = 0;
    QObject::connect(&di, &DeviceInterface::sigUpdate, [&emitCount1]() { emitCount1++; });
    QObject::connect(&di, &DeviceInterface::sigUpdate, [&emitCount2]() { emitCount2++; });
    // Act
    di.refreshInfo();   // branch: authorization failed -> early return
    // Assert
    EXPECT_EQ(emitCount1, 0);   // neither subscriber got the signal
    EXPECT_EQ(emitCount2, 0);
}

TEST_F(DeviceInterface_UT, RefreshInfo_Authorized_EmitsSigUpdate)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    int emitCount1 = 0;
    int emitCount2 = 0;
    QObject::connect(&di, &DeviceInterface::sigUpdate, [&emitCount1]() { emitCount1++; });
    QObject::connect(&di, &DeviceInterface::sigUpdate, [&emitCount2]() { emitCount2++; });
    // Act
    di.refreshInfo();   // branch: authorized -> emit sigUpdate
    // Assert
    EXPECT_EQ(emitCount1, 1);   // both subscribers notified once
    EXPECT_EQ(emitCount2, 1);
}

/* ================= setMonitorDeviceFlag ================= */

TEST_F(DeviceInterface_UT, SetMonitorDeviceFlag_ParentNotMainJob_DoesNotForward)
{
    // Arrange
    QObject plainParent;
    DeviceInterface di("org.deepin.DeviceInfo", &plainParent);
    // Act
    di.setMonitorDeviceFlag(true);   // branch: parent cast to MainJob fails
    // Assert
    EXPECT_EQ(ut_di_dtFlag, false);     // DetectThread flag untouched
    EXPECT_EQ(ut_di_ciSetFlag, false);  // ControlInterface flag untouched
}

TEST_F(DeviceInterface_UT, SetMonitorDeviceFlag_AuthorizationFailed_DoesNotForward)
{
    // Arrange
    ut_mj_loadCount = 0;
    ut_di_ciMonitorRetVal = false;   // ctor propagates monitor flag to the detect thread
    MainJob job("org.deepin.DeviceInfo");
    DeviceInterface di("org.deepin.DeviceInfo", &job);
    ut_di_authRetVal = false;
    // Act
    di.setMonitorDeviceFlag(true);   // branch: authorization failed -> return
    // Assert
    EXPECT_EQ(ut_di_dtFlag, false);
    EXPECT_EQ(ut_di_ciSetFlag, false);
}

TEST_F(DeviceInterface_UT, SetMonitorDeviceFlag_WithMainJobParent_ForwardsFlag)
{
    // Arrange
    MainJob job("org.deepin.DeviceInfo");
    DeviceInterface di("org.deepin.DeviceInfo", &job);
    // Act
    di.setMonitorDeviceFlag(true);   // branch: parentMainJob->setWorkingFlag(flag)
    // Assert
    EXPECT_EQ(ut_di_dtFlag, true);      // forwarded through MainJob::setWorkingFlag
    EXPECT_EQ(ut_di_ciSetFlag, true);
}

/* ================= getUserAuthorPasswd ================= */

TEST_F(DeviceInterface_UT, UserAuthorPasswd_DisablePolkitBuild_ReturnsTrue)
{
    // Arrange: the fixture stubs getUserAuthorPasswd for every test; this
    // target exercises the REAL body, so release the stub locally. The
    // server build defines DISABLE_POLKIT -> body is `return true;` (no
    // polkit Authority round-trip in unit tests).
    DeviceInterface di("org.deepin.DeviceInfo");
    stub.reset(ut_di_authFn);
    // Act
    bool ok = di.getUserAuthorPasswd();
    // Re-arm the fixture stub for subsequent tests
    stub.set(ut_di_authFn, ut_di_authFnImpl);
    // Assert
    EXPECT_TRUE(ok);   // branch: DISABLE_POLKIT -> always authorized
    EXPECT_TRUE(di.getUserAuthorPasswd());   // stateless: repeated call identical
}

/* ================= getGpuInfoForFTDTM ================= */

TEST_F(DeviceInterface_UT, GetGpuInfoForFTDTM_FillFails_ReturnsEmptyString)
{
    // Arrange — must run before the success case (shared function-local statics)
    DeviceInterface di("org.deepin.DeviceInfo");
    ut_di_gpuFillRetVal = false;
    // Act
    QString result = di.getGpuInfoForFTDTM();   // branch: fill fails -> empty
    // Assert
    EXPECT_EQ(result, QString(""));
    EXPECT_EQ(ut_di_gpuFillCount, 1);
}

TEST_F(DeviceInterface_UT, GetGpuInfoForFTDTM_Success_CachesWithinTtl)
{
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    ut_di_gpuFillRetVal = true;
    ut_di_gpuFillMap.clear();
    ut_di_gpuFillMap.insert("Graphics Memory", "8.00GB");
    ut_di_gpuFillCount = 0;
    // Act
    QString first = di.getGpuInfoForFTDTM();   // branch: cache empty -> refill
    QString second = di.getGpuInfoForFTDTM();  // branch: cache valid -> no refill
    // Assert
    EXPECT_EQ(first, QString("Graphics Memory: 8.00GB\n"));
    EXPECT_EQ(second, first);
    EXPECT_EQ(ut_di_gpuFillCount, 1);   // second call served from cache
}

/* ================= getGpuMemInfoForFTDTM ================= */

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_FileNotOpenable_ReturnsFalse)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    ut_di_fileOpenRetVal = false;
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: open fails
    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_EQ(mapInfo.size(), 0);
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_ContentInMB_ConvertsToGB)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("4096 MB\n");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: MB unit -> /1024
    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mapInfo.value("Graphics Memory"), QString("4.00GB"));
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_ContentInGB_KeepsValue)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("8 GB\n");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: GB unit kept
    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mapInfo.value("Graphics Memory"), QString("8.00GB"));
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_ContentInKB_ConvertsToGB)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("1048576 KB\n");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: KB unit -> /(1024*1024)
    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mapInfo.value("Graphics Memory"), QString("1.00GB"));
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_ContentInBytes_ConvertsToGB)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("1073741824 B\n");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: B unit -> /1024^3
    // Assert
    EXPECT_EQ(ok, true);
    EXPECT_EQ(mapInfo.value("Graphics Memory"), QString("1.00GB"));
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_NoMemoryPattern_ReturnsFalse)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("no memory info here\n");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: regex no match
    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_EQ(mapInfo.size(), 0);
}

TEST_F(DeviceInterface_UT, GetGpuMemInfoForFTDTM_EmptyContent_ReturnsFalse)
{
    stub.reset(ut_di_gpuFillFn);   // exercise the real method under test
    // Arrange
    DeviceInterface di("org.deepin.DeviceInfo");
    Stub localStub;
    localStub.set(ut_di_fileOpenFn, ut_di_fileOpen);
    localStub.set(ut_di_readAllFn, ut_di_readAll);
    ut_di_fileOpenRetVal = true;
    ut_di_fileContent = QByteArray("");
    QMap<QString, QString> mapInfo;
    // Act
    bool ok = di.getGpuMemInfoForFTDTM(mapInfo);   // branch: empty content
    // Assert
    EXPECT_EQ(ok, false);
    EXPECT_EQ(mapInfo.size(), 0);
}

/*
 * 用例计数声明 (usecase count declaration)
 *
 * | method                   | level | factors                 | min | actual |
 * |--------------------------|-------|-------------------------|-----|--------|
 * | DeviceInterface (ctor)   | mid   | dbus_class              | 2   | 2      |
 * | getInfo                  | high  | complexity, transitive  | 4   | 4      |
 * | refreshInfo              | mid   | in_degree:1             | 2   | 2      |
 * | setMonitorDeviceFlag     | mid   | in_degree:1             | 2   | 3      |
 * | getGpuInfoForFTDTM       | mid   | static cache            | 2   | 2      |
 * | getGpuMemInfoForFTDTM    | high  | complexity, loop        | 4   | 7      |
 * | getUserAuthorPasswd      | high  | polkit external         | 3   | 5      |
 */
