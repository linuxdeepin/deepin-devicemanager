// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DISABLE_DRIVER

#ifndef FDUTIL_H
#define FDUTIL_H

#include <QString>

/**
 * @brief 面向文件描述符的安全文件操作工具
 *
 * 沙箱加固（PMS: BUG-376053）后，守护进程启用 PrivateTmp/ProtectHome，
 * 不再按路径访问前端提供的 /home、/tmp 文件。前端打开文件/目录后通过
 * DBus 传递 QDBusUnixFileDescriptor，本模块负责校验 fd 并将其内容
 * 拷贝到守护进程私有目录，供既有的 dpkg-deb/deepin-deb-verify/QApt
 * 安装与校验逻辑（均要求按路径访问）复用。
 *
 * 安全约定：
 * 1. 一切访问基于 fd（fstat/openat/fdopendir），不经过 /proc/self/fd 路径重构；
 * 2. openat 一律携带 O_NOFOLLOW，目录项须为普通文件/目录，拒绝符号链接；
 * 3. 前端传来的名字只作文件名（非路径）使用，须通过 isValidComponentName 白名单校验；
 * 4. 桥接文件落在守护进程私有目录，权限 0600，用完即删。
 */
namespace FdUtil {

/**
 * @brief 校验路径分量名（纯文件名，非路径）
 *
 * 规则：仅允许字母、数字及 . _ + - ~，长度 1~200，且不得以 "." 开头，
 * 从而排除路径分隔符、目录穿越（..）与隐藏文件。
 * @param name 待校验的名字
 * @return true: 合法
 */
bool isValidComponentName(const QString &name);

/**
 * @brief 把任意文件名净化为可安全用作桥接文件名的分量
 *
 * 取最后一个 '/' 之后的基名，剥去前导点，将 [A-Za-z0-9._+~-] 之外的
 * 字符（含中文、空格、冒号等）替换为 '_'，保留最后一个扩展名，长度截断到 200。
 * @param filename 前端提供的原始文件名
 * @return 净化后的安全文件名（永不为空）
 */
QString sanitizedFileName(const QString &filename);

/**
 * @brief 将已打开的文件描述符内容完整拷贝到指定目标路径
 *
 * 仅接受普通文件（S_ISREG）；目标以 O_CREAT|O_EXCL|O_NOFOLLOW 创建，权限 0600。
 * @param srcFd 源文件描述符（不关闭，由调用方持有）
 * @param destPath 目标绝对路径（须位于守护进程私有目录）
 * @return true: 拷贝成功；失败时不残留目标文件
 */
bool copyFileByFd(int srcFd, const QString &destPath);

/**
 * @brief 将目录描述符下的 *.deb 普通文件拷贝到目标目录
 *
 * 用 fdopendir 枚举目录项，逐项 openat(O_NOFOLLOW) 打开；
 * 跳过点项、非 *.deb、符号链接与子目录（对齐旧 qCopyDirectory 的 *.deb 过滤），
 * 任一 .deb 项读取或拷贝失败则整体失败。
 * @param dirFd 源目录描述符（须为 O_RDONLY 打开的目录）
 * @param destDir 目标目录（须已存在）
 * @return true: 全部拷贝成功
 */
bool copyDebsFromDirFd(int dirFd, const QString &destDir);

/**
 * @brief 把 fd 内容桥接到守护进程私有临时文件
 *
 * 在 QDir::tempPath()（PrivateTmp 下为守护进程私有 /tmp 实例）下创建
 * devicemanager-fdbridge-<uuid> 目录，将 fd 内容拷贝为其中的净化后文件名。
 * 文件名只用于命名，不用于路径定位：先取基名并剥去前导点，再把
 * [A-Za-z0-9._+~-] 之外的字符替换为 '_'，保留扩展名以便后缀判定，
 * 因此任何前端文件名都不会导致拒绝（拒绝只基于 fd 本身）。
 * @param fd 前端传来的文件描述符
 * @param filename 前端提供的原始文件名（仅用于生成桥接文件名）
 * @param errOut 失败原因
 * @return 桥接文件绝对路径；失败返回空串（不留残留目录）
 */
QString bridgeFdToTempFile(int fd, const QString &filename, QString &errOut);

/**
 * @brief 删除 bridgeFdToTempFile 生成的桥接文件（连同其私有目录）
 * @param bridgedPath 桥接文件路径；空串或非桥接目录下的路径则不做任何事
 */
void removeBridgedTempFile(const QString &bridgedPath);

} // namespace FdUtil

#endif // FDUTIL_H
#endif // DISABLE_DRIVER
