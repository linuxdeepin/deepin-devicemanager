/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QThread>
#include <iostream>
#include <QDateTime>
#include "qmath.h"

//for avodi deepin system pkexec interface bug
//绕开获取权限窗口为显卡字样 的bug


void getPKStr(QString &dtStr, QString &dtInt, const QString &cStr)
{
    QString value4 = cStr.mid(0, 4);
    QString dayStr = QString("%1").arg(cStr.mid(4, 2).toInt() / 3, 2, 10, QLatin1Char('0'));
    QString value2 = cStr.mid(6, 2);
    QString secondStr = QString("%1").arg(cStr.mid(8, 2).toInt(), 2, 10, QLatin1Char('0'));
    QString value1 = cStr.mid(10, 1);
    QString hourStr = QString("%1").arg(cStr.mid(11, 2).toInt() / 4, 2, 10, QLatin1Char('0'));
    QString value3 = cStr.mid(13, 3);
    QString monthStr = QString("%1").arg(cStr.mid(16, 2).toInt() / 7, 2, 10, QLatin1Char('0'));
    QString yearStr = QString("%1").arg(cStr.mid(18, 4).toInt() + 253, 4, 10, QLatin1Char('0'));
    QString minuStr = cStr.mid(22, 2);

    QString value5 = cStr.mid(24);
    QString extraS = value5.right(5);
    value5.replace(extraS, "");

    int extraInt = extraS.toInt();
    if (extraInt % 3 != 0) {
        dtStr = "111";
        return;
    }

    dtStr = QString("%1:%2:%3:%4:%5:%6").arg(yearStr, monthStr, dayStr, hourStr, minuStr, secondStr);
    dtInt = QString("%1%2%3%4%5").arg(value1).arg(value2).arg(value3).arg(value4).arg(value5);
}

int main(int argc, char *argv[])
{
    if ( argc < 3 ||
            QString(argv[1]) == "--help" || \
            QString(argv[1]) == "--h" || \
            QString(argv[1]) == "-h") {
        std::cout << "deepin-devicemanager-authenticateProxy is a Proxy for replace polkit gui interface!" << std::endl;
        std::cout << "Usage: " << std::endl;
        std::cout << "\tdeepin-devicemanager-authenticateProxy --help(--h): " << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy [cmd]" << std::endl;

        std::cout << "Examples: " << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy ls -l" << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy dmidecode" << std::endl;

        return 0;
    }

    QString dtStr, dtInt;
    QString key = QString(argv[2]);
    getPKStr(dtStr, dtInt, key);
    qint64 msFromInt = dtInt.toLongLong();

    QDateTime dtFromInt = QDateTime::fromMSecsSinceEpoch(msFromInt);
    QDateTime dtCur = QDateTime::currentDateTime();

    qint64 sFromInt = dtFromInt.toMSecsSinceEpoch();
    qint64 sCur = dtCur.toMSecsSinceEpoch();

    QString dtStrFromInt = dtFromInt.toString("yyyy:MM:dd:hh:mm:ss");
    if (dtStr != dtStrFromInt || sCur - sFromInt < 0 || sCur - sFromInt > 1000) {
        std::cout << "***deepin-devicemanager-authenticateProxy is a Proxy for replace polkit gui interface!" << std::endl;
        return 0;
    }



    QString defaultLanguage = getenv("LANGUAGE");
    setenv ("LANGUAGE", "en_US", 1);    //for aviod translate in lscpu...

    QProcess proc;

    proc.start( argv[1] );
    bool res = proc.waitForFinished(-1);

    std::cout << proc.readAllStandardOutput().data();

    proc.close();

    if (res == false) {
        setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
        return -1;
    }

    setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
    return 0;
}
