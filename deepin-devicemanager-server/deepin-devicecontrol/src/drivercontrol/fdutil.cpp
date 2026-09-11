// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fdutil.h"
#include "DDLog.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUuid>
#include <QLoggingCategory>

#include <cerrno>
#include <cstring>
#include <vector>

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace DDLog;

namespace {

bool fstatIs(int fd, mode_t wantMask)
{
    struct stat st;
    if (::fstat(fd, &st) != 0) {
        qCWarning(appLog) << "fdutil: fstat failed:" << ::strerror(errno);
        return false;
    }
    return (st.st_mode & S_IFMT) == wantMask;
}

} // namespace

namespace FdUtil {

bool isValidComponentName(const QString &name)
{
    // 纯文件名：非空、不以 "." 开头（排除 ".."/隐藏文件）、仅允许安全字符集
    static const QRegularExpression safeName(QLatin1String("^[A-Za-z0-9._+~-]{1,200}$"));
    if (name.isEmpty() || name.startsWith(QLatin1Char('.'))) {
        return false;
    }
    return safeName.match(name).hasMatch();
}

bool copyFileByFd(int srcFd, const QString &destPath)
{
    if (srcFd < 0) {
        qCWarning(appLog) << "fdutil: copyFileByFd with invalid fd";
        return false;
    }
    if (!fstatIs(srcFd, S_IFREG)) { // 仅接受普通文件
        qCWarning(appLog) << "fdutil: src fd is not a regular file";
        return false;
    }
    const QByteArray destBytes = QFile::encodeName(destPath);
    // O_EXCL + O_NOFOLLOW：目标不存在才创建，且不允许是符号链接
    int destFd = ::open(destBytes.constData(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (destFd < 0) {
        qCWarning(appLog) << "fdutil: open dest failed:" << destPath << ::strerror(errno);
        return false;
    }

    std::vector<char> buf(256 * 1024);
    bool ok = true;
    for (;;) {
        ssize_t nread = ::read(srcFd, buf.data(), buf.size());
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            }
            qCWarning(appLog) << "fdutil: read src failed:" << ::strerror(errno);
            ok = false;
            break;
        }
        if (nread == 0) { // EOF
            break;
        }
        ssize_t written = 0;
        while (written < nread) {
            ssize_t nwrite = ::write(destFd, buf.data() + written, static_cast<size_t>(nread - written));
            if (nwrite < 0) {
                if (errno == EINTR) {
                    continue;
                }
                qCWarning(appLog) << "fdutil: write dest failed:" << destPath << ::strerror(errno);
                ok = false;
                break;
            }
            written += nwrite;
        }
        if (!ok) {
            break;
        }
    }

    if (::close(destFd) != 0) {
        qCWarning(appLog) << "fdutil: close dest failed:" << ::strerror(errno);
        ok = false;
    }
    if (!ok) {
        ::unlink(destBytes.constData()); // 失败不残留半截文件
    }
    return ok;
}

bool copyDebsFromDirFd(int dirFd, const QString &destDir)
{
    if (dirFd < 0) {
        qCWarning(appLog) << "fdutil: copyDebsFromDirFd with invalid fd";
        return false;
    }
    if (!fstatIs(dirFd, S_IFDIR)) { // 须为目录
        qCWarning(appLog) << "fdutil: dir fd is not a directory";
        return false;
    }
    int dupFd = ::dup(dirFd);
    if (dupFd < 0) {
        qCWarning(appLog) << "fdutil: dup dir fd failed:" << ::strerror(errno);
        return false;
    }
    DIR *dir = ::fdopendir(dupFd); // 成功后 dupFd 归 DIR 所有
    if (dir == nullptr) {
        qCWarning(appLog) << "fdutil: fdopendir failed:" << ::strerror(errno);
        ::close(dupFd);
        return false;
    }

    const QDir destQDir(destDir);
    bool ok = true;
    for (;;) {
        errno = 0;
        struct dirent *entry = ::readdir(dir);
        if (entry == nullptr) {
            if (errno != 0) { // 读目录出错
                qCWarning(appLog) << "fdutil: readdir failed:" << ::strerror(errno);
                ok = false;
            }
            break;
        }
        const QString name = QString::fromLocal8Bit(entry->d_name);
        if (name == QLatin1String(".") || name == QLatin1String("..")
                || name.startsWith(QLatin1Char('.'))) {
            continue;
        }
        // 与旧 qCopyDirectory 一致：只拷贝 *.deb（apt 版本号可含冒号等字符，
        // 按后缀过滤即可；d_name 本身不含路径分隔符，无穿越风险）
        if (!name.endsWith(QLatin1String(".deb"), Qt::CaseInsensitive)) {
            continue;
        }
        // 仅普通文件：拒绝符号链接/子目录/设备等（AT_SYMLINK_NOFOLLOW 不解引用）
        struct stat est;
        if (::fstatat(dirFd, entry->d_name, &est, AT_SYMLINK_NOFOLLOW) != 0) {
            qCWarning(appLog) << "fdutil: fstatat failed for" << name << ::strerror(errno);
            ok = false;
            break;
        }
        if (!S_ISREG(est.st_mode)) {
            qCInfo(appLog) << "fdutil: skip non-regular entry:" << name;
            continue;
        }
        int fileFd = ::openat(dirFd, entry->d_name, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        if (fileFd < 0) { // 含 ELOOP：检查后被替换成符号链接，宁可失败不可误拷
            qCWarning(appLog) << "fdutil: openat failed for" << name << ::strerror(errno);
            ok = false;
            break;
        }
        if (!copyFileByFd(fileFd, destQDir.filePath(name))) {
            ::close(fileFd);
            ok = false;
            break;
        }
        ::close(fileFd);
        qCInfo(appLog) << "fdutil: copied deb from dir fd:" << name;
    }

    ::closedir(dir);
    return ok;
}

QString sanitizedFileName(const QString &filename)
{
    // 取基名，剥去前导点，净化非白名单字符，保留扩展名
    QString base = filename;
    int slash = base.lastIndexOf(QLatin1Char('/'));
    if (slash >= 0) {
        base = base.mid(slash + 1);
    }
    while (base.startsWith(QLatin1Char('.'))) {
        base.remove(0, 1);
    }

    static const QRegularExpression validChars(QLatin1String("[^A-Za-z0-9._+~-]"));
    base.replace(validChars, QStringLiteral("_"));

    if (base.isEmpty()) {
        return QStringLiteral("driver.bin");
    }
    if (base.size() > 200) {
        // 截断时保留最后一个扩展名，避免影响后缀判定
        int dot = base.lastIndexOf(QLatin1Char('.'));
        const QString ext = (dot > 0) ? base.mid(dot) : QString();
        base = base.left(200 - ext.size()) + ext;
    }
    return base;
}

QString bridgeFdToTempFile(int fd, const QString &filename, QString &errOut)
{
    errOut.clear();
    if (fd < 0) {
        errOut = QStringLiteral("invalid fd");
        return QString();
    }
    struct stat st;
    if (::fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        errOut = QStringLiteral("fd is not a regular file");
        return QString();
    }
    // 文件名仅用于命名（非路径定位），净化后使用，永不因名字拒绝
    const QString safeName = sanitizedFileName(filename);

    // QDir::tempPath() 在 PrivateTmp=true 下是守护进程私有 /tmp 实例，
    // 其中的桥接文件对其他进程（含前端）不可见
    const QString bridgeDir = QDir::tempPath()
            + QStringLiteral("/devicemanager-fdbridge-%1")
                  .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (!QDir().mkpath(bridgeDir)) {
        errOut = QStringLiteral("create bridge dir failed");
        return QString();
    }
    // 限仅属主可访问（守护进程私有目录内再收紧一层）
    QFile::setPermissions(bridgeDir, QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);

    const QString bridgedPath = bridgeDir + QLatin1Char('/') + safeName;
    if (!copyFileByFd(fd, bridgedPath)) {
        errOut = QStringLiteral("copy fd content failed");
        QDir(bridgeDir).removeRecursively();
        return QString();
    }
    return bridgedPath;
}

void removeBridgedTempFile(const QString &bridgedPath)
{
    if (bridgedPath.isEmpty()) {
        return;
    }
    QDir dir = QFileInfo(bridgedPath).absoluteDir();
    // 仅清理本模块创建的桥接目录，防止误删
    if (dir.exists() && dir.dirName().startsWith(QStringLiteral("devicemanager-fdbridge-"))) {
        dir.removeRecursively();
    }
}

} // namespace FdUtil
