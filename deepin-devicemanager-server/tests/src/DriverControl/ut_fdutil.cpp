// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include "drivercontrol/fdutil.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <fcntl.h>
#include <unistd.h>

using namespace FdUtil;

namespace {

// 进程唯一的工作目录，避免并发跑测试相互干扰
QString uniqueWorkDir()
{
    static QTemporaryDir dir(QDir::tempPath() + "/ut_fdutil_" + QString::number(::getpid()) + "_XXXXXX");
    return dir.path();
}

int openReadOnly(const QString &path, int extraFlags = 0)
{
    return ::open(QFile::encodeName(path).constData(), O_RDONLY | O_CLOEXEC | extraFlags);
}

} // namespace

class UT_FdUtil : public UT_HEAD
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(QDir().mkpath(uniqueWorkDir()));
    }
};

// ---------- isValidComponentName ----------

TEST_F(UT_FdUtil, UT_FdUtil_isValidComponentName_Valid)
{
    EXPECT_TRUE(isValidComponentName("nvidia-driver-535.100_all.deb"));
    EXPECT_TRUE(isValidComponentName("a.deb"));
    EXPECT_TRUE(isValidComponentName("pkg-1+2~3.deb"));
    EXPECT_TRUE(isValidComponentName("vendor_pkg.tar.xz"));
    // 注：apt epoch 冒号“:”不在白名单内 —— debname 门禁从严；
    // 含冒号的文件名场景由 sanitizedFileName 净化后处理
}

TEST_F(UT_FdUtil, UT_FdUtil_isValidComponentName_Invalid)
{
    EXPECT_FALSE(isValidComponentName(""));
    EXPECT_FALSE(isValidComponentName("."));
    EXPECT_FALSE(isValidComponentName(".."));
    EXPECT_FALSE(isValidComponentName(".hidden"));
    EXPECT_FALSE(isValidComponentName("a/b"));
    EXPECT_FALSE(isValidComponentName("a\\b"));
    EXPECT_FALSE(isValidComponentName("a b.deb"));
    EXPECT_FALSE(isValidComponentName("中文名.deb"));
    EXPECT_FALSE(isValidComponentName(QString(201, 'a')));
}

// ---------- sanitizedFileName ----------

TEST_F(UT_FdUtil, UT_FdUtil_sanitizedFileName)
{
    // 普通名字保持不变
    EXPECT_EQ(sanitizedFileName("nvidia-535.deb"), "nvidia-535.deb");
    // 路径分隔符：取基名
    EXPECT_EQ(sanitizedFileName("/tmp/xx/test.ko"), "test.ko");
    // 前导点剥除（防 ".."/隐藏文件语义）
    EXPECT_EQ(sanitizedFileName("..deb"), "deb");
    // 非白名单字符替换为 '_'（“中文名”三个字符），扩展名保留
    EXPECT_EQ(sanitizedFileName("中文名.deb"), "___" ".deb");
    EXPECT_EQ(sanitizedFileName("2:1.0-1_amd64.deb"), "2_1.0-1_amd64.deb");
    // 空串兜底
    EXPECT_FALSE(sanitizedFileName("").isEmpty());
    // 超长截断保留扩展名
    QString longName = QString(300, 'a') + ".deb";
    QString sanitized = sanitizedFileName(longName);
    EXPECT_TRUE(sanitized.size() <= 200);
    EXPECT_TRUE(sanitized.endsWith(".deb"));
    // 结果总能通过分量名校验
    EXPECT_TRUE(isValidComponentName(sanitized));
}

// ---------- copyFileByFd ----------

TEST_F(UT_FdUtil, UT_FdUtil_copyFileByFd_RegularFile)
{
    const QString src = uniqueWorkDir() + "/src.bin";
    QFile f(src);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    const QByteArray payload(1024 * 7, 'A');
    f.write(payload);
    f.close();

    int srcFd = openReadOnly(src);
    ASSERT_GE(srcFd, 0);
    const QString dest = uniqueWorkDir() + "/dest.bin";
    EXPECT_TRUE(copyFileByFd(srcFd, dest));
    ::close(srcFd);

    QFile destFile(dest);
    ASSERT_TRUE(destFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(destFile.readAll(), payload);
    // 权限 0600
    QFile::Permissions perms = QFile::permissions(dest);
    EXPECT_TRUE(perms & QFile::ReadOwner);
    EXPECT_FALSE(perms & QFile::ReadGroup);
    EXPECT_FALSE(perms & QFile::ReadOther);
}

TEST_F(UT_FdUtil, UT_FdUtil_copyFileByFd_RejectDirFd)
{
    int dirFd = openReadOnly(uniqueWorkDir(), O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_FALSE(copyFileByFd(dirFd, uniqueWorkDir() + "/should_not_exist"));
    ::close(dirFd);
    EXPECT_FALSE(QFile::exists(uniqueWorkDir() + "/should_not_exist"));
}

TEST_F(UT_FdUtil, UT_FdUtil_copyFileByFd_InvalidFd)
{
    EXPECT_FALSE(copyFileByFd(-1, uniqueWorkDir() + "/should_not_exist2"));
}

TEST_F(UT_FdUtil, UT_FdUtil_copyFileByFd_NoResidueOnFailure)
{
    // 目标已存在（O_EXCL 冲突）→ 失败且不破坏既有文件
    const QString dest = uniqueWorkDir() + "/existing.bin";
    {
        QFile f(dest);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("old");
    }
    const QString src = uniqueWorkDir() + "/src2.bin";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("new-content");
    }
    int srcFd = openReadOnly(src);
    ASSERT_GE(srcFd, 0);
    EXPECT_FALSE(copyFileByFd(srcFd, dest));
    ::close(srcFd);
    QFile f(dest);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), QByteArray("old"));
}

// ---------- copyDebsFromDirFd ----------

TEST_F(UT_FdUtil, UT_FdUtil_copyDebsFromDirFd_FiltersEntries)
{
    const QString srcDir = uniqueWorkDir() + "/staging";
    ASSERT_TRUE(QDir().mkpath(srcDir));
    {
        QFile f(srcDir + "/pkg_1.0-1_amd64.deb");
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("deb1");
    }
    {
        QFile f(srcDir + "/dep_2:1.0_amd64.deb"); // 冒号：apt epoch 场景
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("deb2");
    }
    {
        QFile f(srcDir + "/readme.txt");
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("txt");
    }
    // 符号链接 .deb：拒绝
    ASSERT_EQ(::symlink(QFile::encodeName(srcDir + "/pkg_1.0-1_amd64.deb").constData(),
                        QFile::encodeName(srcDir + "/evil_link.deb").constData()), 0);
    // 子目录 .deb：拒绝
    ASSERT_TRUE(QDir().mkpath(srcDir + "/subdir.deb"));

    const QString destDir = uniqueWorkDir() + "/backup_out";
    ASSERT_TRUE(QDir().mkpath(destDir));
    int dirFd = openReadOnly(srcDir, O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_TRUE(copyDebsFromDirFd(dirFd, destDir));
    ::close(dirFd);

    QDir out(destDir);
    EXPECT_EQ(out.entryList(QStringList() << "*.deb", QDir::Files).size(), 2);
    EXPECT_TRUE(QFile::exists(destDir + "/pkg_1.0-1_amd64.deb"));
    EXPECT_TRUE(QFile::exists(destDir + "/dep_2:1.0_amd64.deb"));
    EXPECT_FALSE(QFile::exists(destDir + "/readme.txt"));
    EXPECT_FALSE(QFile::exists(destDir + "/evil_link.deb"));
    EXPECT_FALSE(QFile::exists(destDir + "/subdir.deb"));
    EXPECT_TRUE(QFile::exists(srcDir + "/evil_link.deb")); // 源符号链接不受影响
}

TEST_F(UT_FdUtil, UT_FdUtil_copyDebsFromDirFd_RejectFileFd)
{
    const QString src = uniqueWorkDir() + "/plainfile.bin";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("x");
    }
    int fd = openReadOnly(src);
    ASSERT_GE(fd, 0);
    EXPECT_FALSE(copyDebsFromDirFd(fd, uniqueWorkDir()));
    ::close(fd);
}

TEST_F(UT_FdUtil, UT_FdUtil_copyDebsFromDirFd_EmptyDir)
{
    const QString emptyDir = uniqueWorkDir() + "/empty";
    ASSERT_TRUE(QDir().mkpath(emptyDir));
    const QString destDir = uniqueWorkDir() + "/backup_out_empty";
    ASSERT_TRUE(QDir().mkpath(destDir));
    int dirFd = openReadOnly(emptyDir, O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_TRUE(copyDebsFromDirFd(dirFd, destDir)); // 空目录也是成功（无 .deb 可拷）
    ::close(dirFd);
}

// ---------- bridgeFdToTempFile / removeBridgedTempFile ----------

TEST_F(UT_FdUtil, UT_FdUtil_bridgeFdToTempFile)
{
    const QString src = uniqueWorkDir() + "/driver中文名.deb";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("bridge-me");
    }
    int fd = openReadOnly(src);
    ASSERT_GE(fd, 0);
    QString err;
    const QString bridged = bridgeFdToTempFile(fd, src, err);
    ::close(fd);
    EXPECT_TRUE(err.isEmpty());
    EXPECT_TRUE(QFile::exists(bridged));
    EXPECT_TRUE(bridged.contains("devicemanager-fdbridge-"));
    EXPECT_TRUE(bridged.endsWith(".deb")); // 扩展名保留
    QFile bf(bridged);
    ASSERT_TRUE(bf.open(QIODevice::ReadOnly));
    EXPECT_EQ(bf.readAll(), QByteArray("bridge-me"));

    removeBridgedTempFile(bridged);
    EXPECT_FALSE(QFile::exists(bridged));
    EXPECT_FALSE(QFileInfo(bridged).absoluteDir().exists()); // 目录一并清理
}

TEST_F(UT_FdUtil, UT_FdUtil_bridgeFdToTempFile_InvalidInputs)
{
    QString err;
    EXPECT_TRUE(bridgeFdToTempFile(-1, "a.deb", err).isEmpty());

    // 目录 fd：拒绝
    int dirFd = openReadOnly(uniqueWorkDir(), O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    EXPECT_TRUE(bridgeFdToTempFile(dirFd, "a.deb", err).isEmpty());
    ::close(dirFd);
}

TEST_F(UT_FdUtil, UT_FdUtil_removeBridgedTempFile_NoOpForForeignPath)
{
    const QString foreign = uniqueWorkDir() + "/not_bridge.txt";
    {
        QFile f(foreign);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("keep");
    }
    removeBridgedTempFile(foreign); // 非桥接目录路径：不做任何事
    EXPECT_TRUE(QFile::exists(foreign));
    removeBridgedTempFile(QString()); // 空串：安全
    EXPECT_TRUE(QFile::exists(foreign));
}
