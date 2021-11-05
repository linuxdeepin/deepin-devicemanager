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
#ifndef UTILS_H
#define UTILS_H
#include <QString>

class Utils
{
public:
    Utils();
    static QString file2PackageName(const QString &fileName);
    static QString getModuleFilePath(const QString &moduleName);
    static QString kernelRelease();
    static QString machineArch();
    static bool addModBlackList(const QString &moduleName);
    static bool unInstallPackage(const QString &packageName);
    //判断Deb是否为驱动包
    static bool isDriverPackage(const QString &filepath);
    //更新mod dep
    static bool updateModDeps(bool bquick = true);
    //判断文件是否被占用
    static bool isFileLocked(const QString &filepath, bool bread = false);
};

#endif // UTILS_H
