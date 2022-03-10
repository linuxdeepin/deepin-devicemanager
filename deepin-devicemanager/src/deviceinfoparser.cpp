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

#include "deviceinfoparser.h"
#include <QObject>
#include <sys/utsname.h>
#include <iostream>
#include <QFile>
#include <QProcess>
#include <QRegExp>
#include <DLog>
#include <com_deepin_daemon_power.h>
#include "commondefine.h"
#include "deviceattributedefine.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMouse.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceKeyboard.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"
#include <DApplication>
#include <DSysInfo>
//#include "logpasswordauth.h"
#include "DMessageBox"
#include <cups.h>
#include "Logger.h"


DWIDGET_USE_NAMESPACE

using PowerInter = com::deepin::daemon::Power;

DCORE_USE_NAMESPACE

DeviceInfoParser::DeviceInfoParser()
    : QObject()
{

}

DeviceInfoParser::~DeviceInfoParser()
{

}

QString DeviceInfoParser::loadGeneratorKey()
{
    // 获取设备信息
    QString key = "";
    QString deviceInfo;
    // According to Huawei's requirements , Modify the way of judging klu and panguv
    if (!getDeviceInfo(QString("gdbus introspect -y -d com.deepin.system.SystemInfo -o /com/deepin/system/SystemInfo -p"), deviceInfo, "gdbus.txt")) {
        return key;
    }
    if (deviceInfo.contains("klu")) { // klu 华为确认将判断条件改为L410 KLVU-WDU0
        key = "KLU";
    } else if (deviceInfo.contains("panguV")) { // panguv
        key = "PanGuV";
    }
    return key;
}


bool DeviceInfoParser::getRootPassword()
{
    QString deviceInfo;
    bool res = runCmd("id -un", deviceInfo); // file path is fixed. So write cmd direct
    if (res == true && standOutput_.trimmed() == "root") {
        return true;
    }

#ifdef TEST_DATA_FROM_FILE
    return true;
#endif
    if (false == executeProcess("sudo whoami", deviceInfo)) {
        return false;
    }

    return true;
}

bool DeviceInfoParser::getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile)
{
    qDebug() << debugFile;
    if (!deviceInfo.isEmpty()) {
        return true;
    }
    if (false == executeProcess(command, deviceInfo)) {
        return false;
    }
#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();
#endif

    return true;
}

bool DeviceInfoParser::executeProcess(const QString &cmd, QString &deviceInfo)
{
#ifdef TEST_DATA_FROM_FILE
    return true;
#else
    if (false == cmd.startsWith("sudo")) {
        return runCmd(cmd, deviceInfo);
    }

    runCmd("id -un", deviceInfo);
    if (deviceInfo.trimmed() == "root") {
        return runCmd(cmd, deviceInfo);
    }

    QString newCmd = "pkexec deepin-devicemanager-authenticateProxy \"";
    newCmd += cmd;
    newCmd += "\"";
    newCmd.remove("sudo");
    return runCmd(newCmd, deviceInfo);
#endif
}

bool DeviceInfoParser::runCmd(const QString &proxy, QString &deviceInfo)
{
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyy:MM:dd:hh:mm:ss");
    QString dtInt = QString::number(dt.toMSecsSinceEpoch());
    QString key = getPKStr(dtStr, dtInt);

//    用于测试
//    QString str1, str2;
//    getPKStr(str1, str2, key);

    QString cmd = proxy;
    QProcess process_;
    int msecs = 10000;
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy")) {
        cmd = proxy + QString(" ") + key;
        msecs = -1;
    }

    process_.start(cmd);

    bool res = process_.waitForFinished(msecs);
    deviceInfo = process_.readAllStandardOutput();
    int exitCode = process_.exitCode();
    if (cmd.startsWith("pkexec deepin-devicemanager-authenticateProxy") && (exitCode == 127 || exitCode == 126)) {
        //dError("Run \'" + cmd + "\' failed: Password Error! " + QString::number(exitCode) + "\n");
        return false;
    }

    if (res == false) {
        //dError("Run \'" + cmd + "\' failed\n");
    }

    return res;
}

QString DeviceInfoParser::getPKStr(const QString &dtStr, const QString &dtInt)
{
    QString res = "";
    QString str = dtStr;
    str.replace(":", "");

    int year = str.mid(0, 4).toInt() - 253;
    int month = str.mid(4, 2).toInt() * 7;
    int day = str.mid(6, 2).toInt() * 3;
    int hour = str.mid(8, 2).toInt() * 4;
    int minus = str.mid(10, 2).toInt();
    int second = str.mid(12, 2).toInt();

    QString yearStr = QString("%1").arg(year, 4, 10, QLatin1Char('0'));
    QString monthStr = QString("%1").arg(month, 2, 10, QLatin1Char('0'));
    QString dayStr = QString("%1").arg(day, 2, 10, QLatin1Char('0'));
    QString hourStr = QString("%1").arg(hour, 2, 10, QLatin1Char('0'));
    QString minusStr = QString("%1").arg(minus, 2, 10, QLatin1Char('0'));
    QString secondStr = QString("%1").arg(second, 2, 10, QLatin1Char('0'));

    str = dtInt;
    QString value1 = str.mid(0, 1);
    QString value2 = str.mid(1, 2);
    QString value3 = str.mid(3, 3);
    QString value4 = str.mid(6, 4);
    QString value5 = str.mid(10);

    QTime time = QTime::currentTime();
    qsrand(uint(time.msec()) + uint(time.second()) * 1000);
    int random = (qrand() % 10000 + 10000) * 3;  //产生随机数
    QString randomS = QString::number(random);

    QString newDtStr = QString("%1%2%3%4%5%6%7%8%9%10%11%12").arg(value4).arg(dayStr).arg(value2).arg(secondStr).arg(value1).arg(hourStr).arg(value3).arg(monthStr).arg(yearStr).arg(minusStr).arg(value5).arg(randomS);

    return newDtStr;
}

void DeviceInfoParser::getPKStr(QString &dtStr, QString &dtInt, const QString &cStr)
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
