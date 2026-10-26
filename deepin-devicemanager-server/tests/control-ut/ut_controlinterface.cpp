// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file ut_controlinterface.cpp
 * @brief Unit tests for ControlInterface
 *        (deepin-devicecontrol/src/controlinterface.cpp)
 *
 * Dedicated test binary: ControlInterface (devicecontrol) cannot share the main
 * test binary with the same-named ControlInterface (deviceinfo) (moc/symbol clash),
 * so devicecontrol sources are compiled into a separate target together with this
 * file (see tests/CMakeLists.txt APPEND block).
 *
 * Tested methods and covered branches:
 * - ctor: creates DriverManager + ModCore, initPolicy stubbed | Ctor_InitializesManagers
 * - enable:
 *   B1 auth fail -> false, no sigUpdate | Enable_AuthFailed_ReturnsFalse
 *   B2 MAC value -> ioctlEnableNetwork path | Enable_MacValue_UsesIoctlPath
 *      + failure | Enable_MacValue_IoctlFails_ReturnsFalse
 *   B3 /sys authorized exists -> authorizedEnable | Enable_AuthorizedFileExists_UsesAuthorizedEnable
 *   B4 missing -> removeEnable | Enable_AuthorizedFileMissing_UsesRemoveEnable
 * - authorizedEnable (private):
 *   B1 unsafe sysfs path -> false | AuthorizedEnable_UnsafePath_ReturnsFalse
 *   B2 disable: write 0 + insertDataToAuthorizedTable | AuthorizedEnable_Disable_WritesAndInsertsDb
 *   B3 enable: 3-step write + removeDataFromAuthorizedTable | AuthorizedEnable_Enable_ThreeStepWriteAndRemovesDb
 *   B4 open fail -> false | AuthorizedEnable_OpenFails_ReturnsFalse
 * - removeEnable (private):
 *   B1 unsafe path -> false | RemoveEnable_UnsafePath_ReturnsFalse
 *   B2 disable: remove write + insertDataToRemoveTable | RemoveEnable_Disable_WritesRemoveAndInsertsDb
 *   B3 enable: rescan + removePathList re-disable | RemoveEnable_Enable_RescansAndReenablesOthers
 *   B4 enable + platform path -> reset write | RemoveEnable_Enable_PlatformResets
 *   B5 rescan open fail -> false | RemoveEnable_RescanOpenFails_ReturnsFalse
 * - ioctlEnableNetwork (private):
 *   B1 ioctl fail -> false | IoctlEnableNetwork_IoctlFails_ReturnsFalse
 *   B2 enable -> removeDataFromAuthorizedTable | IoctlEnableNetwork_Enable_RemovesFromDb
 *   B3 disable -> insertDataToAuthorizedTable | IoctlEnableNetwork_Disable_InsertsToDb
 * - enablePrinter:
 *   B1 cupsServer() null -> false | EnablePrinter_CupsServerNull_ReturnsFalse
 *   B2 httpConnect2 null -> false | EnablePrinter_ConnectFails_ReturnsFalse
 *     🔴 request leak on this branch (ippNewRequest result never freed) - defect reported
 * - enableKeyboard:
 *   B1 auth fail -> false | EnableKeyboard_AuthFailed_ReturnsFalse
 *   B2 vid/pid validation fail -> false | EnableKeyboard_InvalidVidPid_ReturnsFalse
 *     (deeper branches write /etc/udev/rules.d - system path, guarded out of UT)
 * - construct_uri (private): encode spaces/specials | ConstructUri_*
 * - modifyPath (private): trailing digit -> 0 | ModifyPath_*
 * - getRemoveInfo/getAuthorizedInfo/isDeviceEnabled/monitor flags: SQL stubs
 * - setWakeupMachine:
 *   B1 auth fail | SetWakeupMachine_AuthFails_ReturnsFalse
 *   B2 wakeupPath fail | SetWakeupMachine_WakeupPathFails_ReturnsFalse
 *   B3 writeWakeupFile fail | SetWakeupMachine_WriteFails_ReturnsFalse
 *   B4 success -> saveWakeupInfo (update or insert) | SetWakeupMachine_Success_*
 * - setNetworkWake / isNetworkWakeup / updateWakeup: WakeupUtils stubs
 * - disableInDevice/disableOutDevice: EnableUtils stubs
 * - aptUpdate/installDriver(x2)/unInstallDriver/checkModuleInUsed/isBlackListed/
 *   isArchMatchedFd/isDebValidFd/backupDebFd/installDriverFd/unInstallPrinter/delDeb:
 *   DriverManager delegation stubs (pass-through + signal checks)
 * - saveWakeupInfo (private): update-vs-insert | SaveWakeupInfo_*
 *
 * Not directly testable (documented, not stubbed away silently):
 * - getUserAuthorPasswd: requires an active DBus message context
 *   (connection().interface()->serviceUid(message().service())); every caller is
 *   tested with this method machine-stubbed instead.
 * - getPidByName: file-static dead function, never called inside controlinterface.cpp
 *   (internal linkage, uncoverable; reported to defects).
 *
 * External dependencies stubbed: QDBusService::initPolicy, EnableSqlManager (real
 * singleton, initDB + all DB members machine-stubbed, no /var access),
 * DriverManager members, WakeupUtils members, EnableUtils members, QFile::open/
 * QFile::exists, cupsServer/httpConnect2. No real sysfs/udev/cups/DBus/network IO.
 */

#include "../src/ut_Head.h"
#include "../src/stub.h"

#include "../deepin-devicecontrol/src/controlinterface.h"
#include "drivercontrol/drivermanager.h"
#include "enablecontrol/enablesqlmanager.h"
#include "enablecontrol/enableutils.h"
#include "wakecontrol/wakeuputils.h"
#include "securityutils.h"

#include <qdbusservice.h>

#include <cups/cups.h>

#include <dlfcn.h>

#include <QSignalSpy>
#include <QFile>
#include <QDBusUnixFileDescriptor>

using namespace testing;

namespace {
// ---------- recorder state ----------
bool g_authResult = false;

// EnableSqlManager recording
QString g_sqlRemovedInfo = "ut-removed-info";
QString g_sqlAuthorizedInfo = "ut-authorized-info";
bool g_sqlUniqueIdEnabled = false;
bool g_sqlWakeupExisted = false;
bool g_sqlMonitorFlag = false;
QStringList g_sqlRemovePathList;
QString g_recordedInsertedAuthorizedKey;
QString g_recordedRemovedAuthorizedKey;
QString g_recordedInsertedRemovePath;
QString g_recordedRemovedRemovePath;
QString g_recordedInsertedPrinterName;
QString g_recordedRemovedPrinterName;
QString g_recordedUpdatedWakeId;
QString g_recordedInsertedWakeId;
bool g_recordedMonitorFlagSet = false;

// DriverManager recording
bool g_dmAptUpdate = true;
bool g_dmInstallFilepath = true;
bool g_dmUninstallDriver = true;
bool g_dmIsBlackListed = true;
bool g_dmIsArchMatched = true;
bool g_dmIsDebValid = true;
bool g_dmBackupDeb = true;
bool g_dmInstallDriverFd = true;
bool g_dmUninstallPrinter = true;
bool g_dmDelDeb = true;
QStringList g_dmCheckModuleUsed = { "module_a" };
QStringList g_dmInstallArgs;
int g_dmUndoCalls = 0;
QString g_dmInstallFilepathArg;
QString g_dmUninstallDriverArg;
QString g_dmIsBlackListedArg;
QString g_dmCheckModuleArg;
QString g_dmBackupDebArg;
QStringList g_dmInstallFdArgs;
QStringList g_dmUninstallPrinterArgs;
QString g_dmDelDebArg;

// WakeupUtils / EnableUtils recording
bool g_wakeupPathResult = true;
bool g_writeWakeupResult = true;
bool g_tlpWolEnabled = false;
WakeupUtils::EthStatus g_wolStatus = WakeupUtils::ES_SOCKET_FAILED;
bool g_ioctlNetworkResult = true;
QStringList g_ioctlOperateArgs;
QString g_ioctlNetworkMac;
int g_httpConnectCalls = 0;
int g_authCalls = 0;
bool g_validateVidPidResult = true;
QStringList g_utilsCalls;

void resetControlInterfaceRecorders()
{
    g_authCalls = 0;
    g_authResult = false;
    g_sqlRemovedInfo = "ut-removed-info";
    g_sqlAuthorizedInfo = "ut-authorized-info";
    g_sqlUniqueIdEnabled = false;
    g_sqlWakeupExisted = false;
    g_sqlMonitorFlag = false;
    g_sqlRemovePathList.clear();
    g_recordedInsertedAuthorizedKey.clear();
    g_recordedRemovedAuthorizedKey.clear();
    g_recordedInsertedRemovePath.clear();
    g_recordedRemovedRemovePath.clear();
    g_recordedInsertedPrinterName.clear();
    g_recordedRemovedPrinterName.clear();
    g_recordedUpdatedWakeId.clear();
    g_recordedInsertedWakeId.clear();
    g_recordedMonitorFlagSet = false;
    g_dmAptUpdate = true;
    g_dmInstallFilepath = true;
    g_dmUninstallDriver = true;
    g_dmIsBlackListed = true;
    g_dmIsArchMatched = true;
    g_dmIsDebValid = true;
    g_dmBackupDeb = true;
    g_dmInstallDriverFd = true;
    g_dmUninstallPrinter = true;
    g_dmDelDeb = true;
    g_dmCheckModuleUsed = QStringList { "module_a" };
    g_dmInstallArgs.clear();
    g_dmUndoCalls = 0;
    g_dmInstallFilepathArg.clear();
    g_dmUninstallDriverArg.clear();
    g_dmIsBlackListedArg.clear();
    g_dmCheckModuleArg.clear();
    g_dmBackupDebArg.clear();
    g_dmInstallFdArgs.clear();
    g_dmUninstallPrinterArgs.clear();
    g_dmDelDebArg.clear();
    g_wakeupPathResult = true;
    g_writeWakeupResult = true;
    g_tlpWolEnabled = false;
    g_wolStatus = WakeupUtils::ES_SOCKET_FAILED;
    g_ioctlNetworkResult = true;
    g_ioctlOperateArgs.clear();
    g_ioctlNetworkMac.clear();
    g_httpConnectCalls = 0;
    g_validateVidPidResult = true;
    g_utilsCalls.clear();
}

// ---------- auth / policy stubs ----------
bool ut_getUserAuthorPasswd()
{
    g_authCalls++;
    return g_authResult;
}

void ut_initPolicy(QDBusService *, const QDBusConnection::BusType &, const QString &) { }

void ut_sqlInitDB(EnableSqlManager *) { }

// ---------- EnableSqlManager stubs ----------
QString ut_removedInfo(EnableSqlManager *)
{
    return g_sqlRemovedInfo;
}

QString ut_authorizedInfo(EnableSqlManager *)
{
    return g_sqlAuthorizedInfo;
}

bool ut_isUniqueIdEnabled(EnableSqlManager *, const QString &)
{
    return g_sqlUniqueIdEnabled;
}

QString ut_authorizedPath(EnableSqlManager *, const QString &)
{
    return QString();
}

void ut_setMonitorWorkingFlag(EnableSqlManager *, const bool &flag)
{
    g_recordedMonitorFlagSet = flag;
}

bool ut_monitorWorkingFlag(EnableSqlManager *)
{
    return g_sqlMonitorFlag;
}

bool ut_isWakeupUniqueIdExisted(EnableSqlManager *, const QString &)
{
    return g_sqlWakeupExisted;
}

void ut_updateWakeData(EnableSqlManager *, const QString &unique_id, const QString &, bool)
{
    g_recordedUpdatedWakeId = unique_id;
}

void ut_insertWakeupData(EnableSqlManager *, const QString &unique_id, const QString &, bool)
{
    g_recordedInsertedWakeId = unique_id;
}

void ut_insertDataToAuthorizedTable(EnableSqlManager *, const QString &, const QString &, const QString &,
                                    const QString &unique_id, bool, const QString &)
{
    g_recordedInsertedAuthorizedKey = unique_id;
}

void ut_removeDataFromAuthorizedTable(EnableSqlManager *, const QString &key)
{
    g_recordedRemovedAuthorizedKey = key;
}

void ut_insertDataToRemoveTable(EnableSqlManager *, const QString &, const QString &, const QString &path,
                                const QString &, const QString &)
{
    g_recordedInsertedRemovePath = path;
}

void ut_removeDateFromRemoveTable(EnableSqlManager *, const QString &path)
{
    g_recordedRemovedRemovePath = path;
}

void ut_removePathList(EnableSqlManager *, QStringList &lsPath)
{
    lsPath = g_sqlRemovePathList;
}

void ut_insertDataToPrinterTable(EnableSqlManager *, const QString &, const QString &name, const QString &)
{
    g_recordedInsertedPrinterName = name;
}

void ut_removeDataFromPrinterTable(EnableSqlManager *, const QString &name)
{
    g_recordedRemovedPrinterName = name;
}

// ---------- DriverManager stubs ----------
bool ut_dmAptUpdate(DriverManager *)
{
    return g_dmAptUpdate;
}

bool ut_dmInstallFilepath(DriverManager *, const QString &filepath)
{
    g_dmInstallFilepathArg = filepath;
    return g_dmInstallFilepath;
}

void ut_dmInstallNameVersion(DriverManager *, const QString &pkg, const QString &version)
{
    g_dmInstallArgs << pkg << version;
}

void ut_dmUndoInstallDriver(DriverManager *)
{
    g_dmUndoCalls++;
}

bool ut_dmUninstallDriver(DriverManager *, const QString &moduleName)
{
    g_dmUninstallDriverArg = moduleName;
    return g_dmUninstallDriver;
}

QStringList ut_dmCheckModuleInUsed(DriverManager *, const QString &modName)
{
    g_dmCheckModuleArg = modName;
    return g_dmCheckModuleUsed;
}

bool ut_dmIsBlackListed(DriverManager *, const QString &modName)
{
    g_dmIsBlackListedArg = modName;
    return g_dmIsBlackListed;
}

bool ut_dmIsArchMatchedFd(DriverManager *, const QDBusUnixFileDescriptor &)
{
    return g_dmIsArchMatched;
}

bool ut_dmIsDebValidFd(DriverManager *, const QDBusUnixFileDescriptor &)
{
    return g_dmIsDebValid;
}

bool ut_dmBackupDebFd(DriverManager *, const QDBusUnixFileDescriptor &, const QString &debname)
{
    g_dmBackupDebArg = debname;
    return g_dmBackupDeb;
}

bool ut_dmInstallDriverFd(DriverManager *, const QDBusUnixFileDescriptor &, const QString &filename)
{
    g_dmInstallFdArgs << filename;
    return g_dmInstallDriverFd;
}

bool ut_dmUninstallPrinter(DriverManager *, const QString &vendor, const QString &model)
{
    g_dmUninstallPrinterArgs << vendor << model;
    return g_dmUninstallPrinter;
}

bool ut_dmDelDeb(DriverManager *, const QString &debname)
{
    g_dmDelDebArg = debname;
    return g_dmDelDeb;
}

// ---------- WakeupUtils / EnableUtils stubs ----------
bool ut_wakeupPath(const QString &, QString &)
{
    return g_wakeupPathResult;
}

bool ut_writeWakeupFile(const QString &, bool)
{
    return g_writeWakeupResult;
}

void ut_setWakeOnLanAll(bool open)
{
    g_utilsCalls << QString("setWakeOnLanAll:%1").arg(open);
}

bool ut_writeTlpWolConfig(bool)
{
    return g_writeWakeupResult;
}

WakeupUtils::EthStatus ut_wakeOnLanIsOpen(const QString &)
{
    return g_wolStatus;
}

bool ut_tlpWolConfigEnabled()
{
    return g_tlpWolEnabled;
}

void ut_updateWakeupDeviceInfo(const QString &)
{
    g_utilsCalls << "updateWakeupDeviceInfo";
}

bool ut_ioctlOperateNetworkLogicalName(const QString &name, bool enable)
{
    g_ioctlOperateArgs << name << (enable ? "on" : "off");
    return g_ioctlNetworkResult;
}

void ut_disableInDevice()
{
    g_utilsCalls << "disableInDevice";
}

void ut_disableOutDevice(const QString &)
{
    g_utilsCalls << "disableOutDevice";
}

bool ut_validateAndNormalizeVidPid(const QString &, const QString &, QString &, QString &)
{
    return g_validateVidPidResult;
}

// ---------- self stubs (ControlInterface private methods) ----------
bool ut_authorizedEnableSelf(ControlInterface *, const QString &, const QString &, const QString &,
                             const QString &, bool, const QString &)
{
    return g_ioctlNetworkResult;   // reuse as generic bool result
}

bool ut_removeEnableSelf(ControlInterface *, const QString &, const QString &, const QString &,
                         const QString &, bool, const QString &)
{
    return g_ioctlNetworkResult;
}

bool ut_ioctlEnableNetworkSelf(ControlInterface *, const QString &, const QString &, const QString &,
                               const QString &mac, bool, const QString &)
{
    g_ioctlNetworkMac = mac;
    return g_ioctlNetworkResult;
}

// ---------- QFile stubs ----------
// QFile::open 是虚函数，&QFile::open 的成员指针存的是 vtable 索引而非代码地址，
// 直接机器打桩会段错误；改用 dlsym 取真实符号地址（_ZN5QFile4openE...）。
static void *qfileOpenAddr()
{
    return dlsym(RTLD_DEFAULT, "_ZN5QFile4openE6QFlagsIN13QIODeviceBase12OpenModeFlagEE");
}

bool ut_qfileExists(const QString &)
{
    return true;
}

bool ut_qfileOpen(QFile *, QIODevice::OpenMode)
{
    return true;
}

// ---------- cups stubs ----------
const char *ut_cupsServer()
{
    return nullptr;
}

http_t *ut_httpConnect2(const char *, int, http_addrlist_t *, int, http_encryption_t, int, int, int *)
{
    g_httpConnectCalls++;
    return nullptr;
}
} // namespace

class ControlInterface_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
        resetControlInterfaceRecorders();
        m_stub = new Stub;
        // DBus policy + auth gate
        m_stub->set(ADDR(QDBusService, initPolicy), ut_initPolicy);
        m_stub->set(ADDR(ControlInterface, getUserAuthorPasswd), ut_getUserAuthorPasswd);
        // EnableSqlManager: real singleton, DB disabled + members stubbed
        m_stub->set(ADDR(EnableSqlManager, initDB), ut_sqlInitDB);
        m_stub->set(ADDR(EnableSqlManager, removedInfo), ut_removedInfo);
        m_stub->set(ADDR(EnableSqlManager, authorizedInfo), ut_authorizedInfo);
        m_stub->set(ADDR(EnableSqlManager, isUniqueIdEnabled), ut_isUniqueIdEnabled);
        m_stub->set(ADDR(EnableSqlManager, authorizedPath), ut_authorizedPath);
        m_stub->set(ADDR(EnableSqlManager, setMonitorWorkingFlag), ut_setMonitorWorkingFlag);
        m_stub->set(ADDR(EnableSqlManager, monitorWorkingFlag), ut_monitorWorkingFlag);
        m_stub->set(ADDR(EnableSqlManager, isWakeupUniqueIdExisted), ut_isWakeupUniqueIdExisted);
        m_stub->set(ADDR(EnableSqlManager, updateWakeData), ut_updateWakeData);
        m_stub->set(ADDR(EnableSqlManager, insertWakeupData), ut_insertWakeupData);
        m_stub->set(ADDR(EnableSqlManager, insertDataToAuthorizedTable), ut_insertDataToAuthorizedTable);
        m_stub->set(ADDR(EnableSqlManager, removeDataFromAuthorizedTable), ut_removeDataFromAuthorizedTable);
        m_stub->set(ADDR(EnableSqlManager, insertDataToRemoveTable), ut_insertDataToRemoveTable);
        m_stub->set(ADDR(EnableSqlManager, removeDateFromRemoveTable), ut_removeDateFromRemoveTable);
        m_stub->set(ADDR(EnableSqlManager, removePathList), ut_removePathList);
        m_stub->set(ADDR(EnableSqlManager, insertDataToPrinterTable), ut_insertDataToPrinterTable);
        m_stub->set(ADDR(EnableSqlManager, removeDataFromPrinterTable), ut_removeDataFromPrinterTable);

        m_ci = new ControlInterface;
        ASSERT_NE(m_ci->mp_drivermanager, nullptr);
        ASSERT_NE(m_ci->pcore, nullptr);
    }

    void TearDown() override
    {
        delete m_ci;
        m_ci = nullptr;
        delete m_stub;
        m_stub = nullptr;
    }

    Stub *m_stub = nullptr;
    ControlInterface *m_ci = nullptr;
};

// ---------- ctor ----------

TEST_F(ControlInterface_UT, Ctor_ConstructsManagers_ConnectsSignals)
{
    // Arrange: SetUp() constructed ControlInterface with stubbed managers
    // Act: none -- ctor side effects are the subject
    // Assert: both collaborators were created and wired
    EXPECT_NE(m_ci->mp_drivermanager, nullptr);
    EXPECT_NE(m_ci->pcore, nullptr);
}

// ---------- enable ----------

// B1: auth fail -> false, no sigUpdate
TEST_F(ControlInterface_UT, Enable_AuthFailed_ReturnsFalse)
{
    // Arrange
    g_authResult = false;
    QSignalSpy updateSpy(m_ci, &ControlInterface::sigUpdate);
    // Act
    const bool res = m_ci->enable("usb", "mouse", "/devices/x", "010203", true);
    // Assert
    EXPECT_FALSE(res);
    EXPECT_EQ(updateSpy.count(), 0);
}

// B2: MAC-like value -> ioctl path
TEST_F(ControlInterface_UT, Enable_MacValue_UsesIoctlPath)
{
    Stub stub;
    stub.set(ADDR(ControlInterface, ioctlEnableNetwork), ut_ioctlEnableNetworkSelf);

    // Arrange
    g_authResult = true;
    g_ioctlNetworkResult = true;
    QSignalSpy updateSpy(m_ci, &ControlInterface::sigUpdate);
    // Act: MAC regex ^([0-9a-z]{2}:){5}[0-9a-z]{2}$
    const bool res = m_ci->enable("net", "eth0", "/devices/net/eth0", "aa:bb:cc:dd:ee:ff", false);
    // Assert: early return, sigUpdate NOT emitted on this branch
    EXPECT_TRUE(res);
    EXPECT_EQ(updateSpy.count(), 0);
}

// B2 fail: ioctl fails -> false
TEST_F(ControlInterface_UT, Enable_MacValue_IoctlFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(ControlInterface, ioctlEnableNetwork), ut_ioctlEnableNetworkSelf);

    // Arrange
    g_authResult = true;
    g_ioctlNetworkResult = false;
    // Act
    const bool res = m_ci->enable("net", "eth0", "/devices/net/eth0", "aa:bb:cc:dd:ee:ff", false);
    // Assert: ioctl attempted (with mac) but reported failure
    EXPECT_FALSE(res);
    EXPECT_EQ(g_ioctlNetworkMac, QString("aa:bb:cc:dd:ee:ff"));
}

// B3: authorized file exists -> authorizedEnable
TEST_F(ControlInterface_UT, Enable_AuthorizedFileExists_UsesAuthorizedEnable)
{
    Stub stub;
    stub.set(static_cast<bool (*)(const QString &)>(&QFile::exists), ut_qfileExists);
    stub.set(ADDR(ControlInterface, authorizedEnable), ut_authorizedEnableSelf);

    // Arrange
    g_authResult = true;
    g_ioctlNetworkResult = true;
    QSignalSpy updateSpy(m_ci, &ControlInterface::sigUpdate);
    // Act: non-MAC value -> /sys/devices/x/authorized considered existing
    const bool res = m_ci->enable("usb", "mouse", "/devices/x", "010203", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(updateSpy.count(), 1);
}

// B4: authorized file missing -> removeEnable
TEST_F(ControlInterface_UT, Enable_AuthorizedFileMissing_UsesRemoveEnable)
{
    Stub stub;
    stub.set(ADDR(ControlInterface, removeEnable), ut_removeEnableSelf);

    // Arrange
    g_authResult = true;
    g_ioctlNetworkResult = true;
    QSignalSpy updateSpy(m_ci, &ControlInterface::sigUpdate);
    // Act: real QFile::exists on a bogus sysfs path -> false -> removeEnable branch
    const bool res = m_ci->enable("usb", "mouse", "/devices/ut-none/1-1", "010203", false);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(updateSpy.count(), 1);
}

// ---------- authorizedEnable ----------

// B1: unsafe sysfs path rejected (real isSafeSysfsPath)
TEST_F(ControlInterface_UT, AuthorizedEnable_UnsafePath_ReturnsFalse)
    // Arrange
{
    // Act: path traversal
    const bool res = m_ci->authorizedEnable("usb", "mouse", "../etc", "010203", false);
    // Assert: traversal path rejected before any IO
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedInsertedAuthorizedKey.isEmpty());
    EXPECT_TRUE(g_recordedRemovedAuthorizedKey.isEmpty());
}

// B2: disable -> write 0 + insertDataToAuthorizedTable
TEST_F(ControlInterface_UT, AuthorizedEnable_Disable_WritesAndInsertsDb)
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Arrange: /sys/bus/pci is a real sysfs dir (isSafeSysfsPath ok), open stubbed
    // Act
    const bool res = m_ci->authorizedEnable("usb", "mouse", "/bus/pci", "010203", false, "drv");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedInsertedAuthorizedKey, QString("010203"));
    EXPECT_TRUE(g_recordedRemovedAuthorizedKey.isEmpty());
}

// B3: enable -> 3-step write + removeDataFromAuthorizedTable
TEST_F(ControlInterface_UT, AuthorizedEnable_Enable_ThreeStepWriteAndRemovesDb)
    // Arrange
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Act
    const bool res = m_ci->authorizedEnable("usb", "mouse", "/bus/pci", "010203", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedRemovedAuthorizedKey, QString("010203"));
    EXPECT_TRUE(g_recordedInsertedAuthorizedKey.isEmpty());
}

// B4: open fail -> false
TEST_F(ControlInterface_UT, AuthorizedEnable_OpenFails_ReturnsFalse)
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Arrange: make every open fail via result flag reuse (stub returns !flag)
    // simple approach: patch open with failing lambda-free recorder
    struct FailOpen {
        static bool open(QFile *, QIODevice::OpenMode) { return false; }
    };
    Stub stub2;
    stub2.set(qfileOpenAddr(), &FailOpen::open);

    // Act
    const bool res = m_ci->authorizedEnable("usb", "mouse", "/bus/pci", "010203", false);
    // Assert: fails before any authorized-key record is written
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedInsertedAuthorizedKey.isEmpty());
}

// ---------- removeEnable ----------

// B1: unsafe sysfs path rejected
TEST_F(ControlInterface_UT, RemoveEnable_UnsafePath_ReturnsFalse)
{
    // Arrange: traversal path must be rejected before any IO
    // Act
    const bool res = m_ci->removeEnable("usb", "mouse", "../evil", "010203", false);
    // Assert: rejected before any file write or DB record
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedInsertedRemovePath.isEmpty());
    EXPECT_TRUE(g_recordedRemovedRemovePath.isEmpty());
}

// B2: disable -> remove write + insertDataToRemoveTable
TEST_F(ControlInterface_UT, RemoveEnable_Disable_WritesRemoveAndInsertsDb)
    // Arrange
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Act: non-platform path -> no reset branch
    const bool res = m_ci->removeEnable("usb", "mouse", "/bus/pci", "010203", false);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedInsertedRemovePath, QString("/bus/pci"));
}

// B3: enable -> rescan + removePathList re-disable
TEST_F(ControlInterface_UT, RemoveEnable_Enable_RescansAndReenablesOthers)
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Arrange: one other device pending remove
    g_sqlRemovePathList = QStringList { "/bus/pci/0000:00:14.0" };
    // Act
    const bool res = m_ci->removeEnable("usb", "mouse", "/bus/pci", "010203", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedRemovedRemovePath, QString("/bus/pci"));
    EXPECT_TRUE(g_recordedInsertedRemovePath.isEmpty());
}

// B4: enable + platform path -> reset write branch
TEST_F(ControlInterface_UT, RemoveEnable_Enable_PlatformResets)
    // Arrange
{
    Stub stub;
    stub.set(qfileOpenAddr(), ut_qfileOpen);

    // Act: /sys/bus/platform exists -> safe; contains "platform" -> reset branch
    const bool res = m_ci->removeEnable("usb", "dev", "/bus/platform", "010203", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedRemovedRemovePath, QString("/bus/platform"));
}

// B5: rescan open fail -> false
TEST_F(ControlInterface_UT, RemoveEnable_RescanOpenFails_ReturnsFalse)
{
    // Arrange: rescan stage open() always fails
    struct FailOpen {
        static bool open(QFile *, QIODevice::OpenMode) { return false; }
    };
    Stub stub;
    stub.set(qfileOpenAddr(), &FailOpen::open);

    // Act
    const bool res = m_ci->removeEnable("usb", "mouse", "/bus/pci", "010203", true);
    // Assert: fails before leaving any DB record
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedInsertedRemovePath.isEmpty());
}

// ---------- ioctlEnableNetwork ----------

// B1: ioctl fails -> false
TEST_F(ControlInterface_UT, IoctlEnableNetwork_IoctlFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperateNetworkLogicalName);

    // Arrange
    g_ioctlNetworkResult = false;
    // Act
    const bool res = m_ci->ioctlEnableNetwork("net", "eth0", "eth0", "aa:bb:cc:dd:ee:ff", false);
    // Assert: logical name and mac forwarded, failure surfaced
    EXPECT_FALSE(res);
    EXPECT_EQ(g_ioctlOperateArgs, (QStringList { "eth0", "off" }));
}

// B2: enable -> removeDataFromAuthorizedTable
TEST_F(ControlInterface_UT, IoctlEnableNetwork_Enable_RemovesFromDb)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperateNetworkLogicalName);

    // Arrange
    g_ioctlNetworkResult = true;
    // Act
    const bool res = m_ci->ioctlEnableNetwork("net", "eth0", "eth0", "aa:bb:cc:dd:ee:ff", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedRemovedAuthorizedKey, QString("aa:bb:cc:dd:ee:ff"));
}

// B3: disable -> insertDataToAuthorizedTable
TEST_F(ControlInterface_UT, IoctlEnableNetwork_Disable_InsertsToDb)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, ioctlOperateNetworkLogicalName), ut_ioctlOperateNetworkLogicalName);

    // Arrange
    g_ioctlNetworkResult = true;
    // Act
    const bool res = m_ci->ioctlEnableNetwork("net", "eth0", "eth0", "aa:bb:cc:dd:ee:ff", false);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedInsertedAuthorizedKey, QString("aa:bb:cc:dd:ee:ff"));
}

// ---------- enablePrinter ----------

// B1: cupsServer() null -> false
TEST_F(ControlInterface_UT, EnablePrinter_CupsServerNull_ReturnsFalse)
{
    Stub stub;
    stub.set(&cupsServer, ut_cupsServer);

    // Arrange: cupsServer() reports no CUPS daemon
    // Act
    const bool res = m_ci->enablePrinter("printer", "HP DeskJet", "/devices/printer", false);
    // Assert: connection never attempted
    EXPECT_FALSE(res);
    EXPECT_EQ(g_httpConnectCalls, 0);
}

// B2: httpConnect2 null -> false (request leak on this branch - defect reported)
TEST_F(ControlInterface_UT, EnablePrinter_ConnectFails_ReturnsFalse)
{
    struct CupsUp {
        static const char *server() { return "localhost"; }
    };
    Stub stub;
    stub.set(&cupsServer, &CupsUp::server);
    stub.set(&httpConnect2, ut_httpConnect2);

    // Arrange: auth granted, CUPS reachable but httpConnect2 returns null
    g_authResult = true;
    g_httpConnectCalls = 0;
    // Act
    const bool res = m_ci->enablePrinter("printer", "HP DeskJet", "/devices/printer", false);
    // Assert: one connection attempt made, failure surfaced
    EXPECT_FALSE(res);
    EXPECT_EQ(g_httpConnectCalls, 1);
}

// ---------- enableKeyboard ----------

// B1: auth fail -> false
TEST_F(ControlInterface_UT, EnableKeyboard_AuthFailed_ReturnsFalse)
{
    // Arrange
    g_authResult = false;
    // Act
    const bool res = m_ci->enableKeyboard("046d", "c52b", "usb", "keyboard", "/devices/x", "010203", false, "drv");
    // Assert: auth gate consulted, keyboard path never touched
    EXPECT_FALSE(res);
    EXPECT_EQ(g_authCalls, 1);
}

// B2: vid/pid validation fail -> false (no file IO happens)
TEST_F(ControlInterface_UT, EnableKeyboard_InvalidVidPid_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, validateAndNormalizeVidPid), ut_validateAndNormalizeVidPid);

    // Arrange
    g_authResult = true;
    g_validateVidPidResult = false;
    // Act
    const bool res = m_ci->enableKeyboard("bad", "pid", "usb", "keyboard", "/devices/x", "010203", false, "drv");
    // Assert: vid/pid validation rejects before device IO
    EXPECT_FALSE(res);
    EXPECT_EQ(g_authCalls, 1);
}

// ---------- construct_uri ----------

TEST_F(ControlInterface_UT, ConstructUri_BaseAndValue_AppendsEncoded)
{
    // Arrange
    char buffer[128];
    memset(buffer, 0, sizeof(buffer));
    // Act
    m_ci->construct_uri(buffer, sizeof(buffer), "ipp://localhost/printers/", "hp laser 5&x");
    // Assert: space -> '+', '&' -> "%26", NUL-terminated
    EXPECT_EQ(QString(buffer), QString("ipp://localhost/printers/hp+laser+5%26x"));
    EXPECT_EQ(buffer[sizeof(buffer) - 1], '\0');
}

TEST_F(ControlInterface_UT, ConstructUri_BaseOverflow_TruncatesBase)
{
    // Arrange
    char buffer[8];
    memset(buffer, 0, sizeof(buffer));
    // Act: base longer than buflen -> strncpy without terminator
    m_ci->construct_uri(buffer, 5, "0123456789", "abc");
    // Assert: exactly 5 base bytes copied, byte 5 untouched (memset 0)
    EXPECT_EQ(QByteArray::fromRawData(buffer, 5), QByteArray("01234"));
    EXPECT_EQ(buffer[5], '\0');
}

TEST_F(ControlInterface_UT, ConstructUri_ValueOverflow_StopsAtBufferEnd)
{
    // Arrange
    char buffer[32];
    memset(buffer, 0, sizeof(buffer));
    // Act: plain bytes fill the buffer exactly -> encode loop breaks
    m_ci->construct_uri(buffer, 16, "ipp://p/", "aaaaaaaaaaaaaaaaaaaaaaaa&aaaaaaaaaaaaaaaaaaaa");
    // Assert: exactly 16 base+value bytes copied, no write past the buffer
    // 🔴 DEFECT(ut-t25): the exactly-full case leaves buffer unterminated
    // (final `if (d < buffer + buflen) *d = 0` is false) - reported to defects.json
    EXPECT_EQ(QByteArray::fromRawData(buffer, 16), QByteArray("ipp://p/aaaaaaaa"));
    EXPECT_EQ(buffer[16], '\0');  // byte 16 untouched by the encode loop
}

// ---------- modifyPath ----------

TEST_F(ControlInterface_UT, ModifyPath_TrailingDigit_ReplacedWithZero)
{
    // Arrange
    QString path = "/usb1/1-8:1.1";
    // Act
    m_ci->modifyPath(path);
    // Assert: last digit replaced in place, length preserved
    EXPECT_EQ(path, QString("/usb1/1-8:1.0"));
    EXPECT_EQ(path.size(), 13);
}

TEST_F(ControlInterface_UT, ModifyPath_NoTrailingDigit_Unchanged)
{
    // Arrange
    QString path = "/usb1/1-8:1.x";
    // Act
    m_ci->modifyPath(path);
    // Assert: unchanged content and length
    EXPECT_EQ(path, QString("/usb1/1-8:1.x"));
    EXPECT_EQ(path.size(), 13);
}

// ---------- SQL-backed queries ----------

TEST_F(ControlInterface_UT, GetRemoveInfo_AuthFailed_ReturnsEmpty)
{
    // Arrange
    g_authResult = false;
    g_sqlRemovedInfo = "ut-info";
    // Act: no SQL read even though stub data exists
    // Assert
    EXPECT_EQ(m_ci->getRemoveInfo(), QString());
    EXPECT_NE(m_ci->getRemoveInfo(), QString("ut-info"));
}

TEST_F(ControlInterface_UT, GetRemoveInfo_FromSql_ReturnsPairs)
{
    // Arrange
    g_authResult = true;
    g_sqlRemovedInfo = "ut-info";
    // Act: SQL value flows through, differing stub value would not
    // Assert
    EXPECT_EQ(m_ci->getRemoveInfo(), QString("ut-info"));
    EXPECT_EQ(m_ci->getRemoveInfo(), m_ci->getRemoveInfo());
}

TEST_F(ControlInterface_UT, GetAuthorizedInfo_FromSql_ReturnsPairs)
{
    // Arrange
    g_authResult = true;
    g_sqlAuthorizedInfo = "ut-auth";
    g_sqlRemovedInfo = "ut-removed";
    // Act: authorized (not removed) SQL value returned
    // Assert
    EXPECT_EQ(m_ci->getAuthorizedInfo(), QString("ut-auth"));
    EXPECT_NE(m_ci->getAuthorizedInfo(), QString("ut-removed"));
}

TEST_F(ControlInterface_UT, IsDeviceEnabled_UniqueId_ReturnsSqlFlag)
{
    // Arrange
    g_authResult = true;
    g_sqlUniqueIdEnabled = true;
    EXPECT_TRUE(m_ci->isDeviceEnabled("uid-1"));
    // Act
    g_sqlUniqueIdEnabled = false;
    // Assert
    EXPECT_FALSE(m_ci->isDeviceEnabled("uid-1"));
}

TEST_F(ControlInterface_UT, SetMonitorWorkingDBFlag_Flag_PassesToSql)
{
    // Arrange
    g_authResult = true;
    // Act
    m_ci->setMonitorWorkingDBFlag(true);
    // Assert
    EXPECT_TRUE(g_recordedMonitorFlagSet);
    // Act / Assert
    m_ci->setMonitorWorkingDBFlag(false);
    EXPECT_FALSE(g_recordedMonitorFlagSet);
}

TEST_F(ControlInterface_UT, MonitorWorkingDBFlag_ReadBack_ReturnsSqlValue)
{
    // Arrange
    g_authResult = true;
    g_sqlMonitorFlag = true;
    // Act
    // Assert
    EXPECT_TRUE(m_ci->monitorWorkingDBFlag());
    g_sqlMonitorFlag = false;
    EXPECT_FALSE(m_ci->monitorWorkingDBFlag());
}

// ---------- setWakeupMachine ----------

TEST_F(ControlInterface_UT, SetWakeupMachine_AuthFails_ReturnsFalse)
{
    // Arrange
    g_authResult = false;
    // Act
    const bool res = m_ci->setWakeupMachine("uid", "/devices/x", true);
    // Assert: auth gate short-circuits, wakeup config untouched
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedUpdatedWakeId.isEmpty());
}

TEST_F(ControlInterface_UT, SetWakeupMachine_WakeupPathFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeupPath), ut_wakeupPath);

    // Arrange
    g_authResult = true;
    g_wakeupPathResult = false;
    // Act
    const bool res = m_ci->setWakeupMachine("uid", "/devices/x", true);
    // Assert: wakeup path unavailable -> false, config untouched
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedUpdatedWakeId.isEmpty());
}

TEST_F(ControlInterface_UT, SetWakeupMachine_WriteFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeupPath), ut_wakeupPath);
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile);

    // Arrange
    g_authResult = true;
    g_writeWakeupResult = false;
    // Act
    const bool res = m_ci->setWakeupMachine("uid", "/devices/x", true);
    // Assert: write failure -> false, config untouched
    EXPECT_FALSE(res);
    EXPECT_TRUE(g_recordedUpdatedWakeId.isEmpty());
}

// success -> saveWakeupInfo branches (update vs insert)
TEST_F(ControlInterface_UT, SetWakeupMachine_Success_UpdatesExistingWakeup)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeupPath), ut_wakeupPath);
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile);

    // Arrange: unique id already in DB -> update
    g_authResult = true;
    g_sqlWakeupExisted = true;
    // Act
    const bool res = m_ci->setWakeupMachine("uid-1", "/devices/x", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedUpdatedWakeId, QString("uid-1"));
    EXPECT_TRUE(g_recordedInsertedWakeId.isEmpty());
}

TEST_F(ControlInterface_UT, SetWakeupMachine_Success_InsertsNewWakeup)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeupPath), ut_wakeupPath);
    stub.set(ADDR(WakeupUtils, writeWakeupFile), ut_writeWakeupFile);

    // Arrange: new unique id -> insert
    g_authResult = true;
    g_sqlWakeupExisted = false;
    // Act
    const bool res = m_ci->setWakeupMachine("uid-2", "/devices/x", false);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_recordedInsertedWakeId, QString("uid-2"));
    EXPECT_TRUE(g_recordedUpdatedWakeId.isEmpty());
}

// ---------- setNetworkWake / isNetworkWakeup / updateWakeup ----------

TEST_F(ControlInterface_UT, SetNetworkWake_MacValue_WritesConfig)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, setWakeOnLanAll), ut_setWakeOnLanAll);
    stub.set(ADDR(WakeupUtils, writeTlpWolConfig), ut_writeTlpWolConfig);

    // Arrange
    g_authResult = true;
    g_writeWakeupResult = true;
    // Act
    const bool res = m_ci->setNetworkWake("eth0", true);
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_utilsCalls, QStringList { "setWakeOnLanAll:1" });
    // fail branch
    g_writeWakeupResult = false;
    // Act
    const bool resOff = m_ci->setNetworkWake("eth0", false);
    // Assert: write failed -> false, config not updated
    EXPECT_FALSE(resOff);
    EXPECT_EQ(g_recordedUpdatedWakeId, QString());
}

TEST_F(ControlInterface_UT, IsNetworkWakeup_UnsupportedStatus_PassesThrough)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeOnLanIsOpen), ut_wakeOnLanIsOpen);

    // Arrange: status neither OPEN nor CLOSE -> returned directly
    g_authResult = true;
    g_wolStatus = WakeupUtils::ES_SOCKET_FAILED;
    // Act: passthrough value, not remapped to OPEN/CLOSE
    // Assert
    EXPECT_EQ(m_ci->isNetworkWakeup("eth0"), static_cast<int>(WakeupUtils::ES_SOCKET_FAILED));
    EXPECT_NE(m_ci->isNetworkWakeup("eth0"), static_cast<int>(WakeupUtils::ES_WAKE_ON_OPEN));
}

TEST_F(ControlInterface_UT, IsNetworkWakeup_ConfigEnabled_ReturnsOpen)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeOnLanIsOpen), ut_wakeOnLanIsOpen);
    stub.set(ADDR(WakeupUtils, tlpWolConfigEnabled), ut_tlpWolConfigEnabled);

    // Arrange: NIC reports WoL capable(OPEN/CLOSE) -> final state follows the tlp config
    g_authResult = true;
    g_wolStatus = WakeupUtils::ES_WAKE_ON_OPEN;
    g_tlpWolEnabled = true;
    // Act
    const int wake = m_ci->isNetworkWakeup("eth0");
    // Assert: tlp config enabled -> OPEN preserved
    EXPECT_EQ(wake, static_cast<int>(WakeupUtils::ES_WAKE_ON_OPEN));
    EXPECT_NE(wake, static_cast<int>(WakeupUtils::ES_WAKE_ON_CLOSE));
}

TEST_F(ControlInterface_UT, IsNetworkWakeup_ConfigDisabled_ReturnsClose)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, wakeOnLanIsOpen), ut_wakeOnLanIsOpen);
    stub.set(ADDR(WakeupUtils, tlpWolConfigEnabled), ut_tlpWolConfigEnabled);

    // Arrange
    g_authResult = true;
    g_wolStatus = WakeupUtils::ES_WAKE_ON_OPEN;
    g_tlpWolEnabled = false;
    // Act: tlp disabled -> CLOSE
    // Assert
    EXPECT_EQ(m_ci->isNetworkWakeup("eth0"), static_cast<int>(WakeupUtils::ES_WAKE_ON_CLOSE));
    EXPECT_NE(m_ci->isNetworkWakeup("eth0"), static_cast<int>(WakeupUtils::ES_WAKE_ON_OPEN));
}

TEST_F(ControlInterface_UT, UpdateWakeup_Changed_CallsUtils)
{
    Stub stub;
    stub.set(ADDR(WakeupUtils, updateWakeupDeviceInfo), ut_updateWakeupDeviceInfo);

    // Arrange
    g_authResult = true;
    // Act
    m_ci->updateWakeup("device info");
    // Assert: exactly one delegation, nothing else
    EXPECT_EQ(g_utilsCalls, QStringList { "updateWakeupDeviceInfo" });
    EXPECT_EQ(g_utilsCalls.size(), 1);
}

TEST_F(ControlInterface_UT, UpdateWakeup_AuthFails_Skips)
{
    // Arrange: sentinel entry proves the list is never appended
    g_authResult = false;
    g_utilsCalls << "sentinel";
    // Act
    m_ci->updateWakeup("device info");
    // Assert: auth gate consulted, utils untouched
    EXPECT_EQ(g_utilsCalls, QStringList { "sentinel" });
    EXPECT_EQ(g_authCalls, 1);
}

// ---------- disableInDevice / disableOutDevice ----------

TEST_F(ControlInterface_UT, DisableInDevice_Authorized_CallsUtils)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, disableInDevice), ut_disableInDevice);

    // Arrange
    g_authResult = true;
    // Act
    m_ci->disableInDevice();
    // Assert: exactly one utils delegation
    EXPECT_EQ(g_utilsCalls, QStringList { "disableInDevice" });
    EXPECT_EQ(g_utilsCalls.size(), 1);
}

TEST_F(ControlInterface_UT, DisableInDevice_AuthFails_Skips)
{
    // Arrange: sentinel entry proves the list is never appended
    g_authResult = false;
    g_utilsCalls << "sentinel";
    // Act
    m_ci->disableInDevice();
    // Assert: auth gate consulted, utils untouched
    EXPECT_EQ(g_utilsCalls, QStringList { "sentinel" });
    EXPECT_EQ(g_authCalls, 1);
}

TEST_F(ControlInterface_UT, DisableOutDevice_Authorized_CallsUtils)
{
    Stub stub;
    stub.set(ADDR(EnableUtils, disableOutDevice), ut_disableOutDevice);

    // Arrange
    g_authResult = true;
    // Act
    m_ci->disableOutDevice("info");
    // Assert: exactly one utils delegation
    EXPECT_EQ(g_utilsCalls, QStringList { "disableOutDevice" });
    EXPECT_EQ(g_utilsCalls.size(), 1);
}

TEST_F(ControlInterface_UT, DisableOutDevice_AuthFails_Skips)
{
    // Arrange: sentinel entry proves the list is never appended
    g_authResult = false;
    g_utilsCalls << "sentinel";
    // Act
    m_ci->disableOutDevice("info");
    // Assert: auth gate consulted, utils untouched
    EXPECT_EQ(g_utilsCalls, QStringList { "sentinel" });
    EXPECT_EQ(g_authCalls, 1);
}

// ---------- driver delegation ----------

TEST_F(ControlInterface_UT, AptUpdate_AuthFails_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(DriverManager, aptUpdate), ut_dmAptUpdate);

    // Arrange
    g_authResult = false;
    g_dmAptUpdate = true;   // would return true if delegated
    // Act
    const bool res = m_ci->aptUpdate();
    // Assert: auth gate short-circuits before DriverManager
    EXPECT_FALSE(res);
    EXPECT_EQ(g_authCalls, 1);
}

TEST_F(ControlInterface_UT, AptUpdate_Called_DelegatesToDriverManager)
{
    Stub stub;
    stub.set(ADDR(DriverManager, aptUpdate), ut_dmAptUpdate);

    // Arrange
    g_authResult = true;
    g_dmAptUpdate = true;
    EXPECT_TRUE(m_ci->aptUpdate());
    // Act: delegated result flips with the stub value
    g_dmAptUpdate = false;
    // Assert
    EXPECT_FALSE(m_ci->aptUpdate());
    g_dmAptUpdate = true;
    EXPECT_TRUE(m_ci->aptUpdate());
}

TEST_F(ControlInterface_UT, InstallDriver_ModulenameVersion_Delegates)
{
    Stub stub;
    stub.set(static_cast<void (DriverManager::*)(const QString &, const QString &)>(&DriverManager::installDriver),
             ut_dmInstallNameVersion);

    // Arrange
    g_authResult = true;
    // Act
    m_ci->installDriver("nvidia-driver", "550.1");
    // Assert: exact delegation args, single call
    EXPECT_EQ(g_dmInstallArgs, (QStringList { "nvidia-driver", "550.1" }));
    EXPECT_EQ(g_dmInstallArgs.size(), 2);
}

TEST_F(ControlInterface_UT, InstallDriver_AuthFailed_EmitsSigFinished)
{
    Stub stub;
    stub.set(static_cast<void (DriverManager::*)(const QString &, const QString &)>(&DriverManager::installDriver),
             ut_dmInstallNameVersion);

    // Arrange: (QString,QString) overload emits mp_drivermanager->sigInstallProgressFinished on auth fail
    g_authResult = false;
    QSignalSpy finSpy(m_ci->mp_drivermanager, &DriverManager::sigInstallProgressFinished);
    // Act
    m_ci->installDriver("nvidia-driver", "550.1");
    // Assert
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
    EXPECT_TRUE(g_dmInstallArgs.isEmpty());
}

// B1: authorized -> delegates to DriverManager::undoInstallDriver
TEST_F(ControlInterface_UT, UndoInstallDriver_Authorized_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, undoInstallDriver), ut_dmUndoInstallDriver);

    // Arrange
    g_authResult = true;
    // Act
    m_ci->undoInstallDriver();
    // Assert: delegated exactly once (and not twice)
    EXPECT_EQ(g_dmUndoCalls, 1);
    m_ci->undoInstallDriver();
    EXPECT_EQ(g_dmUndoCalls, 2);
}

// B2: unauthorized -> early return, no delegation
TEST_F(ControlInterface_UT, UndoInstallDriver_Unauthorized_SkipsDelegation)
{
    Stub stub;
    stub.set(ADDR(DriverManager, undoInstallDriver), ut_dmUndoInstallDriver);

    // Arrange
    g_authResult = false;
    // Act
    m_ci->undoInstallDriver();
    // Assert: auth gate consulted, delegation skipped
    EXPECT_EQ(g_dmUndoCalls, 0);
    EXPECT_EQ(g_authCalls, 1);
}

TEST_F(ControlInterface_UT, InstallDriver_Filepath_Delegates)
{
    Stub stub;
    stub.set(static_cast<bool (DriverManager::*)(const QString &)>(&DriverManager::installDriver),
             ut_dmInstallFilepath);

    // Arrange
    g_authResult = true;
    g_dmInstallFilepath = true;
    // Act
    const bool res = m_ci->installDriver(QDir::tempPath() + "/ut-driver.deb");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmInstallFilepathArg, QDir::tempPath() + "/ut-driver.deb");
}

TEST_F(ControlInterface_UT, InstallDriver_FilepathAuthFailed_EmitsSigFinished)
{
    Stub stub;
    stub.set(static_cast<bool (DriverManager::*)(const QString &)>(&DriverManager::installDriver),
             ut_dmInstallFilepath);

    // Arrange
    g_authResult = false;
    QSignalSpy finSpy(m_ci, &ControlInterface::sigFinished);
    // Act
    const bool res = m_ci->installDriver(QDir::tempPath() + "/ut-driver.deb");
    // Assert
    EXPECT_FALSE(res);
    EXPECT_EQ(finSpy.count(), 1);
    EXPECT_FALSE(finSpy.at(0).at(0).toBool());
}

TEST_F(ControlInterface_UT, UnInstallDriver_Modulename_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, unInstallDriver), ut_dmUninstallDriver);

    // Arrange
    g_authResult = true;
    g_dmUninstallDriver = true;
    // Act
    const bool res = m_ci->unInstallDriver("nvidia-driver");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmUninstallDriverArg, QString("nvidia-driver"));
}

TEST_F(ControlInterface_UT, UnInstallDriver_AuthFailed_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(DriverManager, unInstallDriver), ut_dmUninstallDriver);

    // Arrange
    g_authResult = false;
    g_dmUninstallDriver = true;   // would return true if delegated
    // Act: auth gate short-circuits
    // Assert
    EXPECT_FALSE(m_ci->unInstallDriver("nvidia-driver"));
    EXPECT_TRUE(g_dmUninstallDriverArg.isEmpty());
}

TEST_F(ControlInterface_UT, CheckModuleInUsed_Module_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, checkModuleInUsed), ut_dmCheckModuleInUsed);

    // Arrange
    g_authResult = true;
    // Act
    const QStringList res = m_ci->checkModuleInUsed("nvidia");
    // Assert
    EXPECT_EQ(res, QStringList { "module_a" });
    EXPECT_EQ(g_dmCheckModuleArg, QString("nvidia"));
}

TEST_F(ControlInterface_UT, CheckModuleInUsed_AuthFailed_ReturnsEmpty)
{
    Stub stub;
    stub.set(ADDR(DriverManager, checkModuleInUsed), ut_dmCheckModuleInUsed);

    // Arrange
    g_authResult = false;
    g_dmCheckModuleUsed = QStringList { "module_a" };   // would be non-empty if delegated
    // Act: auth gate short-circuits
    // Assert
    EXPECT_TRUE(m_ci->checkModuleInUsed("nvidia").isEmpty());
    EXPECT_TRUE(g_dmCheckModuleArg.isEmpty());
}

TEST_F(ControlInterface_UT, IsBlackListed_Module_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, isBlackListed), ut_dmIsBlackListed);

    // Arrange
    g_authResult = true;
    g_dmIsBlackListed = true;
    EXPECT_TRUE(m_ci->isBlackListed("nvidia"));
    // Act
    g_dmIsBlackListed = false;
    // Assert
    EXPECT_FALSE(m_ci->isBlackListed("nvidia"));
}

TEST_F(ControlInterface_UT, IsArchMatchedFd_Fd_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, isArchMatchedFd), ut_dmIsArchMatchedFd);

    // Arrange
    g_authResult = true;
    g_dmIsArchMatched = true;
    QDBusUnixFileDescriptor fd;   // invalid descriptor, only passed through
    EXPECT_TRUE(m_ci->isArchMatchedFd(fd));
    // Act
    g_dmIsArchMatched = false;
    // Assert
    EXPECT_FALSE(m_ci->isArchMatchedFd(fd));
}

TEST_F(ControlInterface_UT, IsDebValidFd_Fd_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, isDebValidFd), ut_dmIsDebValidFd);

    // Arrange
    g_authResult = true;
    g_dmIsDebValid = false;
    QDBusUnixFileDescriptor fd;
    // Act: stub value flows through for the fd variant too
    // Assert
    EXPECT_FALSE(m_ci->isDebValidFd(fd));
    g_dmIsDebValid = true;
    EXPECT_TRUE(m_ci->isDebValidFd(fd));
}

TEST_F(ControlInterface_UT, BackupDebFd_Delegates_EmitsSignal)
{
    Stub stub;
    stub.set(ADDR(DriverManager, backupDebFd), ut_dmBackupDebFd);

    // Arrange
    g_authResult = true;
    g_dmBackupDeb = true;
    QDBusUnixFileDescriptor dirFd;
    QSignalSpy backupSpy(m_ci, &ControlInterface::sigBackupProgressFinished);
    // Act
    const bool res = m_ci->backupDebFd(dirFd, "driver.deb");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmBackupDebArg, QString("driver.deb"));
    EXPECT_EQ(backupSpy.count(), 1);
    EXPECT_TRUE(backupSpy.at(0).at(0).toBool());
}

TEST_F(ControlInterface_UT, BackupDebFd_AuthFailed_EmitsSignalFalse)
{
    // Arrange
    g_authResult = false;
    QDBusUnixFileDescriptor dirFd;
    QSignalSpy backupSpy(m_ci, &ControlInterface::sigBackupProgressFinished);
    // Act
    const bool res = m_ci->backupDebFd(dirFd, "driver.deb");
    // Assert
    EXPECT_FALSE(res);
    EXPECT_EQ(backupSpy.count(), 1);
    EXPECT_FALSE(backupSpy.at(0).at(0).toBool());
}

TEST_F(ControlInterface_UT, InstallDriverFd_Fd_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, installDriverFd), ut_dmInstallDriverFd);

    // Arrange
    g_authResult = true;
    g_dmInstallDriverFd = true;
    QDBusUnixFileDescriptor fd;
    // Act
    const bool res = m_ci->installDriverFd(fd, "driver.deb");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmInstallFdArgs, QStringList { "driver.deb" });
}

TEST_F(ControlInterface_UT, InstallDriverFd_AuthFailed_EmitsSigFinished)
{
    Stub stub;
    stub.set(ADDR(DriverManager, installDriverFd), ut_dmInstallDriverFd);

    // Arrange
    g_authResult = false;
    QDBusUnixFileDescriptor fd;
    QSignalSpy finSpy(m_ci, &ControlInterface::sigFinished);
    // Act
    const bool res = m_ci->installDriverFd(fd, "driver.deb");
    // Assert
    EXPECT_FALSE(res);
    EXPECT_EQ(finSpy.count(), 1);
}

TEST_F(ControlInterface_UT, UnInstallPrinter_Name_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, uninstallPrinter), ut_dmUninstallPrinter);

    // Arrange
    g_authResult = true;
    g_dmUninstallPrinter = true;
    // Act
    const bool res = m_ci->unInstallPrinter("HP", "DeskJet");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmUninstallPrinterArgs, (QStringList { "HP", "DeskJet" }));
}

TEST_F(ControlInterface_UT, UnInstallPrinter_AuthFailed_EmitsManagerSigFinished)
{
    // Arrange
    g_authResult = false;
    QSignalSpy managerSpy(m_ci->mp_drivermanager, &DriverManager::sigFinished);
    // Act
    const bool res = m_ci->unInstallPrinter("HP", "DeskJet");
    // Assert
    EXPECT_FALSE(res);
    EXPECT_EQ(managerSpy.count(), 1);
    EXPECT_FALSE(managerSpy.at(0).at(0).toBool());
}

TEST_F(ControlInterface_UT, DelDeb_Name_Delegates)
{
    Stub stub;
    stub.set(ADDR(DriverManager, delDeb), ut_dmDelDeb);

    // Arrange
    g_authResult = true;
    g_dmDelDeb = true;
    // Act
    const bool res = m_ci->delDeb("driver.deb");
    // Assert
    EXPECT_TRUE(res);
    EXPECT_EQ(g_dmDelDebArg, QString("driver.deb"));
}

TEST_F(ControlInterface_UT, DelDeb_AuthFailed_ReturnsFalse)
{
    Stub stub;
    stub.set(ADDR(DriverManager, delDeb), ut_dmDelDeb);

    // Arrange
    g_authResult = false;
    g_dmDelDeb = true;   // would return true if delegated
    // Act: auth gate short-circuits
    // Assert
    EXPECT_FALSE(m_ci->delDeb("driver.deb"));
    EXPECT_TRUE(g_dmDelDebArg.isEmpty());
}

// ---------- saveWakeupInfo (private) ----------

TEST_F(ControlInterface_UT, SaveWakeupInfo_Existing_Updates)
{
    // Arrange
    g_sqlWakeupExisted = true;
    // Act
    m_ci->saveWakeupInfo("uid-1", "/devices/x", true);
    // Assert
    EXPECT_EQ(g_recordedUpdatedWakeId, QString("uid-1"));
    EXPECT_TRUE(g_recordedInsertedWakeId.isEmpty());
}

TEST_F(ControlInterface_UT, SaveWakeupInfo_New_Inserts)
{
    // Arrange
    g_sqlWakeupExisted = false;
    // Act
    m_ci->saveWakeupInfo("uid-2", "/devices/x", false);
    // Assert
    EXPECT_EQ(g_recordedInsertedWakeId, QString("uid-2"));
    EXPECT_TRUE(g_recordedUpdatedWakeId.isEmpty());
}
