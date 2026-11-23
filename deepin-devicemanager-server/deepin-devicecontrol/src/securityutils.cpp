// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file securityutils.cpp
 * @brief 路径安全校验公共函数实现
 */

#include "securityutils.h"

#include <QString>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDir>

bool isValidModName(const QString &modName)
{
    if (modName.isEmpty())
        return false;
    // 仅允许字母、数字、下划线和连字符，禁止路径穿越字符和空白符
    static const QRegularExpression kValidModName(QLatin1String("^[a-zA-Z0-9_-]+$"));
    return kValidModName.match(modName).hasMatch();
}

bool isPathWithinDirectory(const QString &filePath, const QString &baseDir)
{
    if (filePath.isEmpty() || baseDir.isEmpty())
        return false;

    QFileInfo fileInfo(filePath);
    QString canonicalPath = fileInfo.canonicalFilePath();
    if (canonicalPath.isEmpty())
        return false;

    QFileInfo dirInfo(baseDir);
    QString canonicalDir = dirInfo.canonicalFilePath();
    if (canonicalDir.isEmpty())
        return false;

    // 确保基准目录以 "/" 结尾，防止 "/etc" 被 "/etc-modprobe.d" 前缀匹配
    if (!canonicalDir.endsWith(QLatin1Char('/')))
        canonicalDir += QLatin1Char('/');

    return canonicalPath.startsWith(canonicalDir);
}

bool isSafeSysfsPath(const QString &relativePath)
{
    if (relativePath.isEmpty())
        return false;

    // 必须是以 / 开头的绝对路径（相对 /sys），防止拼出 /sysdevices 之类的路径
    if (!relativePath.startsWith(QLatin1Char('/')))
        return false;

    // 第一层：拒绝包含 ".." 的路径，防止路径穿越
    if (relativePath.contains(QLatin1String("..")))
        return false;

    /*
     * 第二层：对路径中「已存在的最深祖先目录」做 canonical path 校验，
     * 确保真实解析位置在 /sys 之下，防止符号链接穿越到 /sys 外部。
     *
     * 注意：不能要求完整路径本身存在——设备被禁用时会通过
     * /sys/.../remove 从内核移除，对应的 sysfs 目录随之消失，
     * 而「启用」操作正是针对这些不存在的路径恢复设备。
     * 若此处要求 canonicalFilePath() 非空，启用操作将永远被拒绝。
     */
    QString existing = QStringLiteral("/sys") + relativePath;
    while (!QFileInfo::exists(existing)) {
        const QString parent = QFileInfo(existing).dir().absolutePath();
        if (parent == QLatin1String("/") || parent == existing) {
            existing = QStringLiteral("/");
            break;
        }
        existing = parent;
    }

    QString canonicalPath = QFileInfo(existing).canonicalFilePath();
    if (canonicalPath.isEmpty())
        return false;

    // 确保 canonical path 在 /sys 之内（/sys 本身也是合法祖先）
    if (canonicalPath != QLatin1String("/sys") && !canonicalPath.startsWith(QLatin1String("/sys/")))
        return false;

    return true;
}

bool validateModNameForInstall(const QString &modName, const QString &kernelRelease, QString &errMsg)
{
    // 第一层：模块名白名单校验，确保不含路径分隔符或穿越字符
    if (!isValidModName(modName)) {
        errMsg = QString("Invalid module name: %1").arg(modName);
        return false;
    }

    // 第二层：校验构造的安装路径是否在预期目录下
    // 使用 QDir::cleanPath() 规范化路径后检查前缀（适用于尚未创建的目录）
    QString installdir = QString("/lib/modules/%1/custom/%2").arg(kernelRelease).arg(modName);
    QString normalizedPath = QDir::cleanPath(installdir);
    QString expectedPrefix = QString("/lib/modules/%1/custom/").arg(kernelRelease);

    if (!normalizedPath.startsWith(expectedPrefix)) {
        errMsg = "Path traversal attempt detected";
        return false;
    }

    return true;
}
