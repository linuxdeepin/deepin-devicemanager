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
#ifndef MODCORE_H
#define MODCORE_H

#include <QObject>
#include <QStringList>

#include <libkmod.h>

class ModCore: public QObject
{
    Q_OBJECT
public:
    //错误类型
    enum ErrorCode {
        // An invalid/unknown value
        UnknownError = -1,
        // An error code representing no error
        Success = 0,
        // Module not found
        NotFoundError,
        // kmod new error
        KmodNewError,
        // Module already in kernel
        ModExistError,
        // Unknown symbol in module or unknown parameter (see dmesg)
        UnknownSymbolError,
        // Insert insert error
        InsertModError
    };
    //模块信息相关类型
    enum ModInfoType {
        EAlias = 0,
        ELiense,
        EVersion,
        EAuther,
        EDescription,
        ESrcVersion,
        EName,
        EVermagic
    };

    //conf type
    enum ModConfType {
        EBlackListConf = 0,
        EAliasConf,
        EOptionsConf,
        ESoftdepConf
    };

public:
    explicit ModCore(QObject *parent = nullptr);
    //获取依赖当前模块在使用的模块
    QStringList checkModuleInUsed(const QString &modName);
    //强制移除驱动模块 等效于rmmod -f
    bool rmModForce(const QString &modName);
    //ko安装
    ErrorCode modInstall(const QString &modName, unsigned int flags = 0);
    //直接通过ko文件的绝对路径安装
    ErrorCode koInstall(const QString &path, unsigned int flags = 0);
    //通过模块名获取驱动所在路径
    QString  modGetPath(const QString &modName);
    //获取指定驱动名称
    QString modGetName(const QString &modPath);
    //获取驱动信息
    QString modGetInfo(const QString &modName, ModInfoType infotype);
    //判断模块是否为内建
    bool modIsBuildIn(const QString &modName);
    //判断模块是否已加载
    bool modIsLoaded(const QString &modName);
    //判断模块是否在黑名单中
    bool modIsBlackListed(const QString &modName);
    //获取指定项conf
    QStringList modGetConfsWithType(ModConfType conftype);
    //判断是否为驱动文件
    bool isModFile(const QString &filePath);
    //将Mod加入黑名单
    bool addModBlackList(const QString &modName);
    //将Mod移除黑名单
    void rmFromBlackList(const QString &modName);
    //设置Mod开机启动
    bool setModLoadedOnBoot(const QString &modName);
    //移除mod loaded on boot
    void rmModLoadedOnBoot(const QString &modName);
    //通过错误码获取错误信息内容
    static QString errCode2String(ErrorCode errcode);


private:
    //new一个新kmod_module
    int modNew(struct kmod_ctx *ctx, const QString &modName, kmod_module *&mod);
    //判断mod是文件路径还是模块名
    bool bFromPath(const QString &modName);
    //infotype to string
    QString infoType2String(ModInfoType infotype);
    //获取kmod init state
    int modGetInitState(const QString &modName);
    //blacklist 文件删除操作
    void rmModFromBlackListFile(const QString &filepath, const QString &modName);
    //删除自启动配置
    void rmModFromLoadonBootConf(const QString &filepath, const QString &modName);
    //删除文件指定行
    void deleteLineOfFileWithItem(const QString &filepath, const QString &item);
    /**
     * @brief updateInitramfs       更新现有的initramfs
     */
    void updateInitramfs();

};

#endif // MODCORE_H
