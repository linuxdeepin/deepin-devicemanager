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
#include "Utils.h"

#include <QProcess>

#include <sys/utsname.h>

const QString BLACKLIST_CONF = "/etc/modprobe.d/blacklist-devicemanager.conf";

Utils::Utils()
{

}

QString Utils::file2PackageName(const QString &fileName)
{
    QString packageName("");
    QProcess process;
    process.start("dpkg", QStringList() << "-S" << fileName);
    if (!process.waitForFinished())
        return packageName;
    //content like linux-image-4.19.0-amd64-desktop: /lib/modules/4.19.0-amd64-desktop/kernel/drivers/hid/usbhid/usbhid.ko
    QString strContent = process.readAll();
    QStringList strList = strContent.split(":");

    if (strList.size() > 0) {
        packageName = strList.at(0);
    }

    return  packageName;
}


QString Utils::getModuleFilePath(const QString &moduleName)
{
    QString koPath("");
    QProcess process;
    process.start("modinfo", QStringList() << "-n" << moduleName);
    if (!process.waitForFinished())
        return koPath;
    koPath = process.readAll();
    koPath.remove("\n");

    return  koPath;
}

QString Utils::kernelRelease()
{
    struct utsname name;
    uname(&name);
    QString strKernel(name.release);

    return  strKernel;
}

bool Utils::addModBlackList(const QString &moduleName)
{
    QProcess process;
    process.start(QString("echo blacklist %1 >> %2").arg(moduleName).arg(BLACKLIST_CONF));
    if (!process.waitForFinished())
        return  false;

    return  true;
}

bool Utils::unInstallPackage(const QString &packageName)
{
    QProcess process;
    process.start(QString("apt remove %1").arg(packageName));
    if (!process.waitForFinished())
        return  false;

    return  true;
}
