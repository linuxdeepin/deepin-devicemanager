// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file securityutils.h
 * @brief 路径安全校验公共函数库
 *
 * 提供统一的路径穿越防护校验函数，供所有模块使用。
 * 所有涉及用户输入构造文件路径的操作都应使用这些函数进行校验。
 */

#ifndef SECURITYUTILS_H
#define SECURITYUTILS_H

#include <QString>

/**
 * @brief 校验模块名是否合法（仅允许字母、数字、下划线和连字符）
 *
 * Linux 内核模块名本身只允许字母、数字、下划线和连字符。
 * 此函数用于防止通过构造恶意模块名进行路径穿越攻击。
 *
 * @param modName 模块名称
 * @return true:合法 false:非法
 */
bool isValidModName(const QString &modName);

/**
 * @brief 校验解析后的真实路径是否在预期目录内（防止路径穿越）
 *
 * 使用 QFileInfo::canonicalFilePath() 解析符号链接和 .. 路径，
 * 然后验证最终路径是否在预期的基准目录内。
 *
 * @param filePath 构造的文件路径（可以是相对或绝对路径）
 * @param baseDir 预期的基准目录（必须是绝对路径）
 * @return true:合法 false:发生路径穿越
 */
bool isPathWithinDirectory(const QString &filePath, const QString &baseDir);

/**
 * @brief 校验 sysfs 路径是否安全，防止路径穿越攻击
 *
 * sysfs 虚拟文件系统中的路径通常以 /sys 开头。
 * 此函数用于防止通过构造恶意路径访问 /sys 外部的系统文件。
 *
 * @param relativePath 相对于 /sys 的设备路径（不应包含 .. 或绝对路径成分）
 * @return true:安全 false:不安全
 */
bool isSafeSysfsPath(const QString &relativePath);

/**
 * @brief 校验模块名是否可用于驱动安装路径构造
 *
 * 综合检查：模块名合法性 + 安装目录路径边界检查。
 * 用于 installDriver 等需要安装驱动的场景。
 *
 * @param modName 模块名称
 * @param kernelRelease 内核版本号（来自 /lib/modules/<kernelRelease>/custom/）
 * @param errMsg 如果验证失败，通过此参数返回错误信息
 * @return true:验证通过 false:验证失败（errMsg 包含原因）
 */
bool validateModNameForInstall(const QString &modName, const QString &kernelRelease, QString &errMsg);

#endif // SECURITYUTILS_H
