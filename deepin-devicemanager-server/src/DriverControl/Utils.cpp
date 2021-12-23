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
#include <QDir>
#include <QUuid>
#include <QFile>
#include <QDebug>

#include <sys/utsname.h>
#include <unistd.h>
#include <fcntl.h>

const QString BLACKLIST_CONF = "/etc/modprobe.d/blacklist-devicemanager.conf";
const QString DEVICEMANAGER = "deepin-devicemanager";

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

QString Utils::machineArch()
{
    struct utsname name;
    uname(&name);
    QString strArch(name.machine);
    qInfo() << name.machine << name.sysname << name.domainname << name.nodename << name.release;

    return  strArch;
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

/**
 * @brief Utils::isDriverPackage 判断Deb是否为驱动包
 * @param filepath 包文件路径
 * @return true: 是 false: 否
 */
bool Utils::isDriverPackage(const QString &filepath)
{
    if (!QFile::exists(filepath)) {
        return  false;
    }
    // 该过滤方法无法设备管理器自己，先添加特殊处理
    if(filepath.contains(DEVICEMANAGER)){
        return false;
    }
    bool bsuccess = false;
    QDir tmpDir = QDir::temp();
    QString tmpPath = QString("devicemanager-%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    if (tmpDir.mkdir(tmpPath)) {
        tmpDir.cd(tmpPath);
        QString strExtract = tmpDir.absolutePath();
        QProcess process;
        process.start("sh", QStringList() << "-c" << QString("dpkg-deb -x '%1' %2").arg(filepath).arg(strExtract));
        if (process.waitForFinished()) {
            //查找关键字 ko insmod modprobe和 路径 /lib/module   // "grep -irHE 'insmod|modprobe|/lib/module' %1  ||"
            process.start("sh", QStringList() << "-c" << QString("grep -irHE '/lib/module' %1 || find %1 -name '*.ko' -o -name '*.ppd'").arg(strExtract));
            if (process.waitForFinished()) {
                //获取查找结果，有结果不为空
                QString strKeyContent = process.readAllStandardOutput();
                qInfo() << strKeyContent;
                if (!strKeyContent.isEmpty()) {
                    bsuccess = true;
                }
            }
        }
        //此处主动调用删除临时文件，临时文件在/tmp目录每次重启会自动清除，所以不对删除结果做处理
        tmpDir.removeRecursively();
    }
    return  bsuccess;
}

bool Utils::updateModDeps(bool bquick)
{
    QProcess process;
    QString strcomd;
    if (bquick) {
        strcomd = QString("depmod %1").arg("--quick");
    } else {
        strcomd = QString("depmod %1").arg("--all");
    }
    process.start(strcomd);
    if (!process.waitForFinished())
        return  false;

    return  true;
}

bool Utils::isFileLocked(const QString &filepath, bool bread)
{
    short int ltype;
    int opentype;
    if (bread) {
        ltype = F_RDLCK;
        opentype = O_RDONLY;
    } else {
        ltype = F_WRLCK;
        opentype = O_WRONLY;
    }
    struct flock fl;
    fl.l_type   = ltype;  /*F_RDLCK read/F_WRLCK write lock */
    fl.l_whence = SEEK_SET; /* beginning of file */
    fl.l_start  = 0;        /* offset from l_whence */
    fl.l_len    = 0;        /* length, 0 = to EOF */
    fl.l_pid    = getpid(); /* PID */

    int fd = open(filepath.toStdString().c_str(), opentype);
    //文件打开失败默认为被锁住
    if (fd < 0) {
        return  true;
    }

    if (-1 == fcntl(fd, F_SETLK, &fl)) {
        //设置失败，文件已被锁住
        qInfo() << __func__ << "file already locked";
        return true;
    }
    //file not locked,set lock success
    fl.l_type   = F_UNLCK;
    //unset lock 必须取消锁定否则其它地方无法获取文件锁
    fcntl(fd, F_SETLK, &fl);
    return  false;
}

bool Utils::isDpkgLocked()
{
    QProcess proc;
    proc.setProgram("bash");
    proc.setArguments(QStringList() << "-c"
                                    << "ps -e -o comm | grep dpkg");
    proc.start();
    proc.waitForFinished();
    QString info = proc.readAllStandardOutput();
    info = info.trimmed();
    if (info.isEmpty() || info == "dpkg-query")
        return false;
    return true;
}


