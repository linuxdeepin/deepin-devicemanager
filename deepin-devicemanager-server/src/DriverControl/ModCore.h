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
    explicit ModCore(QObject *parent = nullptr);
    //获取依赖当前模块在使用的模块
    QStringList checkModuleInUsed(const QString &modName, bool bfrompath = false);
    //强制移除驱动模块 等效于rmmod -f
    bool rmModForce(const QString &modName);
    //通过模块名获取驱动所在路径
    QString  modGetPath(const QString &modName);


private:
    //new一个新kmod_module
    int modNew(struct kmod_ctx *ctx, const QString &modName, kmod_module *&mod, bool bfrompath);

};

#endif // MODCORE_H
