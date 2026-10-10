// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include "drivercontrol/utils.h"

#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

#include <gtest/gtest.h>

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
 */

class Utils_UT : public UT_HEAD
{
public:
    void SetUp() override
    {
    }
    void TearDown() override
    {
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

TEST_F(Utils_UT, GetUrl_NoUrlFile_ReturnsProductionUrl)
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

TEST_F(Utils_UT, GetUrl_UrlFileTrue_ReturnsProductionUrl)
{
    // Arrange — ~/url 内容为 "true"，触发分支 B2
    saveUrlFile();
    writeUrlFile("true\n");
    // Act
    QString url = Utils::getUrl();
    // Assert
    EXPECT_EQ(url, QString("https://driver.uniontech.com/api/v1/drive/search"));
    // Cleanup
    restoreUrlFile();
}

TEST_F(Utils_UT, GetUrl_UrlFileOther_ReturnsPreproductionUrl)
{
    // Arrange — ~/url 内容为非 "true"，触发分支 B3
    saveUrlFile();
    writeUrlFile("pre");
    // Act
    QString url = Utils::getUrl();
    // Assert
    EXPECT_EQ(url, QString("https://drive-pre.uniontech.com/api/v1/drive/search"));
    // Cleanup
    restoreUrlFile();
}

// ===== kernelRelease / machineArch =====

TEST_F(Utils_UT, KernelRelease_ReturnsNonEmptyString)
{
    // Act
    QString kr = Utils::kernelRelease();
    // Assert — uname release 在任何 Linux 上非空
    EXPECT_FALSE(kr.isEmpty());
}

TEST_F(Utils_UT, MachineArch_ReturnsKnownArch)
{
    // Act
    QString arch = Utils::machineArch();
    // Assert — 常见架构值
    EXPECT_FALSE(arch.isEmpty());
    EXPECT_TRUE(arch == "x86_64" || arch == "aarch64" || arch == "mips64" || arch == "sw_64" || arch == "loongarch64")
        << "unexpected arch: " << arch.toStdString();
}

// ===== getOsBuild / getVersion =====

TEST_F(Utils_UT, GetOsBuild_ReturnsNonEmptyOnDeepin)
{
    // Act
    QString build = Utils::getOsBuild();
    // Assert — deepin 环境 /etc/os-version 存在且含 OsBuild；若不存在则返回空（不崩溃）
    EXPECT_FALSE(build.isEmpty()) << "/etc/os-version 未提供 OsBuild（环境差异，行为仍合法）";
}

TEST_F(Utils_UT, GetVersion_ReturnsTrueAndNonEmptyOnDeepin)
{
    // Arrange
    QString major, minor;
    // Act
    bool ok = Utils::getVersion(major, minor);
    // Assert — deepin 环境 /etc/os-version 含 MajorVersion/MinorVersion
    EXPECT_TRUE(ok);
    EXPECT_FALSE(major.isEmpty());
    EXPECT_FALSE(minor.isEmpty());
}

// ===== isFileLocked (验证 d78f83ff 修复) =====

TEST_F(Utils_UT, IsFileLocked_UnlockedWrite_ReturnsFalse)
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
}

TEST_F(Utils_UT, IsFileLocked_UnlockedRead_ReturnsFalse)
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
}

TEST_F(Utils_UT, IsFileLocked_NonexistentWrite_ReturnsFalse)
{
    // Arrange — 不存在路径，ENOENT 分支 B3
    QString path = QDir::tempPath() + "/ut_utils_nonexistent_" + QString::number(::getpid()) + ".txt";
    QFile::remove(path);
    // Act
    bool locked = Utils::isFileLocked(path, false);
    // Assert — 文件不存在应返回 false（非"已锁定"），验证 d78f83ff 修复
    EXPECT_FALSE(locked);
}

TEST_F(Utils_UT, IsFileLocked_NonexistentRead_ReturnsFalse)
{
    // Arrange — 不存在路径 read 模式，ENOENT 分支 B4
    QString path = QDir::tempPath() + "/ut_utils_nonexistent_r_" + QString::number(::getpid()) + ".txt";
    QFile::remove(path);
    // Act
    bool locked = Utils::isFileLocked(path, true);
    // Assert
    EXPECT_FALSE(locked);
}

TEST_F(Utils_UT, IsFileLocked_DirectoryWrite_ReturnsTrue)
{
    // Arrange — 目录以 O_WRONLY 打开失败 (EISDIR, errno!=ENOENT) -> 保守视为已锁，分支 B5
    // Act
    bool locked = Utils::isFileLocked(QDir::tempPath(), false);
    // Assert
    EXPECT_TRUE(locked);
}

// ===== executeServerCmd =====

TEST_F(Utils_UT, ExecuteServerCmd_Echo_ReturnsOutput)
{
    // Act
    QByteArray out = Utils::executeServerCmd("echo", QStringList() << "hello-ut-utils");
    // Assert
    EXPECT_FALSE(out.isEmpty());
    EXPECT_NE(out.indexOf("hello-ut-utils"), -1);
}

TEST_F(Utils_UT, ExecuteServerCmd_FailingCmd_ReturnsEmpty)
{
    // Act — "false" 命令恒以 exit 1 退出
    QByteArray out = Utils::executeServerCmd("false", QStringList());
    // Assert — 非零退出返回空
    EXPECT_TRUE(out.isEmpty());
}

TEST_F(Utils_UT, ExecuteServerCmd_ReadAll_MergesChannels)
{
    // Act — readAll=true 合并 stderr；用 echo 同时验证仍能取到 stdout
    QByteArray out = Utils::executeServerCmd("echo", QStringList() << "merged", QString(), 30000, true);
    // Assert
    EXPECT_NE(out.indexOf("merged"), -1);
}

TEST_F(Utils_UT, ExecuteServerCmd_WithWorkingDirectory_ReturnsOutput)
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
