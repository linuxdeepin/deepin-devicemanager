/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Libang.Liu <liulibang@uniontech.com>
*
* Maintainer: Libang.Liu <liulibang@uniontech.com>
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

#ifndef COMMONFUNCTION_H
#define COMMONFUNCTION_H

// 其它头文件
#include <QString>
#include <QMap>
#include <QProcess>

#include <sys/utsname.h>

static QMap<QString, QString> mapArch = {   {"aarch64"    , "arm64"}
                                      ,{"x86_64"     , "amd64"}
                                      ,{"mips64"     , "mips64el"}
                                      ,{"i386"       , "i386"}
                                      ,{"sw_64"      , "sw_64"}
                                      ,{"loongarch"  , "loongarch"}
                                      ,{"loongarch64", "loongarch64"}};

static bool initBoardVendorFlag = false;
static bool boardVendorFlag = false;

class Common
{
public:
    Common();
    ~Common();
    static QString getArch()
    {
        QString arch;
        struct utsname utsbuf;
        if (-1 != uname(&utsbuf)){
            arch = QString::fromLocal8Bit(utsbuf.machine);
        }
        return arch;
    }

    static QString getArchStore()
    {
        return mapArch[getArch()];
    }

    static bool checkBoardVendorFlag()
    {
        QProcess process;
        process.start("dmidecode",QStringList()<< "-s"<<"system-product-name");
        process.waitForFinished(-1);
        QString info = process.readAllStandardOutput();
        boardVendorFlag = info.contains("KLVV") || info.contains("KLVU") || info.contains("PGUV") || info.contains("PGUW");
        process.close();

        initBoardVendorFlag = true;
        return boardVendorFlag;
    }

    static bool boardVendorType()
    {
        return initBoardVendorFlag ? boardVendorFlag : checkBoardVendorFlag();
    }
};
#endif // COMMONFUNCTION_H
