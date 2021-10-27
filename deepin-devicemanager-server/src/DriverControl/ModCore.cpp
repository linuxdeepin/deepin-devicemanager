/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     kongyunzhen <kongyunzhen@uniontech.com>
*
* Maintainer: kongyunzhen <kongyunzhen@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ModCore.h"

#include <QDebug>

ModCore::ModCore(QObject *parent)
    : QObject(parent)
{

}

/**
 * @brief ModCore::checkModuleInUsed 获取依赖当前模块在使用的模块
 * @param modName 模块名 sample: hid or hid.ko /xx/xx/hid.ko
 * @param bfrompath modName是否为文件 true:/xx/xx/hid.ko false:hid or hid.ko
 * @return 返回当前依赖模块列表 like ['usbhid', 'hid_generic']
 */
QStringList ModCore::checkModuleInUsed(const QString &modName, bool bfrompath)
{
    QStringList modList;
    struct kmod_ctx *ctx = nullptr;
    const char *null_config = nullptr;
    struct kmod_module *mod = nullptr;

    ctx = kmod_new(nullptr, &null_config);
    if (!ctx) {
        qInfo() << "kmod_new() failed!";
    } else {
        int err = 0;
        err = modNew(ctx, modName, mod, bfrompath);

        if (err < 0) {
            qInfo() << "modulename=" << modName << "modNew failed errcode=" << err;
        } else {
            struct kmod_list *holders = nullptr;
            int state = 0;
            state = kmod_module_get_initstate(mod);

            if (state == KMOD_MODULE_BUILTIN) {
                qInfo() << QString("Module %1 is builtin.").arg(kmod_module_get_name(mod)) << " errcode=" << -ENOENT;
            } else if (state < 0) {
                qInfo() << QString("Module %1 is not currently loaded").arg(kmod_module_get_name(mod)) << " errcode=" << -ENOENT;
            } else {
                holders = kmod_module_get_holders(mod);
                if (holders != nullptr) {
                    struct kmod_list *itr;
                    qInfo() << QString("Module %1 is in use by:").arg(kmod_module_get_name(mod));
                    kmod_list_foreach(itr, holders) {
                        struct kmod_module *hm = kmod_module_get_module(itr);
                        modList.append(QString("%1").arg(kmod_module_get_name(hm)));
                        fprintf(stderr, " %s", kmod_module_get_name(hm));
                        kmod_module_unref(hm);
                    }
                    kmod_module_unref_list(holders);
                }
            }
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }

    return  modList;
}

/**
 * @brief ModCore::rmModForce 强制移除驱动模块 等效于rmmod -f
 * @param modName 模块名sample: hid or hid.ko
 * @return 删除结果 true:成功 false: 失败
 */
bool ModCore::rmModForce(const QString &modName)
{
    bool bsuccess = true;
    struct kmod_ctx *ctx = nullptr;
    const char *null_config = nullptr;
    struct kmod_module *mod = nullptr;

    ctx = kmod_new(nullptr, &null_config);
    if (!ctx) {
        bsuccess = false;
        qInfo() << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        err = modNew(ctx, modName, mod, false);

        if (err < 0) {
            bsuccess = false;
            qInfo() << __func__ << "modulename=" << modName << "modNew failed errcode=" << err;
        } else {
            err = kmod_module_remove_module(mod, KMOD_REMOVE_FORCE);
            if (err < 0) {
                bsuccess = false;
                qInfo() << __func__ << QString("could not remove module %1: %1\n").arg(modName).arg(err);
            }

            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  bsuccess;
}

/**
 * @brief ModCore::modGetPath 通过模块名获取驱动所在路径
 * @param modName 模块名sample: hid or hid.ko
 * @return 模块所在路径
 */
QString ModCore::modGetPath(const QString &modName)
{
    QString path;
    struct kmod_ctx *ctx = nullptr;
    const char *null_config = nullptr;
    struct kmod_module *mod = nullptr;

    ctx = kmod_new(nullptr, &null_config);
    if (!ctx) {
        qInfo() << __func__ << "kmod_new() failed!";
    } else {
        int err = 0;
        err = modNew(ctx, modName, mod, false);

        if (err < 0) {
            qInfo() << __func__ << "modulename=" << modName << "modNew failed errcode=" << err;
        } else {
            path.append(kmod_module_get_path(mod));
            kmod_module_unref(mod);
        }
        kmod_unref(ctx);
    }
    return  path;
}

/**
 * @brief ModCore::modNew new一个新kmod_module
 * @param ctx kmod 上下文对象
 * @param modName 模块名 sample: hid or hid.ko /xx/xx/hid.ko
 * @param mod 模块对象
 * @param bfrompath 是否为文件 true:/xx/xx/hid.ko false:hid or hid.ko
 * @return 错误码
 */
int ModCore::modNew(struct kmod_ctx *ctx, const QString &modName, kmod_module *&mod, bool bfrompath)
{
    int ret = 0;
    if (bfrompath) {
        ret = kmod_module_new_from_path(ctx, modName.toStdString().c_str(), &mod);
    } else {
        ret = kmod_module_new_from_name(ctx, modName.toStdString().c_str(), &mod);
    }
    return  ret;
}

