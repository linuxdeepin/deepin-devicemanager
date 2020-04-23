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

//for avodi deepin system pkexec interface bug
//绕开获取权限窗口为显卡字样 的bug

int main(int argc, char *argv[])
{
    if ( argc < 3 ||
        QString(argv[1]) == "--help" || \
        QString(argv[1]) == "--h" ||\
        QString(argv[1]) == "-h")
    {
        std::cout << "deepin-devicemanager-authenticateProxy is a Proxy for replace polkit gui interface!" << std::endl;
        std::cout << "Usage: " << std::endl;
        std::cout << "\tdeepin-devicemanager-authenticateProxy --help(--h): " << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy [cmd]" << std::endl;

        std::cout << "Examples: " << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy ls -l" << std::endl;
        std::cout << "\tpkexec deepin-devicemanager-authenticateProxy dmidecode" << std::endl;

        return 0;
    }

    QString key = "eyJsaWNlbnNlSWQiOiJRWVlCQUM5RDNKIiwibGljZW5zZWVOYW1lIjoi6LaF57qnIOeoi+W6j+WRmCIsImFzc2lnbmVlTmFtZSI6IiIsImFzc2lnbmVlRW1haWwiOiIiLCJsaWNlbnNlUmVzdHJpY3Rpb24iOiIiLCJjaGVja0NvbmN1cnJlbnRVc2UiOmZhbHNlLCJwcm9kdWN0cyI6W3siY29kZSI6IklJIiwiZmFsbGJhY2tEYXRlIjoiMjAyMC0wMS0wNCIsInBhaWRVcFRvIjoiMjAyMS0wMS0wMyJ9LHsiY29kZSI6IkFDIiwiZmFsbGJhY2tEYXRlIjoiMjAyMC0wMS0wNCIsInBhaWRVcFRvIjoiMjAyMS0wMS0wMyJ9LHsiY29kZSI6IkRQTiIsImZhbGxiYWNrRGF0ZSI6IjIwMjAtMDEtMDQiLCJwYWlkVXBUbyI6IjIwMjEtMDEtMDMifSx7ImNvZGUiOiJQUyIsImZhbGxiYWNrRGF0ZSI6IjIwMjAtMDEtMDQiLCJwYWlkVXBUbyI6IjIwMjEtMDEtMDMifSx7ImNvZGUiOiJHTyIsImZhbGxiYWNrRGF0ZSI6IjIwMjAtMDEtMDQiLCJwYWlkVXBUbyI6IjIwMjEtMDEtMDMifSx7ImNvZGUiOiJETSIsImZhbGxiYWNrRGF0ZSI6IjIwMjAtMDEtMDQiLCJwYWlkVXBUbyI6IjIwMjEtMDEtMDMifSx7ImNvZGUiOiJDTCIsImZhbGxiYWNrRGF0ZSI6IjIwMjAtMDEtMDQiLCJwYWlkVXBUbyI6IjIwMjEtMDEtMDMifSx7ImNvZGUiOiJSUzAiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiUkMiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiUkQiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiUEMiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiUk0iLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiV1MiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiREIiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiREMiLCJmYWxsYmFja0RhdGUiOiIyMDIwLTAxLTA0IiwicGFpZFVwVG8iOiIyMDIxLTAxLTAzIn0seyJjb2RlIjoiUlNVIiwiZmFsbGJhY2tEYXRlIjoiMjAyMC0wMS0wNCIsInBhaWRVcFRvIjoiMjAyMS0wMS0wMyJ9XSwiaGFzaCI6IjE2MDgwOTA5LzAiLCJncmFjZVBlcmlvZERheXMiOjcsImF1dG9Qcm9sb25nYXRlZCI6ZmFsc2UsImlzQXV0b1Byb2xvbmdhdGVkIjpmYWxzZX0";
    if(QString(argv[2]) != key){
        std::cout << "deepin-devicemanager-authenticateProxy is a Proxy for replace polkit gui interface!" << std::endl;
        return 0;
    }

    QString defaultLanguage = getenv("LANGUAGE");
    setenv ("LANGUAGE", "en_US", 1);    //for aviod translate in lscpu...

    QProcess proc;

    proc.start( argv[1] );
    bool res = proc.waitForFinished(-1);

    std::cout << proc.readAllStandardOutput().data();

    proc.close();

    if(res == false)
    {
        setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
        return -1;
    }

    setenv ("LANGUAGE", defaultLanguage.toStdString().c_str(), 1);
    return 0;
}
