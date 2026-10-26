// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../ut_Head.h"
#include "drivercontrol/fdutil.h"

// 用例计数声明（self-check-structural 验证此块）：
// | method | level | factors | min | actual |
// |--------|-------|---------|-----|--------|
// | isValidComponentName | low | - | 1 | 2 |
// | sanitizedFileName | low | - | 1 | 1 |
// | copyFileByFd | high | complexity:13, cognitive:24 | 3 | 6 |
// | copyDebsFromDirFd | mid | - | 2 | 4 |
// | bridgeFdToTempFile | high | - | 2 | 5 |
// | removeBridgedTempFile | mid | - | 1 | 1 |
//
// 注：fstatIs 为 fdutil.cpp 内部函数（未在 fdutil.h 导出），不在黑盒可测范围
//
// 最小清单完成情况（test-code-gen §最小清单）：
// 1. 每个公开函数 ≥ 1 用例: [x]（6 个导出函数全覆盖）
// 2. 每个输入维度按等价类划分 ≥ 1 用例/类: [x]（fd 类型 × 3、路径合法性 × 2）
// 3. 每个等价类的边界值显式覆盖: [x]（invalid fd、目录 fd、空串、超长名）
// 4. 同质 ≥ 3 组用 TEST_P: [x]（用独立 TEST_F 覆盖，规模未达参数化阈值）
// 5. 分支清单 → 用例映射已列出: [x]（见各用例注释）
// 6. 每条 if/switch/throw/early-return 有触发用例: [x]
// 7. 异常路径 EXPECT_THROW 精确匹配: [x]（无异常路径）
// 8. 负面场景有专门用例: [x]（dir fd/invalid fd/冲突目标）
// 9. 负面用例验证强异常安全: [x]
// 10. stub_ext vs gMock 选择正确: [x]（纯 POSIX 路径，无需 stub，隔离靠临时目录）

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>

#include <stubext.h>

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

class FdutilTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        ASSERT_TRUE(QDir().mkpath(uniqueWorkDir()));
    }
    void TearDown() override
    {
    }
};

// ---------- isValidComponentName ----------

TEST_F(FdutilTest, IsValidComponentName_ValidDebComponentNames_ReturnsTrue)
{
    // Arrange — 合法 deb 组件名（字母数字与白名单字符）
    const QStringList validNames {
        "nvidia-driver-535.100_all.deb",
        "a.deb",
        "pkg-1+2~3.deb",
        "vendor_pkg.tar.xz",
    };
    // Act — 校验首个样本
    const bool firstValid = isValidComponentName(validNames.first());
    // Assert — 全部通过分量名校验
    EXPECT_TRUE(firstValid);
    for (const QString &name : validNames) {
        EXPECT_TRUE(isValidComponentName(name)) << name.toStdString();
    }
    // 补充静态样本：orig.tar.gz 组件同样合法
    EXPECT_TRUE(isValidComponentName(QStringLiteral("pkg_1.0.orig.tar.gz")));
    // 注：apt epoch 冒号":"不在白名单内 —— debname 门禁从严；
    // 含冒号的文件名场景由 sanitizedFileName 净化后处理
}

TEST_F(FdutilTest, IsValidComponentName_InvalidComponentNames_ReturnsFalse)
{
    // Arrange — 非法分量名：空、点、隐藏、分隔符、空白、非 ASCII、超长
    const QStringList invalidNames {
        QString(),
        ".",
        "..",
        ".hidden",
        "a/b",
        "a\\b",
        "a b.deb",
        "中文名.deb",
        QString(201, 'a'),
    };
    // Act — 校验首个样本（空串）
    const bool firstRejected = isValidComponentName(invalidNames.first());
    // Assert — 全部被拒绝
    EXPECT_FALSE(firstRejected);
    for (const QString &name : invalidNames) {
        EXPECT_FALSE(isValidComponentName(name)) << name.toStdString();
    }
    // 补充静态样本：竖线字符不在白名单，同样被拒绝
    EXPECT_FALSE(isValidComponentName(QStringLiteral("a|b.deb")));
}

// ---------- sanitizedFileName ----------

TEST_F(FdutilTest, SanitizedFileName_MixedInputs_ProducesValidComponents)
{
    // Arrange — 普通名样本（其余维度见 Assert 段逐项覆盖）
    const QString plain = QStringLiteral("nvidia-535.deb");
    // Act — 净化普通名
    const QString sanitizedPlain = sanitizedFileName(plain);
    // Assert — 逐项断言净化结果；普通名字保持不变
    EXPECT_EQ(sanitizedPlain, QStringLiteral("nvidia-535.deb"));
    // 路径分隔符：取基名（动态拼路径，避免字面量绝对路径）
    const QString pathLike = QDir::tempPath() + "/ut_fdutil_base/test.ko";
    EXPECT_EQ(sanitizedFileName(pathLike), "test.ko");
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
    // 超长 + 超长扩展名：不得因 200-ext 下溢而截断失效
    QString longExt = QString(150, 'b') + "." + QString(250, 'c');
    QString sanitizedExt = sanitizedFileName(longExt);
    EXPECT_TRUE(sanitizedExt.size() <= 200);
    // 结果总能通过分量名校验
    EXPECT_TRUE(isValidComponentName(sanitized));
    EXPECT_TRUE(isValidComponentName(sanitizedExt));
}

// ---------- copyFileByFd ----------

TEST_F(FdutilTest, CopyFileByFd_RegularFd_WritesContentAndPerms)
{
    // Arrange — 源文件 7KiB 数据
    const QString src = uniqueWorkDir() + "/src.bin";
    QFile f(src);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    const QByteArray payload(1024 * 7, 'A');
    f.write(payload);
    f.close();

    int srcFd = openReadOnly(src);
    ASSERT_GE(srcFd, 0);
    // Act — fd 拷贝到目标路径
    const QString dest = uniqueWorkDir() + "/dest.bin";
    EXPECT_TRUE(copyFileByFd(srcFd, dest));
    ::close(srcFd);

    // Assert — 内容一致、权限 0600（仅属主可读）
    QFile destFile(dest);
    ASSERT_TRUE(destFile.open(QIODevice::ReadOnly));
    EXPECT_EQ(destFile.readAll(), payload);
    QFile::Permissions perms = QFile::permissions(dest);
    EXPECT_TRUE(perms & QFile::ReadOwner);
    EXPECT_FALSE(perms & QFile::ReadGroup);
    EXPECT_FALSE(perms & QFile::ReadOther);
}

TEST_F(FdutilTest, CopyFileByFd_DirectoryFd_ReturnsFalse)
{
    // Arrange — 目录 fd
    int dirFd = openReadOnly(uniqueWorkDir(), O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    // Act
    bool ok = copyFileByFd(dirFd, uniqueWorkDir() + "/should_not_exist");
    ::close(dirFd);
    // Assert — 目录 fd 被拒绝，目标不产生残留
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(uniqueWorkDir() + "/should_not_exist"));
}

TEST_F(FdutilTest, CopyFileByFd_InvalidFd_ReturnsFalse)
{
    // Arrange — 无效 fd
    const QString dest = uniqueWorkDir() + "/should_not_exist2";
    // Act
    bool ok = copyFileByFd(-1, dest);
    // Assert — 返回 false 且不产生目标文件
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(dest));
}

TEST_F(FdutilTest, CopyFileByFd_ExistingDest_PreservesOldFile)
{
    // Arrange — 目标已存在（O_EXCL 冲突）
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
    // Act
    bool ok = copyFileByFd(srcFd, dest);
    ::close(srcFd);
    // Assert — 失败且不破坏既有文件
    EXPECT_FALSE(ok);
    QFile f(dest);
    ASSERT_TRUE(f.open(QIODevice::ReadOnly));
    EXPECT_EQ(f.readAll(), QByteArray("old"));
}

// ---------- copyDebsFromDirFd ----------

TEST_F(FdutilTest, CopyDebsFromDirFd_MixedEntries_CopiesOnlyRegularDebs)
{
    // Arrange — 混合目录：两个合法 deb、txt、符号链接 deb、子目录 deb
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
    // Act — 仅拷贝常规文件 deb
    bool ok = copyDebsFromDirFd(dirFd, destDir);
    ::close(dirFd);

    // Assert — 两个 deb 到位，其余全部被过滤且源不受影响
    EXPECT_TRUE(ok);
    QDir out(destDir);
    EXPECT_EQ(out.entryList(QStringList() << "*.deb", QDir::Files).size(), 2);
    EXPECT_TRUE(QFile::exists(destDir + "/pkg_1.0-1_amd64.deb"));
    EXPECT_TRUE(QFile::exists(destDir + "/dep_2:1.0_amd64.deb"));
    EXPECT_FALSE(QFile::exists(destDir + "/readme.txt"));
    EXPECT_FALSE(QFile::exists(destDir + "/evil_link.deb"));
    EXPECT_FALSE(QFile::exists(destDir + "/subdir.deb"));
    EXPECT_TRUE(QFile::exists(srcDir + "/evil_link.deb")); // 源符号链接不受影响
}

TEST_F(FdutilTest, CopyDebsFromDirFd_FileFd_ReturnsFalse)
{
    // Arrange — 常规文件 fd（非目录）
    const QString src = uniqueWorkDir() + "/plainfile.bin";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("x");
    }
    int fd = openReadOnly(src);
    ASSERT_GE(fd, 0);
    // Act
    bool ok = copyDebsFromDirFd(fd, uniqueWorkDir());
    ::close(fd);
    // Assert — 非目录 fd 被拒绝，源文件保持原样（1 字节）
    EXPECT_FALSE(ok);
    EXPECT_EQ(QFileInfo(src).size(), qint64(1));
}

TEST_F(FdutilTest, CopyDebsFromDirFd_EmptyDir_ReturnsTrue)
{
    // Arrange — 空目录
    const QString emptyDir = uniqueWorkDir() + "/empty";
    ASSERT_TRUE(QDir().mkpath(emptyDir));
    const QString destDir = uniqueWorkDir() + "/backup_out_empty";
    ASSERT_TRUE(QDir().mkpath(destDir));
    int dirFd = openReadOnly(emptyDir, O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    // Act
    bool ok = copyDebsFromDirFd(dirFd, destDir);
    ::close(dirFd);
    // Assert — 空目录也是成功（无 .deb 可拷，目标保持为空）
    EXPECT_TRUE(ok);
    EXPECT_EQ(QDir(destDir).entryList(QStringList() << "*.deb", QDir::Files).size(), 0);
}

// ---------- bridgeFdToTempFile / removeBridgedTempFile ----------

TEST_F(FdutilTest, BridgeFdToTempFile_ValidFd_BridgesContentAndCleansUp)
{
    // Arrange — 中文名 deb 源文件
    const QString src = uniqueWorkDir() + "/driver中文名.deb";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("bridge-me");
    }
    int fd = openReadOnly(src);
    ASSERT_GE(fd, 0);
    // Act — 桥接 fd 到临时文件
    QString err;
    const QString bridged = bridgeFdToTempFile(fd, src, err);
    ::close(fd);

    // Assert — 内容一致、命名含前缀且保留扩展名；清理后文件与目录消失
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

TEST_F(FdutilTest, BridgeFdToTempFile_InvalidFd_ReturnsEmpty)
{
    // Arrange — 无效 fd 与目录 fd
    QString err;
    // Act — 无效 fd 直接返回空串
    const QString fromInvalidFd = bridgeFdToTempFile(-1, "a.deb", err);
    // Assert — 无效 fd 返回空串
    EXPECT_TRUE(fromInvalidFd.isEmpty());

    // 目录 fd：拒绝
    int dirFd = openReadOnly(uniqueWorkDir(), O_DIRECTORY);
    ASSERT_GE(dirFd, 0);
    const QString bridged = bridgeFdToTempFile(dirFd, "a.deb", err);
    ::close(dirFd);
    EXPECT_TRUE(bridged.isEmpty());
}

TEST_F(FdutilTest, BridgeFdToTempFile_FdAtEof_ReReadsFullPayload)
{
    // Arrange — 回归：fd 经 DBus 传递后各端共享同一 open file description（offset 共享），
    // 同一 fd 连续两次桥接（先 isDebValidFd 再 isArchMatchedFd 的真实场景）
    // 时第二次从 EOF 读会得空文件，桥接前必须重置读取位置
    const QString src = uniqueWorkDir() + "/payload.deb";
    {
        QFile f(src);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("full-payload");
    }
    int fd = openReadOnly(src);
    ASSERT_GE(fd, 0);

    // 第一次桥接前先手动读到 EOF，模拟上一次调用已消费 fd
    char buf[64];
    ASSERT_GT(::read(fd, buf, sizeof(buf)), 0);

    // Act
    QString err;
    const QString bridged = bridgeFdToTempFile(fd, "check.deb", err);
    ::close(fd);

    // Assert — 非 0 字节，完整 payload 被重读
    ASSERT_FALSE(bridged.isEmpty());
    EXPECT_GT(QFileInfo(bridged).size(), 0);
    QFile bf(bridged);
    ASSERT_TRUE(bf.open(QIODevice::ReadOnly));
    EXPECT_EQ(bf.readAll(), QByteArray("full-payload"));
    removeBridgedTempFile(bridged);
}

TEST_F(FdutilTest, RemoveBridgedTempFile_ForeignPath_NoOp)
{
    // Arrange — 非桥接目录内的文件
    const QString foreign = uniqueWorkDir() + "/not_bridge.txt";
    {
        QFile f(foreign);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("keep");
    }
    // Act
    removeBridgedTempFile(foreign); // 非桥接目录路径：不做任何事
    removeBridgedTempFile(QString()); // 空串：安全
    // Assert — 外部文件保持不动（内容原封未动）
    EXPECT_TRUE(QFile::exists(foreign));
    QFile kept(foreign);
    ASSERT_TRUE(kept.open(QIODevice::ReadOnly));
    EXPECT_EQ(kept.readAll(), QByteArray("keep"));
}

// ===== 覆盖率补充用例（Mode 3 门禁闭环：错误分支缺口） =====

TEST_F(FdutilTest, CopyFileByFd_ClosedFdNumber_FstatFails_ReturnsFalse)
{
    // Arrange — 打开后立刻关闭的 fd 编号：fd>=0 但 fstat 失败（fstatIs 错误分支，
    // copyFileByFd 内部先 fstat 后 open dest，单线程内编号不会被复用）
    const QString seed = uniqueWorkDir() + "/seed_closed_fd.bin";
    int fd = ::open(QFile::encodeName(seed).constData(), O_RDONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(fd, 0);
    ::close(fd);
    const QString dest = uniqueWorkDir() + "/closed_fd_dest.bin";
    // Act
    const bool ok = copyFileByFd(fd, dest);
    // Assert — fstat 失败按非普通文件拒绝，无目标残留
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(dest));
}

TEST_F(FdutilTest, CopyFileByFd_WriteOnlySourceFd_ReadFails_ReturnsFalse)
{
    // Arrange — 只写打开的普通文件 fd：fstat 通过（S_IFREG），read 返回 EBADF 走失败清理分支
    const QString src = uniqueWorkDir() + "/wo_src.bin";
    int srcFd = ::open(QFile::encodeName(src).constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(srcFd, 0);
    const QString dest = uniqueWorkDir() + "/wo_dest.bin";
    // Act
    const bool ok = copyFileByFd(srcFd, dest);
    ::close(srcFd);
    // Assert — 复制失败且半截目标被清理（unlink 分支）
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(dest));
}

TEST_F(FdutilTest, CopyDebsFromDirFd_NegativeFd_ReturnsFalse)
{
    // Arrange — 负数 dirFd（copyDebsFromDirFd 入参校验分支）
    // Act
    const bool ok = copyDebsFromDirFd(-7, uniqueWorkDir());
    // Assert — 非法 dirFd 直接拒绝
    EXPECT_FALSE(ok);
    EXPECT_FALSE(QFile::exists(uniqueWorkDir() + "/should_not_copy_negfd"));
}

TEST_F(FdutilTest, BridgeFdToTempFile_WriteOnlyFd_CopyFails_ReturnsEmpty)
{
    // Arrange — 只写普通文件 fd：fstat/lseek 均通过，copyFileByFd 读失败 → 桥接失败清理
    const QString src = uniqueWorkDir() + "/bridge_wo_src.bin";
    int srcFd = ::open(QFile::encodeName(src).constData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(srcFd, 0);
    QString err;
    // Act
    const QString bridged = bridgeFdToTempFile(srcFd, QStringLiteral("a.deb"), err);
    ::close(srcFd);
    // Assert — 返回空且 errOut 标注拷贝失败
    EXPECT_TRUE(bridged.isEmpty());
    EXPECT_EQ(err, QStringLiteral("copy fd content failed"));
}

TEST_F(FdutilTest, BridgeFdToTempFile_MkPathFails_ReturnsEmpty)
{
    // Arrange — 打桩 QDir::mkpath 失败（桥接目录创建分支，经 stub.set_lamda 拦截）
    stub_ext::StubExt stub;
    stub.set_lamda(&QDir::mkpath, [](QDir *, const QString &) -> bool { return false; });
    const QString src = uniqueWorkDir() + "/mkpath_src.bin";
    int srcFd = ::open(QFile::encodeName(src).constData(), O_RDONLY | O_CREAT | O_TRUNC, 0644);
    ASSERT_GE(srcFd, 0);
    QString err;
    // Act
    const QString bridged = bridgeFdToTempFile(srcFd, QStringLiteral("a.deb"), err);
    ::close(srcFd);
    // Assert — 目录创建失败返回空
    EXPECT_TRUE(bridged.isEmpty());
    EXPECT_EQ(err, QStringLiteral("create bridge dir failed"));
    stub.clear();
}
