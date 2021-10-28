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
#include "DriverManager.h"
#include "Utils.h"
#include "ModCore.h"

#include <QFile>

DriverManager::DriverManager(QObject *parent)
    : QObject(parent)
    , mp_modcore(new ModCore(this))
{

}

/**
 * @brief DriverManager::unInstallDriver 卸载驱动
 * @param modulename 驱动模块名
 * @return  true:卸载成功 false:卸载失败
 */
bool DriverManager::unInstallDriver(const QString &modulename)
{
    bool bsuccess = false;
    sigProgressDetail(5, "");
    QString modulePath = mp_modcore->modGetPath(modulename);
    QString modulePackageName = Utils::file2PackageName(modulePath);
    QString kernelRelease = Utils::kernelRelease();
    sigProgressDetail(10, "");
    //当模块包为内核模块包时不允许删除，采用卸载加黑名单禁用方式
    if (modulePackageName.contains(kernelRelease)) {
        bsuccess = unInstallModule(modulename);
    } else {
        bsuccess = unInstallModule(modulename, modulePackageName);
    }
    sigProgressDetail(100, "");
    return  bsuccess;
}

bool DriverManager::installDriver(const QString &filepath)
{
    Q_UNUSED(filepath);
    return  true;
}

/**
 * @brief DriverManager::checkModuleInUsed 获取依赖当前模块在使用的模块
 * @param modName sample: hid or hid.ko /xx/xx/hid.ko
 * @param bfrompath true:/xx/xx/hid.ko false:hid or hid.ko
 * @return 返回当前依赖模块列表 like ['usbhid', 'hid_generic']
 */
QStringList DriverManager::checkModuleInUsed(const QString &modName)
{
    if (nullptr == mp_modcore)
        return QStringList();
    QFile file(modName);

    bool bpath = file.exists() && modName.endsWith("ko");
    return  mp_modcore->checkModuleInUsed(modName, bpath);
}

/**
 * @brief DriverManager::unInstallModule 驱动卸载
 * @param moduleName 模块名称 like hid or hid.ko
 * @return true: 成功 false: 失败
 */
bool DriverManager::unInstallModule(const QString &moduleName)
{
    bool bsuccess = true;
    sigProgressDetail(40, "");
    if (!mp_modcore->rmModForce(moduleName)) {
        bsuccess = false;
    } else {
        sigProgressDetail(60, "");
        if (!Utils::addModBlackList(moduleName)) {
            bsuccess = false;
        }
    }
    sigProgressDetail(80, "");
    return  bsuccess;
}

/**
 * @brief DriverManager::unInstallModule 驱动卸载
 * @param moduleName 模块名称 sample: hid or hid.ko
 * @param packageName 驱动所在软件包名  sample: nvidia-driver
 * @return true: 成功 false: 失败
 */
bool DriverManager::unInstallModule(const QString &moduleName, const QString &packageName)
{
    bool bsuccess = true;
    sigProgressDetail(20, "");
    if (!mp_modcore->rmModForce(moduleName)) {
        bsuccess = false;
    } else {
        sigProgressDetail(30, "");
        if (!Utils::unInstallPackage(packageName)) {
            bsuccess = false;
        }
    }
    sigProgressDetail(80, "");
    return  bsuccess;
}

