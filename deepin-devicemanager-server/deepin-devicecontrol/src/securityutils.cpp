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

    // 第一层：拒绝包含 ".." 的路径，防止路径穿越
    if (relativePath.contains(QLatin1String("..")))
        return false;

    // 第二层：构建完整路径并验证其 canonical path 以 /sys 开头
    QString fullPath = QStringLiteral("/sys") + relativePath;
    QFileInfo fileInfo(fullPath);
    QString canonicalPath = fileInfo.canonicalFilePath();

    if (canonicalPath.isEmpty())
        return false;

    // 确保 canonical path 以 /sys/ 开头，防止符号链接穿越到 /sys 外部
    if (!canonicalPath.startsWith(QLatin1String("/sys/")))
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
