// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include <gtest/gtest.h>
#include "drivercontrol/drivermanager.h"
#include "drivercontrol/aptinstaller.h"
#include "drivercontrol/driverinstallerapt.h"
#include "DDLog.h"

#include <QThread>

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QDBusUnixFileDescriptor>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QNetworkConfigurationManager>
#else
#include <QNetworkInformation>
#endif

#include "../stub.h"
#include "commonfunction.h"  // EC_NULL / EC_NETWORK 等错误码
#include "drivercontrol/utils.h"  // Utils::kernelRelease
#include <QtTest/QSignalSpy>

/*
 * Branch list for DriverManager::parsePrinterInfo (private, pure JSON parsing):
 *
 *   B1: empty byteArray                -> empty list    | ParsePrinterInfo_EmptyInput_ReturnsEmptyList
 *   B2: invalid JSON                   -> empty list    | ParsePrinterInfo_InvalidJson_ReturnsEmptyList
 *   B3: valid JSON, no "solutions" key -> empty list    | ParsePrinterInfo_NoSolutionsKey_ReturnsEmptyList
 *   B4: valid JSON, empty solutions    -> empty list    | ParsePrinterInfo_EmptySolutions_ReturnsEmptyList
 *   B5: valid JSON, single solution    -> 1 item        | ParsePrinterInfo_SingleSolution_ReturnsOneItem
 *   B6: valid JSON, multiple solutions -> N items       | ParsePrinterInfo_MultipleSolutions_ReturnsAllItems
 *   B7: solution with missing fields   -> default vals  | ParsePrinterInfo_MissingFields_ReturnsDefaults
 *   B8: solution entry is not object   -> skipped       | ParsePrinterInfo_NonObjectEntry_HandledGracefully
 *
 * NOTE: -fno-access-control is set in CMakeLists.txt, so private methods
 * are directly accessible without addr_pri macros.
 */

class DriverManager_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }

    // Helper: build a JSON byte array with a "solutions" array
    static QByteArray makeSolutionsJson(const QJsonArray &solutions)
    {
        QJsonObject root;
        root["solutions"] = solutions;
        QJsonDocument doc(root);
        return doc.toJson(QJsonDocument::Compact);
    }

    // Helper: build a single solution JSON object
    static QJsonObject makeSolution(int sid, const QString &describe,
                                    const QString &ppd, bool excat,
                                    const QString &driver)
    {
        QJsonObject obj;
        obj["sid"] = sid;
        obj["describe"] = describe;
        obj["ppd"] = ppd;
        obj["excat"] = excat;
        obj["driver"] = driver;
        return obj;
    }
};

// ===== parsePrinterInfo =====

TEST_F(DriverManager_UT, ParsePrinterInfo_EmptyInput_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QByteArray empty;
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(empty);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_InvalidJson_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QByteArray invalidJson = "{this is not valid json}";
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(invalidJson);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_NoSolutionsKey_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QJsonObject root;
    root["other_key"] = "some_value";
    QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Compact);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_EmptySolutions_ReturnsEmptyList)
{
    // Arrange
    DriverManager mgr;
    QJsonArray emptyArray;
    QByteArray json = makeSolutionsJson(emptyArray);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    EXPECT_TRUE(result.isEmpty());
    EXPECT_EQ(result.size(), 0);
}

TEST_F(DriverManager_UT, ParsePrinterInfo_SingleSolution_ReturnsOneItem)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    solutions.append(makeSolution(1, "HP LaserJet", "hp-laserjet.ppd", true, "hplip"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].intSid, 1);
    EXPECT_EQ(result[0].strDescribe.toStdString(), "HP LaserJet");
    EXPECT_EQ(result[0].strPpd.toStdString(), "hp-laserjet.ppd");
    EXPECT_TRUE(result[0].isExcat);
    EXPECT_EQ(result[0].strDriver.toStdString(), "hplip");
}

TEST_F(DriverManager_UT, ParsePrinterInfo_MultipleSolutions_ReturnsAllItems)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    solutions.append(makeSolution(1, "HP LaserJet", "hp-laserjet.ppd", true, "hplip"));
    solutions.append(makeSolution(2, "Canon PIXMA", "canon-pixma.ppd", false, "cnijfilter"));
    solutions.append(makeSolution(3, "Epson EcoTank", "epson-ecotank.ppd", true, "epson-inkjet"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].intSid, 1);
    EXPECT_EQ(result[1].intSid, 2);
    EXPECT_EQ(result[2].intSid, 3);
    EXPECT_EQ(result[1].strDescribe.toStdString(), "Canon PIXMA");
    EXPECT_FALSE(result[1].isExcat);
    EXPECT_EQ(result[2].strDriver.toStdString(), "epson-inkjet");
}

TEST_F(DriverManager_UT, ParsePrinterInfo_MissingFields_ReturnsDefaults)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    // Solution object with only "sid", other fields missing
    QJsonObject partial;
    partial["sid"] = 42;
    solutions.append(partial);
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].intSid, 42);
    EXPECT_TRUE(result[0].strDescribe.isEmpty());
    EXPECT_TRUE(result[0].strPpd.isEmpty());
    EXPECT_FALSE(result[0].isExcat);
    EXPECT_TRUE(result[0].strDriver.isEmpty());
}

TEST_F(DriverManager_UT, ParsePrinterInfo_NonObjectEntry_HandledGracefully)
{
    // Arrange
    DriverManager mgr;
    QJsonArray solutions;
    // Add a non-object value (string) followed by a valid object
    solutions.append("not_an_object");
    solutions.append(makeSolution(5, "Brother HL", "brother.ppd", false, "brother-driver"));
    QByteArray json = makeSolutionsJson(solutions);
    // Act
    QList<DriverManager::TDriverInfo> result = mgr.parsePrinterInfo(json);
    // Assert — toObject() on a non-object returns empty QJsonObject,
    // so the first entry produces default values; the second is parsed correctly
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0].intSid, 0);
    EXPECT_TRUE(result[0].strDescribe.isEmpty());
    EXPECT_EQ(result[1].intSid, 5);
    EXPECT_EQ(result[1].strDescribe.toStdString(), "Brother HL");
}

// ==================== T24 补充覆盖：HIGH 方法与外部依赖打桩 ====================
//
// 覆盖目标（对应 tests/.ut/inventory.json 中 DriverManager HIGH 项）：
//   ctor / ~DriverManager / initConnections / aptUpdate / isNetworkOnline
//   installDriverFd / backupDebFd / isArchMatchedFd / isDebValidFd / installDriver(pkg,ver)
// 打桩策略（dependency-tracer 矩阵：外部进程/网络一律 stub）：
//   - QProcess::start / waitForFinished → 空操作（apt/depmod/ping/update-initramfs 不真执行）
//   - DriverInstallerApt::installPackage → 空操作（installDriver(pkg,ver) 不触发真实 apt）
//   - isNetworkOnline 的 ping 结果通过受控的 CWD 相对文件 netlog.bat 喂入

namespace {

void ut_stubQProcessStart(QProcess *, const QString &, const QStringList &, QIODevice::OpenMode)
{
    // 不执行任何外部进程
}

bool ut_stubQProcessWaitFinished(QProcess *, int)
{
    return true;
}

bool g_stubInstallerCalled = false;

void ut_stubInstallerInstallPackage(DriverInstallerApt *, const QString &, const QString &)
{
    g_stubInstallerCalled = true;
}

}  // namespace

class DriverManagerExtra_UT : public UT_HEAD
{
public:
    void SetUp() override { }
    void TearDown() override { QFile::remove("netlog.bat"); }

    // isNetworkOnline 以固定相对路径 "netlog.bat" 读写 ping 输出，这里在 CWD 受控构造
    static void writeNetlog(const QStringList &lines)
    {
        QFile f("netlog.bat");
        EXPECT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        for (const QString &line : lines)
            f.write((line + "\n").toUtf8());
    }

    // 在进程私有临时目录中构造内容文件
    static QString makeContentFile(const QString &name, const QByteArray &content)
    {
        const QString dir = QDir::tempPath() + "/ut_drivermgr_fd_" + QString::number(::getpid());
        QDir().mkpath(dir);
        const QString path = dir + "/" + name;
        QFile f(path);
        EXPECT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(content);
        return path;
    }
};

// ===== ctor / ~DriverManager / initConnections =====

TEST_F(DriverManagerExtra_UT, CtorDtor_InitializesThreadsAndModCore)
{
    DriverManager *mgr = new DriverManager;
    EXPECT_NE(mgr->mp_modcore, nullptr);
    EXPECT_NE(mgr->mp_debinstaller, nullptr);
    EXPECT_NE(mgr->mp_driverInstaller, nullptr);
    EXPECT_TRUE(mgr->mp_deboperatethread->isRunning());
    EXPECT_TRUE(mgr->mp_driverOperateThread->isRunning());
    delete mgr;  // ~DriverManager: quit + wait 两个工作线程，不应挂起
}

TEST_F(DriverManagerExtra_UT, InitConnections_AptInstallerSignals_RetargetProgressAndFinish)
{
    DriverManager mgr;
    int progressValue = -1;
    int finishCount = 0;
    bool finishSuccess = true;
    QString finishMsg;
    QObject::connect(&mgr, &DriverManager::sigProgressDetail, [&](int p, const QString &) {
        progressValue = p;
    });
    QObject::connect(&mgr, &DriverManager::sigFinished, [&](bool ok, QString msg) {
        ++finishCount;
        finishSuccess = ok;
        finishMsg = msg;
    });

    // progressChanged: m_installprocess = 20 + progress * 0.8
    emit mgr.mp_debinstaller->progressChanged(25);
    EXPECT_EQ(progressValue, 40);

    // installFinished(true): sigProgressDetail(90) + sigFinished(true)
    emit mgr.mp_debinstaller->installFinished(true);
    EXPECT_EQ(progressValue, 90);
    EXPECT_EQ(finishCount, 1);
    EXPECT_TRUE(finishSuccess);

    // installFinished(false): sigFinished(false)
    emit mgr.mp_debinstaller->installFinished(false);
    EXPECT_EQ(finishCount, 2);
    EXPECT_FALSE(finishSuccess);

    // errorOccurred: sigFinished(false, errmsg) 并记录 errmsg
    emit mgr.mp_debinstaller->errorOccurred(QString("boom"));
    EXPECT_EQ(finishCount, 3);
    EXPECT_FALSE(finishSuccess);
    EXPECT_EQ(finishMsg.toStdString(), "boom");
}

TEST_F(DriverManagerExtra_UT, InitConnections_DriverInstallerSignals_RelayAndHandleErrors)
{
    DriverManager mgr;
    int installFinishedCount = 0;
    bool installFinishedSuccess = true;
    int installFinishedErr = -1;
    int progressChangedValue = -1;
    QObject::connect(&mgr, &DriverManager::sigInstallProgressFinished, [&](bool ok, int err) {
        ++installFinishedCount;
        installFinishedSuccess = ok;
        installFinishedErr = err;
    });
    QObject::connect(&mgr, &DriverManager::sigInstallProgressChanged, [&](int p) {
        progressChangedValue = p;
    });

    // installProgressFinished(true) → sigInstallProgressFinished(true, EC_NULL)
    emit mgr.mp_driverInstaller->installProgressFinished(true);
    EXPECT_EQ(installFinishedCount, 1);
    EXPECT_TRUE(installFinishedSuccess);
    EXPECT_EQ(installFinishedErr, EC_NULL);

    // installProgressChanged 直接透传
    emit mgr.mp_driverInstaller->installProgressChanged(50);
    EXPECT_EQ(progressChangedValue, 50);

    // 非 EC_NETWORK 错误直接转发：sigInstallProgressFinished(false, err)
    emit mgr.mp_driverInstaller->errorOccurred(99);
    EXPECT_EQ(installFinishedCount, 2);
    EXPECT_FALSE(installFinishedSuccess);
    EXPECT_EQ(installFinishedErr, 99);
    // EC_NETWORK 分支含 30s 重试循环，超时成本过高，不在单测内覆盖
}

// ===== installDriver(pkgName, version)（打桩 DriverInstallerApt::installPackage） =====

TEST_F(DriverManagerExtra_UT, InstallDriver_PkgVersion_ForwardsToInstaller)
{
    Stub stub;
    stub.set(ADDR(DriverInstallerApt, installPackage), ut_stubInstallerInstallPackage);
    g_stubInstallerCalled = false;

    DriverManager mgr;
    EXPECT_TRUE(mgr.mp_driverOperateThread->isRunning());
    mgr.installDriver("ut-fake-pkg", "1.0.0");
    EXPECT_TRUE(g_stubInstallerCalled);
}

// ===== aptUpdate（打桩 QProcess，cmdAptUpdate 不真跑 apt update） =====

TEST_F(DriverManagerExtra_UT, AptUpdate_StubbedProcess_ReturnsTrue)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);

    DriverManager mgr;
    EXPECT_TRUE(mgr.aptUpdate());
}

// ===== isNetworkOnline（打桩 ping，受控 netlog.bat） =====

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_StubbedPing_ParsesNetlogResult)
{
    bool hostOnline = true;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QNetworkInformation *netInfo = QNetworkInformation::instance();
    hostOnline = (netInfo != nullptr
                  && netInfo->reachability() != QNetworkInformation::Reachability::Disconnected);
#else
    QNetworkConfigurationManager netCfg;
    hostOnline = netCfg.isOnline();
#endif

    DriverManager mgr;
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);

    // 0% 丢包 → 在线（仅当宿主机网络可达时才会执行到解析分支）
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes"
                              << "2 packets transmitted, 2 received, 0% packet loss, time 1001ms"
                              << "round-trip min/avg/max = 10.1/10.2/10.3 ms");
    const bool online = mgr.isNetworkOnline();
    if (hostOnline) {
        EXPECT_TRUE(online);
    } else {
        SUCCEED() << "宿主机网络不可达（QNetworkInformation 不可用或离线），在线分支跳过";
    }

    // 100% 丢包 → 无论宿主机状态如何均返回 false
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes"
                              << "3 packets transmitted, 0 received, 100% packet loss, time 2000ms"
                              << "--- www.baidu.com ping statistics ---");
    EXPECT_FALSE(mgr.isNetworkOnline());
}

// ===== isArchMatchedFd / isDebValidFd / installDriverFd / backupDebFd（fd 系列） =====

TEST_F(DriverManagerExtra_UT, FdChecks_InvalidFd_RejectsAll)
{
    DriverManager mgr;
    QDBusUnixFileDescriptor invalid;
    EXPECT_FALSE(invalid.isValid());

    int finishCount = 0;
    QString finishMsg;
    QObject::connect(&mgr, &DriverManager::sigFinished, [&](bool, QString msg) {
        ++finishCount;
        finishMsg = msg;
    });

    EXPECT_FALSE(mgr.isArchMatchedFd(invalid));
    EXPECT_FALSE(mgr.isDebValidFd(invalid));
    // installDriverFd: 无效 fd → errmsg=E_FILE_NOT_EXISTED(100) + sigFinished(false)
    EXPECT_FALSE(mgr.installDriverFd(invalid, "driver.deb"));
    EXPECT_EQ(finishCount, 1);
    EXPECT_EQ(finishMsg.toStdString(), "100");
}

TEST_F(DriverManagerExtra_UT, FdChecks_NonDebTextFile_RejectsValidation)
{
    const QString path = makeContentFile("plain.txt", "not a deb package\n");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    QDBusUnixFileDescriptor fd(f.handle());
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    // 桥接成功后无 mime 快捷分支，两个检查都委托给 AptInstaller（dpkg -I）：
    // 文本内容无 Architecture 字段 → isArchMatchedFd 返回 false；
    // dpkg -I 退出码非 0 → isDebValidFd 返回 false（桥接临时文件由被测函数清理）
    EXPECT_FALSE(mgr.isArchMatchedFd(fd));
    EXPECT_FALSE(mgr.isDebValidFd(fd));
}

TEST_F(DriverManagerExtra_UT, FdChecks_RealDebContent_AcceptsArchAndValidity)
{
    // 用 dpkg-deb 现场构建一个最小合法包（Architecture: all），离线无副作用：
    // isArchMatchedFd：Architecture=all 匹配 → true；isDebValidFd：dpkg -I 退出码 0 → true
    const QString workDir = QDir::tempPath() + "/ut_drivermgr_deb_" + QString::number(::getpid());
    QDir().mkpath(workDir + "/pkg/DEBIAN");
    {
        QFile ctl(workDir + "/pkg/DEBIAN/control");
        ASSERT_TRUE(ctl.open(QIODevice::WriteOnly | QIODevice::Truncate));
        ctl.write("Package: ut-t24-fake-pkg\nVersion: 1.0\nArchitecture: all\n"
                  "Maintainer: ut <ut@uniontech.com>\nDescription: minimal package for unit test\n");
    }
    const QString debPath = workDir + "/ut-t24-fake-pkg_1.0_all.deb";
    const int rc = QProcess::execute("dpkg-deb", QStringList() << "--build"
                                  << workDir + "/pkg" << debPath);
    ASSERT_EQ(rc, 0);
    ASSERT_TRUE(QFile::exists(debPath));

    QFile f(debPath);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    QDBusUnixFileDescriptor fd(f.handle());
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    EXPECT_TRUE(mgr.isArchMatchedFd(fd));
    EXPECT_TRUE(mgr.isDebValidFd(fd));

    QDir(workDir).removeRecursively();
}

TEST_F(DriverManagerExtra_UT, FdChecks_FakeDebContent_RejectsArchAndValidity)
{
    // 内容满足 shared-mime-info 的 ar/deb 魔数（"!<arch>\ndebian"），
    // dpkg -I 对该伪包必然失败 → isArchMatchedFd / isDebValidFd 均返回 false
    const QString path = makeContentFile("fake.deb",
                                         QByteArray("!<arch>\ndebian-binary   "
                                                    "0           0     0     644     4        `"));
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    QDBusUnixFileDescriptor fd(f.handle());
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    EXPECT_FALSE(mgr.isArchMatchedFd(fd));
    EXPECT_FALSE(mgr.isDebValidFd(fd));
}

TEST_F(DriverManagerExtra_UT, InstallDriverFd_TextFile_RejectsAsNonDriver)
{
    const QString path = makeContentFile("driver.txt", "plain text driver candidate\n");
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    QDBusUnixFileDescriptor fd(f.handle());
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    int finishCount = 0;
    QString finishMsg;
    QObject::connect(&mgr, &DriverManager::sigFinished, [&](bool, QString msg) {
        ++finishCount;
        finishMsg = msg;
    });

    // 桥接成功但内容非驱动包：installDriver 路径守卫放行 fdbridge 目录，
    // isDriverPackage 为 false → errmsg=E_NOT_DRIVER(101) + sigFinished(false)
    EXPECT_FALSE(mgr.installDriverFd(fd, "driver.txt"));
    EXPECT_EQ(finishCount, 1);
    EXPECT_EQ(finishMsg.toStdString(), "101");
}

TEST_F(DriverManagerExtra_UT, BackupDebFd_ValidNameInvalidFd_ReturnsFalse)
{
    DriverManager mgr;
    // 名字合法但 dirFd 无效：直接拒绝，不触达磁盘
    EXPECT_FALSE(mgr.backupDebFd(QDBusUnixFileDescriptor(), "ut-backup-deb"));
}

TEST_F(DriverManagerExtra_UT, BackupDebFd_ValidFd_NonRootDenied)
{
    if (::geteuid() == 0)
        GTEST_SKIP() << "root 环境会真实写入 /var/lib 备份目录，跳过该用例";

    const QString dir = QDir::tempPath() + "/ut_drivermgr_dirfd_" + QString::number(::getpid());
    ASSERT_TRUE(QDir().mkpath(dir));
    const int dirfd = ::open(dir.toLocal8Bit().constData(), O_RDONLY | O_DIRECTORY);
    ASSERT_GE(dirfd, 0);
    QDBusUnixFileDescriptor fd(dirfd);
    ::close(dirfd);
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    // 非root 无法 mkpath /var/lib/deepin-devicemanager/driver/<name> → mkpath 失败返回 false
    EXPECT_FALSE(mgr.backupDebFd(fd, "ut-backup-deb"));
}

// ===== delDeb / unInstallDriver / 查询辅助 =====

TEST_F(DriverManagerExtra_UT, DelDeb_NonexistentBackup_ReturnsTrue)
{
    DriverManager mgr;
    // 备份目录不存在：不做删除，直接返回 !exists == true
    EXPECT_TRUE(mgr.delDeb("ut_no_such_backup_deb_xyz"));
}

TEST_F(DriverManagerExtra_UT, UnInstallDriver_NonexistentModule_FailsWithSignal)
{
    DriverManager mgr;
    int finishCount = 0;
    bool finishSuccess = true;
    QObject::connect(&mgr, &DriverManager::sigFinished, [&](bool ok, QString) {
        ++finishCount;
        finishSuccess = ok;
    });

    // 不存在的模块：modGetPath 为空 → file2PackageName 为空 → unInstallModule →
    // rmModForce 删除失败 → 返回 false 并发出 sigFinished(false)
    EXPECT_FALSE(mgr.unInstallDriver("ut_no_such_mod_t24xyz"));
    EXPECT_EQ(finishCount, 1);
    EXPECT_FALSE(finishSuccess);
}

TEST_F(DriverManagerExtra_UT, QueryHelpers_NonexistentInputs_ReturnEmptyOrFalse)
{
    DriverManager mgr;
    EXPECT_TRUE(mgr.checkModuleInUsed("ut_no_such_mod_t24xyz").isEmpty());
    EXPECT_FALSE(mgr.isBlackListed("ut_no_such_mod_t24xyz"));
    EXPECT_FALSE(mgr.isDriverPackage("/nonexistent/path/foo.ko"));
    // sudo dpkg -s 查询不存在的包：stdout 无内容 → false（无系统副作用）
    EXPECT_FALSE(mgr.printerHasInstalled("ut-no-such-pkg-t24xyz"));
}

TEST_F(DriverManagerExtra_UT, UndoInstallDriver_OnlineState_DelegatesSafely)
{
    DriverManager mgr;
    // m_IsNetworkOnline 默认 true → 转发给 DriverInstallerApt::undoInstallDriver
    // （无进行中的安装：m_process 为空，仅置取消标记，安全无操作）
    mgr.undoInstallDriver();
    SUCCEED();
}

// ===================== 第二批：覆盖缺口补强 =====================
// isNetworkOnline：QNetworkInformation 双打桩（instance + reachability）使命运不受宿主机网络状态影响；
// initConnections EC_NETWORK 分支：借助 m_StopQueryNetwork / 打桩 isNetworkOnline 立即退出等待循环；
// 打印机安装/卸载：QProcess 打桩避免真实 sudo 调用；
// installDriverFd 正向内容：使用真实内核模块 bfq.ko（只读）桥接，走 .ko 安装分支至 isSigned 失败。

namespace {

QNetworkInformation *ut_stubNetInfoInstance()
{
    // 不被解引用：reachability() 已同步打桩，仅要求非空
    return reinterpret_cast<QNetworkInformation *>(0x1);
}

QNetworkInformation::Reachability ut_stubNetInfoSite(QNetworkInformation *)
{
    return QNetworkInformation::Reachability::Site;
}

QNetworkInformation::Reachability ut_stubNetInfoDisconnected(QNetworkInformation *)
{
    return QNetworkInformation::Reachability::Disconnected;
}

QProcess::ExitStatus ut_stubQProcessCrashExit(QProcess *)
{
    return QProcess::CrashExit;
}

// 层级校验关闭（NotReady != Available）→ isSigned 走真实校验器分支
Utils::HierarchicalVerifyStatus ut_stubHierDisabled()
{
    return Utils::HierarchicalVerifyNotReady;
}

// 开发者模式检查绕过（本机 com.deepin.sync.Helper DeveloperMode=true，会使 isSigned 提前返回 true）
QVariant ut_stubObjPropertyFalse(QObject *, const char *)
{
    return QVariant(false);
}

void stubNetworkProbeOnline(Stub &stub)
{
    stub.set(ADDR(QNetworkInformation, instance), ut_stubNetInfoInstance);
    stub.set(ADDR(QNetworkInformation, reachability), ut_stubNetInfoSite);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);
}

} // namespace

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_ForceOnlineZeroLoss_ReturnsTrue)
{
    // QNetworkInformation 双打桩 + ping 打桩 → 覆盖在线探测与 netlog 解析全部分支
    Stub stub;
    stubNetworkProbeOnline(stub);
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes"
                              << "2 packets transmitted, 2 received, 0% packet loss, time 1001ms"
                              << "round-trip min/avg/max = 10.1/10.2/10.3 ms");
    DriverManager mgr;
    EXPECT_TRUE(mgr.isNetworkOnline());
}

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_ForceOnlineFullLoss_ReturnsFalse)
{
    Stub stub;
    stubNetworkProbeOnline(stub);
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes"
                              << "3 packets transmitted, 0 received, 100% packet loss, time 2000ms"
                              << "--- www.baidu.com ping statistics ---");
    DriverManager mgr;
    EXPECT_FALSE(mgr.isNetworkOnline());
}

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_PingProcessCrashed_ReturnsFalse)
{
    // exitStatus 打桩为 CrashExit → 覆盖 !bRet 早退分支
    Stub stub;
    stubNetworkProbeOnline(stub);
    stub.set(ADDR(QProcess, exitStatus), ut_stubQProcessCrashExit);
    writeNetlog(QStringList() << "anything");
    DriverManager mgr;
    EXPECT_FALSE(mgr.isNetworkOnline());
}

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_LogTooShort_ReturnsFalse)
{
    // netlog 仅 1 行 → v.size()>1 为假 → 返回 0
    Stub stub;
    stubNetworkProbeOnline(stub);
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes");
    DriverManager mgr;
    EXPECT_FALSE(mgr.isNetworkOnline());
}

TEST_F(DriverManagerExtra_UT, IsNetworkOnline_NoReceivedMarker_ReturnsFalse)
{
    // 倒数第二行无 "received," 标记 → iPos==-1 分支
    Stub stub;
    stubNetworkProbeOnline(stub);
    writeNetlog(QStringList() << "PING www.baidu.com (1.2.3.4): 56 data bytes"
                              << "no marker line here"
                              << "--- statistics ---");
    DriverManager mgr;
    EXPECT_FALSE(mgr.isNetworkOnline());
}

TEST_F(DriverManagerExtra_UT, IsNetworkOffline_DisconnectedReachability_ReturnsFalse)
{
    // reachability 打桩为 Disconnected → 覆盖 Qt6 早退分支
    Stub stub;
    stub.set(ADDR(QNetworkInformation, instance), ut_stubNetInfoInstance);
    stub.set(ADDR(QNetworkInformation, reachability), ut_stubNetInfoDisconnected);
    DriverManager mgr;
    EXPECT_FALSE(mgr.isNetworkOnline());
}

// ===== initConnections：EC_NETWORK 等待分支（立即退出路径） =====

TEST_F(DriverManagerExtra_UT, InitConnections_NetworkError_CancelledImmediately)
{
    // m_StopQueryNetwork=true → 等待循环首轮即退出（避免真实 30s 等待）
    DriverManager mgr;
    mgr.m_StopQueryNetwork = true;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigInstallProgressFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    emit mgr.mp_driverInstaller->errorOccurred(EC_NETWORK);

    // 入口先发 EC_NOTIFY_NETWORK，取消后发 EC_CANCEL
    ASSERT_EQ(finishedSpy.count(), 2);
    EXPECT_FALSE(finishedSpy.at(1).at(0).toBool());
    EXPECT_EQ(finishedSpy.at(1).at(1).toInt(), EC_CANCEL);
    // 循环内已复位取消标记
    EXPECT_FALSE(mgr.m_StopQueryNetwork);
}

TEST_F(DriverManagerExtra_UT, InitConnections_NetworkError_OnlineRecovers_Reinstall)
{
    // isNetworkOnline 打桩为 true → 等待循环首轮命中 EC_REINSTALL
    Stub stub;
    stubNetworkProbeOnline(stub);
    writeNetlog(QStringList() << "2 packets transmitted, 2 received, 0% packet loss, time 1001ms"
                              << "round-trip min/avg/max = 10.1/10.2/10.3 ms");

    DriverManager mgr;
    mgr.m_StopQueryNetwork = false;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigInstallProgressFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    emit mgr.mp_driverInstaller->errorOccurred(EC_NETWORK);

    ASSERT_GE(finishedSpy.count(), 2);
    EXPECT_FALSE(finishedSpy.at(finishedSpy.count() - 1).at(0).toBool());
    EXPECT_EQ(finishedSpy.at(finishedSpy.count() - 1).at(1).toInt(), EC_REINSTALL);
}

TEST_F(DriverManagerExtra_UT, InitConnections_InstallFinished_SuccessCleansBridgedFile)
{
    // 桥接临时文件在安装成功后被清理（真实 bridge 目录，removeBridgedTempFile 只删该前缀目录）
    const QString bridgeDir = QDir::tempPath() + "/devicemanager-fdbridge-utt24";
    QDir().mkpath(bridgeDir);
    const QString bridged = bridgeDir + "/check.deb";
    { QFile f(bridged); ASSERT_TRUE(f.open(QIODevice::WriteOnly)); f.write("x"); }

    DriverManager mgr;
    mgr.m_fdBridgePath = bridged;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    emit mgr.mp_debinstaller->installFinished(true);

    EXPECT_TRUE(mgr.m_fdBridgePath.isEmpty());
    EXPECT_FALSE(QFile::exists(bridged));
    EXPECT_FALSE(QDir(bridgeDir).exists());
    ASSERT_GE(finishedSpy.count(), 1);
    EXPECT_TRUE(finishedSpy.at(finishedSpy.count() - 1).at(0).toBool());
}

TEST_F(DriverManagerExtra_UT, InitConnections_InstallProgressFinished_False_LoggedOnly)
{
    // installProgressFinished(false) 仅记日志，不再转发（unknown reason 分支）
    DriverManager mgr;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigInstallProgressFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    emit mgr.mp_driverInstaller->installProgressFinished(false);

    EXPECT_EQ(finishedSpy.count(), 0);
}

namespace {
// QDateTime::currentSecsSinceEpoch 状态化打桩：首次返回基准值，第二次 +1s，
// 之后跳到 +31s → 网络等待循环执行一轮（覆盖 sleep(1)）后立即超时退出
qint64 ut_stubSecsSinceEpoch()
{
    static int calls = 0;
    static qint64 base = 1700000000;
    ++calls;
    if (calls == 1)
        return base;
    if (calls == 2)
        return base + 1;
    return base + 31;
}
} // namespace

TEST_F(DriverManagerExtra_UT, InitConnections_NetworkError_Timeout_EmitsEcNetwork)
{
    // 网络探测打桩为离线 + 时钟跳变 → 等待循环一轮后超时，最终 EC_NETWORK
    Stub stub;
    stub.set(ADDR(QNetworkInformation, instance), ut_stubNetInfoInstance);
    stub.set(ADDR(QNetworkInformation, reachability), ut_stubNetInfoDisconnected);
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);
    stub.set(ADDR(QDateTime, currentSecsSinceEpoch), ut_stubSecsSinceEpoch);

    DriverManager mgr;
    mgr.m_StopQueryNetwork = false;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigInstallProgressFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    emit mgr.mp_driverInstaller->errorOccurred(EC_NETWORK);

    ASSERT_GE(finishedSpy.count(), 2);
    EXPECT_FALSE(finishedSpy.at(finishedSpy.count() - 1).at(0).toBool());
    EXPECT_EQ(finishedSpy.at(finishedSpy.count() - 1).at(1).toInt(), EC_NETWORK);
    EXPECT_TRUE(mgr.m_IsNetworkOnline);
}

// ===== 桥接失败分支（管道 fd 非普通文件） =====

TEST_F(DriverManagerExtra_UT, FdChecks_BridgeFail_PipeFd_RejectsValidation)
{
    // 管道 fd 不是普通文件 → bridgeFdToTempFile 失败 → 各校验返回 false
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);

    DriverManager mgr;
    QDBusUnixFileDescriptor fd(fds[0]);
    EXPECT_TRUE(fd.isValid());
    EXPECT_FALSE(mgr.isArchMatchedFd(fd));
    EXPECT_FALSE(mgr.isDebValidFd(fd));

    ::close(fds[0]);
    ::close(fds[1]);
}

TEST_F(DriverManagerExtra_UT, InstallDriverFd_BridgeFail_PipeFd_Emits100)
{
    int fds[2];
    ASSERT_EQ(::pipe(fds), 0);

    DriverManager mgr;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    QDBusUnixFileDescriptor fd(fds[0]);
    EXPECT_FALSE(mgr.installDriverFd(fd, "check.deb"));
    ASSERT_GE(finishedSpy.count(), 1);
    EXPECT_FALSE(finishedSpy.at(finishedSpy.count() - 1).at(0).toBool());
    EXPECT_EQ(finishedSpy.at(finishedSpy.count() - 1).at(1).toString(), QStringLiteral("100"));

    ::close(fds[0]);
    ::close(fds[1]);
}

// ===== 打印机安装/卸载（QProcess 打桩） =====

TEST_F(DriverManagerExtra_UT, InstallPrinter_StubbedProcess_NotInstalled_ReturnsFalse)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);
    // printerHasInstalled 实查（无此包）→ false
    DriverManager mgr;
    EXPECT_FALSE(mgr.installPrinter("ut-fake-printer-pkg-t24"));
}

TEST_F(DriverManagerExtra_UT, UnInstallPrinter_StubbedProcess_ReturnsTrue)
{
    Stub stub;
    stub.set((void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode))ADDR(QProcess, start),
             ut_stubQProcessStart);
    stub.set(ADDR(QProcess, waitForFinished), ut_stubQProcessWaitFinished);
    // !printerHasInstalled → 包不存在视为卸载成功
    DriverManager mgr;
    EXPECT_TRUE(mgr.unInstallPrinter("ut-fake-printer-pkg-t24"));
}

TEST_F(DriverManagerExtra_UT, UninstallPrinter_SearchUnavailable_ReturnsFalse)
{
    // 真实网络请求（内网搜索接口不可达或无数据）→ 数据为空返回 false
    DriverManager mgr;
    EXPECT_FALSE(mgr.uninstallPrinter("UT-Vendor-T24", "UT-Model-T24"));
}

// ===== isSigned（真实校验器，未签名内容） =====

TEST_F(DriverManagerExtra_UT, IsSigned_DePath_UnsignedDeb_ReturnsFalse)
{
    // 构造最小合法 deb（dpkg-deb --build）：无签名 → deepin-deb-verify 校验失败 → false
    const QString stage = QDir::tempPath() + "/ut_t24_signstage";
    QDir().mkpath(stage + "/DEBIAN");
    {
        QFile ctl(stage + "/DEBIAN/control");
        ASSERT_TRUE(ctl.open(QIODevice::WriteOnly | QIODevice::Truncate));
        ctl.write("Package: ut-t24-signpkg\nVersion: 1.0\nArchitecture: all\nMaintainer: UT\nDescription: t24 fixture\n");
    }
    const QString out = QDir::tempPath() + "/ut_t24_signpkg.deb";
    QFile::remove(out);
    QProcess dpkg;
    dpkg.start("dpkg-deb", QStringList() << "--build" << stage << out);
    ASSERT_TRUE(dpkg.waitForFinished(20000));
    ASSERT_TRUE(QFile::exists(out));

    DriverManager mgr;
    bool result = true;
    {
        Stub stub;
        stub.set(ADDR(Utils, hierarchicalEnabled), ut_stubHierDisabled);
        stub.set(ADDR(QObject, property), ut_stubObjPropertyFalse);
        result = mgr.isSigned(out);
    }
    EXPECT_FALSE(result);
    QFile::remove(out);
    QDir(stage).removeRecursively();
}

TEST_F(DriverManagerExtra_UT, IsSigned_KoPath_UnsignedKo_ReturnsFalse)
{
    // 真实内核模块（未签名）→ deepin-elf-sign 校验失败 → false
    const QString koPath = QStringLiteral("/lib/modules/%1/kernel/block/bfq.ko").arg(Utils::kernelRelease());
    if (!QFile::exists(koPath)) {
        GTEST_SKIP() << "bfq.ko not available on this machine";
    }
    Stub stub;
    stub.set(ADDR(Utils, hierarchicalEnabled), ut_stubHierDisabled);
    DriverManager mgr;
    bool result = true;
    {
        Stub stub;
        stub.set(ADDR(Utils, hierarchicalEnabled), ut_stubHierDisabled);
        stub.set(ADDR(QObject, property), ut_stubObjPropertyFalse);
        result = mgr.isSigned(koPath);
    }
    EXPECT_FALSE(result);
}

// ===== installDriverFd 正向内容（真实 .ko 桥接，走 .ko 安装分支至 isSigned 失败） =====

TEST_F(DriverManagerExtra_UT, InstallDriverFd_RealKoContent_SignCheckFails)
{
    const QString koPath = QStringLiteral("/lib/modules/%1/kernel/block/bfq.ko").arg(Utils::kernelRelease());
    if (!QFile::exists(koPath)) {
        GTEST_SKIP() << "bfq.ko not available on this machine";
    }
    QFile ko(koPath);
    ASSERT_TRUE(ko.open(QIODevice::ReadOnly));
    QDBusUnixFileDescriptor fd(ko.handle());
    ASSERT_TRUE(fd.isValid());

    DriverManager mgr;
    QSignalSpy finishedSpy(&mgr, &DriverManager::sigFinished);
    ASSERT_TRUE(finishedSpy.isValid());

    // 内容为真实 .ko → isDriverPackage=true → isSigned 失败 → E_NOT_SIGNED(102)
    bool result = true;
    {
        Stub stub;
        stub.set(ADDR(Utils, hierarchicalEnabled), ut_stubHierDisabled);
        stub.set(ADDR(QObject, property), ut_stubObjPropertyFalse);
        result = mgr.installDriverFd(fd, "check.ko");
    }
    EXPECT_FALSE(result);
    ASSERT_GE(finishedSpy.count(), 1);
    EXPECT_FALSE(finishedSpy.at(finishedSpy.count() - 1).at(0).toBool());
    EXPECT_EQ(finishedSpy.at(finishedSpy.count() - 1).at(1).toString(), QStringLiteral("102"));
}
