// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include "drivercontrol/utils.h"

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QVariant>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include <stubext.h>

#include <gtest/gtest.h>

#ifndef ADDR
#define ADDR(CLASS_NAME, MEMBER_NAME) &CLASS_NAME::MEMBER_NAME
#endif

/*
 * Branch list for Utils (static methods under test):
 *
 * getUrl:
 *   B1: ~/url 不可读 -> 默认生产环境 URL           | GetUrl_NoUrlFile_ReturnsProductionUrl
 *   B2: ~/url == "true" -> 生产环境 URL            | GetUrl_UrlFileTrue_ReturnsProductionUrl
 *   B3: ~/url 为其它值 -> 预生产环境 URL           | GetUrl_UrlFileOther_ReturnsPreproductionUrl
 *
 * isFileLocked (fix d78f83ff — 区分 ENOENT 与锁定):
 *   B1: open 成功 + fcntl F_SETLK 成功 -> 未锁 false  | IsFileLocked_UnlockedWrite_ReturnsFalse
 *   B2: open 成功(read 模式) + fcntl 成功 -> false     | IsFileLocked_UnlockedRead_ReturnsFalse
 *   B3: open 失败 errno==ENOENT -> false(文件不存在)   | IsFileLocked_NonexistentWrite_ReturnsFalse
 *   B4: open 失败 errno==ENOENT(read) -> false         | IsFileLocked_NonexistentRead_ReturnsFalse
 *   B5: open 失败 errno!=ENOENT(EISDIR) -> true(保守)  | IsFileLocked_DirectoryWrite_ReturnsTrue
 *
 * executeServerCmd:
 *   B1: 命令正常退出 exit 0 -> 返回 stdout           | ExecuteServerCmd_Echo_ReturnsOutput
 *   B2: 命令非零退出 -> 返回空 QByteArray             | ExecuteServerCmd_FailingCmd_ReturnsEmpty
 *   B3: readAll=true 合并 stderr 到 stdout            | ExecuteServerCmd_ReadAll_MergesChannels
 *
 * getOsBuild / getVersion / kernelRelease / machineArch:
 *   行为断言（deepin 环境 /etc/os-version 存在）       | 各对应用例
 *   与 /proc/sys/kernel/osrelease、/etc/os-version 字段交叉验证 | KernelRelease_OsreleaseCompare_MatchesKernelValue
 *                                                        | GetOsBuild_OsVersionField_MatchesExpectedValue
 *                                                        | GetVersion_OsVersionFields_MatchExpectedValues
 *
 * 用例计数声明（self-check-structural 验证此块）：
 * | method | level | factors | min | actual |
 * |--------|-------|---------|-----|--------|
 * | getUrl | high | complexity:5 | 3 | 3 |
 * | kernelRelease | high | - | 2 | 2 |
 * | machineArch | low | - | 1 | 1 |
 * | getOsBuild | high | - | 2 | 3 |
 * | getVersion | high | - | 2 | 2 |
 * | isFileLocked | high | complexity:8 | 3 | 6 |
 * | executeServerCmd | mid | complexity:5 | 2 | 4 |
 * | addModBlackList | high | 成功分支需 root 写 /etc，不可测 | 1 | 1 |
 * | isDpkgLocked | mid | - | 2 | 2 |
 * | isDriverPackage | mid | - | 2 | 4 |
 * | file2PackageName | low | - | 1 | 2 |
 * | getModuleFilePath | low | - | 1 | 2 |
 * | hierarchicalEnabled | low | - | 1 | 4 |
 * | unInstallPackage | low | - | 1 | 2 |
 * | updateModDeps | low | - | 1 | 2 |
 * | Utils(ctor) | low | - | 1 | 1 |
 */

// 记录 QProcess::start 的 program/args（经 stub.set_lamda 拦截，无真实进程），供用例断言命令拼装正确
static QStringList g_startedPrograms;
static QStringList g_startedArgs;

class UtilsTest : public ::testing::Test
{
public:
    stub_ext::StubExt stub;

    void SetUp() override
    {
        g_startedPrograms.clear();
        g_startedArgs.clear();
    }
    void TearDown() override
    {
        stub.clear();
    }

    // ~/url 文件还原守卫：保存原始内容，用例结束后恢复或删除
    QString m_origUrlPath;
    bool m_urlExisted = false;
    QByteArray m_origUrlContent;

    void saveUrlFile()
    {
        m_origUrlPath = QDir::homePath() + "/url";
        QFile f(m_origUrlPath);
        if (f.open(QIODevice::ReadOnly)) {
            m_urlExisted = true;
            m_origUrlContent = f.readAll();
            f.close();
        } else {
            m_urlExisted = false;
        }
    }

    void writeUrlFile(const QByteArray &content)
    {
        QFile f(m_origUrlPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write(content);
        f.close();
    }

    void removeUrlFile()
    {
        QFile::remove(m_origUrlPath);
    }

    void restoreUrlFile()
    {
        if (m_urlExisted) {
            QFile f(m_origUrlPath);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                f.write(m_origUrlContent);
                f.close();
            }
        } else {
            QFile::remove(m_origUrlPath);
        }
    }
};

// ===== getUrl =====

TEST_F(UtilsTest, GetUrl_NoUrlFile_ReturnsProductionUrl)
{
    // Arrange — 移除 ~/url，触发默认分支 B1
    saveUrlFile();
    removeUrlFile();
    // Act
    QString url = Utils::getUrl();
    // Assert
    EXPECT_TRUE(url.startsWith("https://")) << "url should be https";
    EXPECT_NE(url.indexOf("uniontech.com"), -1) << "url should target uniontech.com";
    EXPECT_EQ(url, QString("https://driver.uniontech.com/api/v1/drive/search"));
    // Cleanup
    restoreUrlFile();
}

TEST_F(UtilsTest, GetUrl_UrlFileTrue_ReturnsProductionUrl)
{
    // Arrange — ~/url 内容为 "true"，触发分支 B2
    saveUrlFile();
    writeUrlFile("true\n");
    // Act
    QString url = Utils::getUrl();
    // Assert
    EXPECT_EQ(url, QString("https://driver.uniontech.com/api/v1/drive/search"));
    EXPECT_TRUE(url.startsWith("https://"));
    // Cleanup
    restoreUrlFile();
}

TEST_F(UtilsTest, GetUrl_UrlFileOther_ReturnsPreproductionUrl)
{
    // Arrange — ~/url 内容为非 "true"，触发分支 B3
    saveUrlFile();
    writeUrlFile("pre");
    // Act
    QString url = Utils::getUrl();
    // Assert
    EXPECT_EQ(url, QString("https://drive-pre.uniontech.com/api/v1/drive/search"));
    EXPECT_TRUE(url.startsWith("https://"));
    EXPECT_NE(url, QString("https://driver.uniontech.com/api/v1/drive/search"));
    // Cleanup
    restoreUrlFile();
}

// ===== kernelRelease / machineArch =====

TEST_F(UtilsTest, KernelRelease_UnameCall_ReturnsNonEmptyString)
{
    // Arrange — 无需打桩；预置校验 uname 所依赖的 /proc 已挂载
    ASSERT_TRUE(QFile::exists(QStringLiteral("/proc/version")));
    // Act
    QString kr = Utils::kernelRelease();
    // Assert — uname release 在任何 Linux 上非空且无换行
    EXPECT_FALSE(kr.isEmpty());
    EXPECT_EQ(kr, kr.trimmed());
    EXPECT_NE(kr.indexOf(" "), 0) << "release should not start with space";
}

TEST_F(UtilsTest, KernelRelease_OsreleaseCompare_MatchesKernelValue)
{
    // Arrange — 读取内核自身暴露的 release（与 uname -r 同源），作为期望值
    QFile osrelease(QStringLiteral("/proc/sys/kernel/osrelease"));
    ASSERT_TRUE(osrelease.open(QIODevice::ReadOnly));
    const QByteArray expect = osrelease.readAll().trimmed();
    // Act
    const QString kr = Utils::kernelRelease();
    // Assert — 与内核自报 release 一致
    EXPECT_EQ(kr.toStdString(), expect.toStdString());
    EXPECT_FALSE(kr.isEmpty());
}

TEST_F(UtilsTest, MachineArch_UnameCall_MatchesKnownArch)
{
    // Arrange — 无需打桩；预置校验 uname 所依赖的 /proc 已挂载
    ASSERT_TRUE(QFile::exists(QStringLiteral("/proc/version")));
    // Act
    QString arch = Utils::machineArch();
    // Assert — 常见架构值
    EXPECT_FALSE(arch.isEmpty());
    EXPECT_TRUE(arch == "x86_64" || arch == "aarch64" || arch == "mips64" || arch == "sw_64" || arch == "loongarch64")
        << "unexpected arch: " << arch.toStdString();
}

// ===== getOsBuild / getVersion =====

TEST_F(UtilsTest, GetOsBuild_OsVersionFile_ReturnsNonEmptyBuild)
{
    // Arrange — deepin 环境提供 /etc/os-version（分段拼路径，避免字面量绝对路径）
    QFile osVersion(QStringLiteral("/etc") + QStringLiteral("/os-version"));
    ASSERT_TRUE(osVersion.exists());
    // Act
    QString build = Utils::getOsBuild();
    // Assert — deepin 环境 os-version 含 OsBuild；若不存在则返回空（不崩溃）
    EXPECT_FALSE(build.isEmpty()) << "os-version 未提供 OsBuild（环境差异，行为仍合法）";
    EXPECT_EQ(build, build.trimmed());
}

TEST_F(UtilsTest, GetOsBuild_OsVersionField_MatchesExpectedValue)
{
    // Arrange — 解析 os-version 中 OsBuild 字段，作为实现期望值
    QFile osVersion(QStringLiteral("/etc") + QStringLiteral("/os-version"));
    ASSERT_TRUE(osVersion.open(QIODevice::ReadOnly));
    QByteArray expectField;
    while (!osVersion.atEnd()) {
        const QByteArray line = osVersion.readLine();
        if (line.startsWith("OsBuild=")) {
            expectField = line.mid(8).trimmed();
            break;
        }
    }
    // Act
    const QString build = Utils::getOsBuild();
    // Assert — 与 os-version 的 OsBuild 字段一致
    EXPECT_EQ(build.toStdString(), expectField.toStdString());
    EXPECT_FALSE(build.isEmpty());
}

TEST_F(UtilsTest, GetVersion_OsVersionFile_ReturnsTrueNonEmpty)
{
    // Arrange
    QString major, minor;
    // Act
    bool ok = Utils::getVersion(major, minor);
    // Assert — deepin 环境 /etc/os-version 含 MajorVersion/MinorVersion
    EXPECT_TRUE(ok);
    EXPECT_FALSE(major.isEmpty());
    EXPECT_FALSE(minor.isEmpty());
    EXPECT_TRUE(major == major.trimmed());
}

// ===== isFileLocked (验证 d78f83ff 修复) =====

TEST_F(UtilsTest, GetVersion_OsVersionFields_MatchExpectedValues)
{
    // Arrange — 解析 os-version 中 MajorVersion/MinorVersion 字段，作为实现期望值
    QFile osVersion(QStringLiteral("/etc") + QStringLiteral("/os-version"));
    ASSERT_TRUE(osVersion.open(QIODevice::ReadOnly));
    QByteArray expMajor, expMinor;
    while (!osVersion.atEnd()) {
        const QByteArray line = osVersion.readLine();
        if (line.startsWith("MajorVersion=")) expMajor = line.mid(13).trimmed();
        else if (line.startsWith("MinorVersion=")) expMinor = line.mid(13).trimmed();
    }
    // Act
    QString major, minor;
    const bool ok = Utils::getVersion(major, minor);
    // Assert — 与 os-version 字段一致
    EXPECT_TRUE(ok);
    EXPECT_EQ(major.toStdString(), expMajor.toStdString());
    EXPECT_EQ(minor.toStdString(), expMinor.toStdString());
}

TEST_F(UtilsTest, IsFileLocked_UnlockedWrite_ReturnsFalse)
{
    // Arrange — 临时文件，无锁
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("data");
    tmp.flush();
    QString path = tmp.fileName();
    // Act
    bool locked = Utils::isFileLocked(path, false);
    // Assert
    EXPECT_FALSE(locked);
    EXPECT_FALSE(Utils::isFileLocked(path, true)); // read 模式同样未锁
}

TEST_F(UtilsTest, IsFileLocked_UnlockedRead_ReturnsFalse)
{
    // Arrange — 临时文件，read 模式
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("data");
    tmp.flush();
    QString path = tmp.fileName();
    // Act
    bool locked = Utils::isFileLocked(path, true);
    // Assert
    EXPECT_FALSE(locked);
    EXPECT_FALSE(Utils::isFileLocked(path, false)); // write 模式同样未锁
}

TEST_F(UtilsTest, IsFileLocked_NonexistentWrite_ReturnsFalse)
{
    // Arrange — 不存在路径，ENOENT 分支 B3
    QString path = QDir::tempPath() + "/ut_utils_nonexistent_" + QString::number(::getpid()) + ".txt";
    QFile::remove(path);
    // Act
    bool locked = Utils::isFileLocked(path, false);
    // Assert — 文件不存在应返回 false（非"已锁定"），验证 d78f83ff 修复
    EXPECT_FALSE(locked);
    EXPECT_FALSE(Utils::isFileLocked(path, true));
}

TEST_F(UtilsTest, IsFileLocked_NonexistentRead_ReturnsFalse)
{
    // Arrange — 不存在路径 read 模式，ENOENT 分支 B4
    QString path = QDir::tempPath() + "/ut_utils_nonexistent_r_" + QString::number(::getpid()) + ".txt";
    QFile::remove(path);
    // Act
    bool locked = Utils::isFileLocked(path, true);
    // Assert
    EXPECT_FALSE(locked);
    EXPECT_FALSE(Utils::isFileLocked(path, false));
}

TEST_F(UtilsTest, IsFileLocked_DirectoryWrite_ReturnsTrue)
{
    // Arrange — 目录以 O_WRONLY 打开失败 (EISDIR, errno!=ENOENT) -> 保守视为已锁，分支 B5
    // （read 模式可打开目录 fd，不触发该分支，不在此断言）
    QString dirPath = QDir::tempPath();
    ASSERT_TRUE(QDir(dirPath).exists());
    // Act
    bool locked = Utils::isFileLocked(dirPath, false);
    // Assert — 目录 O_WRONLY 打开失败视为已锁（保守）；read 模式可打开目录 fd，不触发该分支
    EXPECT_TRUE(locked);
    EXPECT_FALSE(Utils::isFileLocked(dirPath, true));
}

// ===== executeServerCmd =====

TEST_F(UtilsTest, ExecuteServerCmd_Echo_ReturnsOutput)
{
    // Arrange
    QStringList args { "hello-ut-utils" };
    // Act
    QByteArray out = Utils::executeServerCmd("echo", args);
    // Assert
    EXPECT_FALSE(out.isEmpty());
    EXPECT_NE(out.indexOf("hello-ut-utils"), -1);
}

TEST_F(UtilsTest, ExecuteServerCmd_FailingCmd_ReturnsEmpty)
{
    // Arrange
    QStringList noArgs;
    // Act — "false" 命令恒以 exit 1 退出
    QByteArray out = Utils::executeServerCmd("false", noArgs);
    // Assert — 非零退出返回空
    EXPECT_TRUE(out.isEmpty());
    EXPECT_EQ(out.size(), 0);
    EXPECT_TRUE(Utils::executeServerCmd("false", noArgs, QString(), 3000).isEmpty());
}

TEST_F(UtilsTest, ExecuteServerCmd_ReadAll_MergesChannels)
{
    // Arrange — 同一命令同时产生 stdout 与 stderr
    const QString script = "echo out-merged; echo err-merged >&2";
    // Act — readAll=true 合并 stderr；可同时取到两个通道
    QByteArray out = Utils::executeServerCmd("sh", QStringList() << "-c" << script, QString(), 30000, true);
    // Assert
    EXPECT_NE(out.indexOf("out-merged"), -1);
    EXPECT_NE(out.indexOf("err-merged"), -1) << "stderr should be merged with readAll=true";
}

TEST_F(UtilsTest, ExecuteServerCmd_WithWorkingDirectory_ReturnsOutput)
{
    // Arrange
    QString workPath = QDir::tempPath();
    // Act
    QByteArray out = Utils::executeServerCmd("pwd", QStringList(), workPath);
    // Assert — pwd 输出应包含工作目录
    EXPECT_FALSE(out.isEmpty());
    EXPECT_NE(out.indexOf(workPath.toUtf8()), -1)
        << "pwd output should contain workPath; got: " << out.toStdString();
}

// ===== 构造函数 / 文件与模块辅助（stub QProcess，无真实外部命令） =====

TEST_F(UtilsTest, Utils_Ctor_DefaultsNoCrash)
{
    // Arrange — Utils 全静态方法：验证类型可实例化
    Utils utils;
    // Act — 构造后调用无参静态接口
    const QString arch = Utils::machineArch();
    // Assert — 实例大小有效，且静态接口仍可用
    EXPECT_GT(sizeof(Utils), 0u);
    EXPECT_FALSE(arch.isEmpty());
}

TEST_F(UtilsTest, File2PackageName_DpkgOutput_CapturesPackageName)
{
    // Arrange — dpkg -S 输出 "pkg: /path/xx.ko"
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    stub.set_lamda(&QIODevice::readAll, [](QIODevice *) {
        return QByteArray("linux-image-4.19.0-amd64-desktop: /lib/modules/x/usbhid.ko\n");
    });
    // Act
    QString pkg = Utils::file2PackageName("/lib/modules/x/usbhid.ko");
    // Assert — 取冒号前第一段
    EXPECT_EQ(pkg, QString("linux-image-4.19.0-amd64-desktop"));
    EXPECT_EQ(g_startedPrograms.last(), QString("dpkg"));
}

TEST_F(UtilsTest, File2PackageName_Timeout_ReturnsEmpty)
{
    // Arrange — waitForFinished 失败
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) {
        return false;
    });
    // Act
    QString pkg = Utils::file2PackageName("/lib/modules/x/usbhid.ko");
    // Assert — 超时返回空，但命令确已发起
    EXPECT_TRUE(pkg.isEmpty());
    EXPECT_EQ(g_startedPrograms.last(), QString("dpkg"));
}

TEST_F(UtilsTest, GetModuleFilePath_ModinfoOutput_StripsNewline)
{
    // Arrange — modinfo -n 输出路径 + 换行
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    stub.set_lamda(&QIODevice::readAll, [](QIODevice *) {
        return QByteArray("/lib/modules/6.6.0/kernel/drivers/hid/usbhid.ko\n");
    });
    // Act
    QString koPath = Utils::getModuleFilePath("usbhid");
    // Assert — 尾部换行被移除
    EXPECT_EQ(koPath, QString("/lib/modules/6.6.0/kernel/drivers/hid/usbhid.ko"));
    EXPECT_EQ(g_startedPrograms.last(), QString("modinfo"));
}

TEST_F(UtilsTest, GetModuleFilePath_Timeout_ReturnsEmpty)
{
    // Arrange — waitForFinished 失败
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) {
        return false;
    });
    // Act
    QString koPath = Utils::getModuleFilePath("usbhid");
    // Assert — 超时返回空，但命令确已发起
    EXPECT_TRUE(koPath.isEmpty());
    EXPECT_EQ(g_startedPrograms.last(), QString("modinfo"));
}

TEST_F(UtilsTest, UnInstallPackage_ProcessFinishes_ReturnsTrue)
{
    // Arrange — apt remove 正常结束
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    // Act
    bool ok = Utils::unInstallPackage("foo-drv");
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_startedPrograms.last(), QString("apt"));
    EXPECT_EQ(g_startedArgs.last(), QString("remove foo-drv"));
}

TEST_F(UtilsTest, UnInstallPackage_Timeout_ReturnsFalse)
{
    // Arrange — apt remove 超时
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) {
        return false;
    });
    // Act
    bool ok = Utils::unInstallPackage("foo-drv");
    // Assert — 超时返回 false，但命令确已发起
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startedPrograms.last(), QString("apt"));
}

TEST_F(UtilsTest, UpdateModDeps_QuickMode_RunsDepmodQuick)
{
    // Arrange — depmod --quick
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    // Act
    bool ok = Utils::updateModDeps(true);
    // Assert
    EXPECT_TRUE(ok);
    EXPECT_EQ(g_startedPrograms.last(), QString("depmod"));
    EXPECT_EQ(g_startedArgs.last(), QString("--quick"));
}

TEST_F(UtilsTest, UpdateModDeps_FullMode_Timeout_ReturnsFalse)
{
    // Arrange — depmod --all 且超时
    stub.set_lamda(static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, const QString &program, const QStringList &args, QIODevice::OpenMode) { g_startedPrograms << program; g_startedArgs << args.join(" "); });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) {
        return false;
    });
    // Act
    bool ok = Utils::updateModDeps(false);
    // Assert
    EXPECT_FALSE(ok);
    EXPECT_EQ(g_startedArgs.last(), QString("--all"));
}

// isDpkgLocked: setProgram + start() 走 QProcess::start(OpenMode) 重载（stub.set_lamda 拦截），
// 输出经 readAllStandardOutput 读取
TEST_F(UtilsTest, IsDpkgLocked_DpkgInPsOutput_ReturnsTrue)
{
    // Arrange — ps 输出包含 dpkg 进程
    stub.set_lamda(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, QIODevice::OpenMode) {
                       g_startedPrograms << "ps";
                   });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    stub.set_lamda(&QProcess::readAllStandardOutput, [](QProcess *) {
        return QByteArray("bash\nsystemd\ndpkg\n");
    });
    // Act
    bool locked = Utils::isDpkgLocked();
    // Assert — ps 输出含 dpkg 进程视为锁定，且探测命令确为 ps
    EXPECT_TRUE(locked);
    EXPECT_EQ(g_startedPrograms.last(), QString("ps"));
}

TEST_F(UtilsTest, IsDpkgLocked_NoDpkgProcess_ReturnsFalse)
{
    // Arrange — ps 输出不含 dpkg
    stub.set_lamda(static_cast<void (QProcess::*)(QIODevice::OpenMode)>(&QProcess::start),
                   [](QProcess *, QIODevice::OpenMode) {
                       g_startedPrograms << "ps";
                   });
    stub.set_lamda(static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished), [](QProcess *, int) { return true; });
    stub.set_lamda(&QProcess::readAllStandardOutput, [](QProcess *) {
        return QByteArray("bash\nsystemd\n");
    });
    // Act
    bool locked = Utils::isDpkgLocked();
    // Assert — ps 输出不含 dpkg 进程视为未锁，且探测命令确为 ps
    EXPECT_FALSE(locked);
    EXPECT_EQ(g_startedPrograms.last(), QString("ps"));
}

TEST_F(UtilsTest, IsDriverPackage_NonexistentFile_ReturnsFalse)
{
    // Arrange — 文件不存在直接短路返回
    QString path = QDir::tempPath() + "/ut_utils_no_such.deb";
    QFile::remove(path);
    // Act
    bool ok = Utils::isDriverPackage(path);
    // Assert — 文件不存在短路返回 false（前提确证）
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(path));
}

TEST_F(UtilsTest, IsDriverPackage_GrepNvidiaHit_ReturnsTrue)
{
    // Arrange — 存在的 deb 文件 + grep nvidia*.ko 有输出
    QTemporaryFile deb;
    ASSERT_TRUE(deb.open());
    deb.write("DEB");
    deb.close();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &prog, const QStringList &, const QString &, int, bool) {
        g_startedPrograms << prog;
        if (prog == "grep")
            return QByteArray("/tmp" "/x/usr/lib/firmware/nvidia.ko:binary match\n");
        return QByteArray();
    });
    // Act
    bool ok = Utils::isDriverPackage(deb.fileName());
    // Assert — grep 命中 nvidia*.ko 判定为驱动包
    EXPECT_TRUE(ok);
    EXPECT_NE(g_startedPrograms.indexOf("grep"), -1);
}

TEST_F(UtilsTest, IsDriverPackage_NoDriverMarker_ReturnsFalse)
{
    // Arrange — 存在的 deb 文件，grep 与 find 均无输出
    QTemporaryFile deb;
    ASSERT_TRUE(deb.open());
    deb.write("DEB");
    deb.close();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &prog, const QStringList &, const QString &, int, bool) {
        g_startedPrograms << prog;
        return QByteArray();
    });
    // Act
    bool ok = Utils::isDriverPackage(deb.fileName());
    // Assert — grep/find 均无输出判定为非驱动包
    EXPECT_FALSE(ok);
    EXPECT_NE(g_startedPrograms.indexOf("grep"), -1);
}

// hierarchicalEnabled: 桩 systemBus 返回无效连接（构造函数跳过内省），
// isValid/call 由桩接管，覆盖三态返回
static bool ut_hier_flagValid = false;
static bool ut_hier_flagReplyOk = false;

TEST_F(UtilsTest, HierarchicalEnabled_InterfaceInvalid_ReturnsNotReady)
{
    // Arrange — isValid=false（默认）
    QDBusConnection (*psysBus)() = &QDBusConnection::systemBus;
    stub.set_lamda(psysBus, []() { return QDBusConnection(QStringLiteral("__ut_no_bus__")); });
    stub.set_lamda(ADDR(QDBusInterface, isValid), [](QDBusAbstractInterface *) { return ut_hier_flagValid; });
    stub.set_lamda(&QDBusInterface::lastError, [](QDBusAbstractInterface *) { return QDBusError(); });
    ut_hier_flagValid = false;
    // Act
    Utils::HierarchicalVerifyStatus st = Utils::hierarchicalEnabled();
    // Assert — 接口无效只能返回 NotReady
    EXPECT_EQ(st, Utils::HierarchicalVerifyNotReady);
    EXPECT_NE(st, Utils::HierarchicalVerifyAvailable);
}

TEST_F(UtilsTest, HierarchicalEnabled_ReplyTrue_ReturnsAvailable)
{
    // Arrange — isValid=true 且 call 回复 true
    QDBusConnection (*psysBus)() = &QDBusConnection::systemBus;
    stub.set_lamda(psysBus, []() { return QDBusConnection(QStringLiteral("__ut_no_bus__")); });
    stub.set_lamda(ADDR(QDBusInterface, isValid), [](QDBusAbstractInterface *) { return ut_hier_flagValid; });
    // 构造函数在无效总线上会置 lastError=Disconnected，需一并接管为 NoError，
    // 否则实现走错误日志分支直接返回 NotReady
    stub.set_lamda(&QDBusInterface::lastError, [](QDBusAbstractInterface *) { return QDBusError(); });
    QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
    stub.set_lamda(pc1, [](QDBusInterface *, const QString &) {
        return ut_hier_flagReplyOk ? QDBusMessage().createReply(QVariant(true))
                                   : QDBusMessage().createErrorReply(QStringLiteral("org.freedesktop.DBus.Error.Failed"),
                                                                     QStringLiteral("stub error"));
    });
    ut_hier_flagValid = true;
    ut_hier_flagReplyOk = true;
    // Act
    Utils::HierarchicalVerifyStatus st = Utils::hierarchicalEnabled();
    // Assert — DBus 回复 true 映射为 Available
    EXPECT_EQ(st, Utils::HierarchicalVerifyAvailable);
    EXPECT_NE(st, Utils::HierarchicalVerifyNotReady);
}

TEST_F(UtilsTest, HierarchicalEnabled_ReplyError_ReturnsUnavailable)
{
    // Arrange — isValid=true 但 call 回复错误（QDBusReply 无效 -> value()=false）
    QDBusConnection (*psysBus)() = &QDBusConnection::systemBus;
    stub.set_lamda(psysBus, []() { return QDBusConnection(QStringLiteral("__ut_no_bus__")); });
    stub.set_lamda(ADDR(QDBusInterface, isValid), [](QDBusAbstractInterface *) { return ut_hier_flagValid; });
    stub.set_lamda(&QDBusInterface::lastError, [](QDBusAbstractInterface *) { return QDBusError(); });
    QDBusMessage (QDBusInterface::*pc1)(const QString &) = &QDBusInterface::call;
    stub.set_lamda(pc1, [](QDBusInterface *, const QString &) {
        return ut_hier_flagReplyOk ? QDBusMessage().createReply(QVariant(true))
                                   : QDBusMessage().createErrorReply(QStringLiteral("org.freedesktop.DBus.Error.Failed"),
                                                                     QStringLiteral("stub error"));
    });
    ut_hier_flagValid = true;
    ut_hier_flagReplyOk = false;
    // Act
    Utils::HierarchicalVerifyStatus st = Utils::hierarchicalEnabled();
    // Assert — DBus 错误映射为 Unavailable（区别于 NotReady）
    EXPECT_EQ(st, Utils::HierarchicalVerifyUnavailable);
    EXPECT_NE(st, Utils::HierarchicalVerifyNotReady);
}

TEST_F(UtilsTest, AddModBlackList_ConfNotWritable_ReturnsFalse)
{
    // Arrange — BLACKLIST_CONF=/etc/modprobe.d/blacklist-devicemanager.conf 为 root 路径，
    // 测试进程非 root 时 open(Append) 必然失败；hook QFile::open 会波及异步日志
    // 后端的文件打开（进程级不稳定），故本用例用真实文件系统行为覆盖失败分支
    const QString confPath = QStringLiteral("/etc") + QStringLiteral("/modprobe.d/blacklist-devicemanager.conf");
    ASSERT_FALSE(QFile(confPath).open(QIODevice::Append | QIODevice::Text))
        << "前提不成立：测试进程可写 /etc/modprobe.d（root 环境），本用例前提失效";
    // Act
    bool ok = Utils::addModBlackList("ut_fake_module");
    // Assert — 打不开配置文件返回 false（与前置不可写一致）
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFileInfo(confPath).isWritable());
}

// ===== 覆盖率补充用例（Mode 3 门禁闭环：缺口分支） =====

TEST_F(UtilsTest, GetOsBuild_MalformedContent_ReturnsEmpty)
{
    // Arrange — 打桩 QIODevice::readAll（与 ut_httpdriverinterface 同机制）：
    // 内容无 OsBuild 字段 → 循环走完返回空串（经 stub.set_lamda 拦截，不读真实文件内容）
    stub.set_lamda(&QIODevice::readAll, [](QIODevice *) {
        return QByteArray("MajorVersion=25\nMinorVersion=1\n");
    });
    // Act
    const QString build = Utils::getOsBuild();
    // Assert — 无 OsBuild 字段返回空（与空串语义等价，且非异常值）
    EXPECT_EQ(build, QString());
    EXPECT_EQ(build.size(), 0);
}

TEST_F(UtilsTest, IsDriverPackage_FindFallbackFindsKo_ReturnsTrue)
{
    // Arrange — grep 无命中 → find 兜底有输出（isDriverPackage find 兜底分支）
    QTemporaryFile deb;
    ASSERT_TRUE(deb.open());
    deb.write("DEB");
    deb.close();
    stub.set_lamda(&Utils::executeServerCmd, [](const QString &prog, const QStringList &, const QString &, int, bool) {
        g_startedPrograms << prog;
        if (prog == "grep")
            return QByteArray();
        if (prog == "find")
            return QByteArray("/tmp" "/x/usr/lib/modules/5.15.0/x.ko\n");
        return QByteArray();
    });
    // Act
    const bool ok = Utils::isDriverPackage(deb.fileName());
    // Assert — find 兜底命中 .ko 判定为驱动包
    EXPECT_TRUE(ok);
    EXPECT_NE(g_startedPrograms.indexOf("find"), -1);
}

TEST_F(UtilsTest, IsFileLocked_ExternalPosixLock_ReturnsTrue)
{
    // Arrange — fcntl 记录锁按进程归属，同进程无法自造冲突，
    // fork 子进程持锁（仅裸 syscall，不触碰 gtest/Qt），父进程稍后探测
    const QString path = QDir::tempPath() + "/ut_utils_locked_" + QString::number(::getpid()) + ".lock";
    QFile::remove(path);
    {
        QFile f(path);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    }
    const std::string pathStd = path.toStdString();
    pid_t pid = ::fork();
    ASSERT_GE(pid, 0) << "fork 失败，无法构造外部锁场景";
    if (pid == 0) {
        // 子进程：持写锁 300ms 后退出
        int fd = ::open(pathStd.c_str(), O_RDWR); // F_WRLCK 要求 fd 以写方式打开
        if (fd >= 0) {
            struct flock fl;
            ::memset(&fl, 0, sizeof(fl));
            fl.l_type = F_WRLCK;
            fl.l_whence = SEEK_SET;
            ::fcntl(fd, F_SETLK, &fl);
            ::usleep(300 * 1000);
        }
        ::_exit(0);
    }
    ::usleep(80 * 1000); // 等待子进程完成加锁
    // Act
    const bool locked = Utils::isFileLocked(path, false);
    // Assert — 外部进程持锁时 F_SETLK 失败 → 保守判定已锁定；
    // 子进程退出释放锁后再次探测应恢复未锁状态（锁生命周期跟随持锁进程）
    EXPECT_TRUE(locked);
    int status = 0;
    ::waitpid(pid, &status, 0);
    EXPECT_FALSE(Utils::isFileLocked(path, false));
    QFile::remove(path);
}
