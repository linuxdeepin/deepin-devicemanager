// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonfunction.h"
#include "commondefine.h"
#include "DBusInterface.h"
#include "DDLog.h"

// 其它头文件
#include <QString>
#include <QMap>
#include <QProcess>
#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>

#include <sys/utsname.h>

using namespace DDLog;

static QMap<QString, QString> mapArch = {   {"aarch64", "arm64"}
    , {"x86_64", "amd64"}
    , {"mips64", "mips64el"}
    , {"i386", "i386"}
    , {"sw_64", "sw_64"}
    , {"loongarch", "loongarch"}
    , {"loongarch64", "loongarch64"}
};

static bool initBoardVendorFlag = false;
static QString boardVendorKey = "";
int Common::specialComType = -1;

QString Common::getArch()
{
    qCDebug(appLog) << "Getting system architecture";

    QString arch;
    struct utsname utsbuf;
    if (-1 != uname(&utsbuf)) {
        arch = QString::fromLocal8Bit(utsbuf.machine);
    }
    return arch;
}

QString Common::getArchStore()
{
    qCDebug(appLog) << "Getting architecture store mapping";

    return mapArch[getArch()];
}

static bool getDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    qCDebug(appLog) << "Getting device info for" << debugFile;
    QString key = debugFile;
    key.replace(".txt", "");
    if (DBusInterface::getInstance()->getInfo(key, deviceInfo)) {
        qCDebug(appLog) << "Successfully get device info from D-Bus for" << key;
        return true;
    }

    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty()) {
        qCDebug(appLog) << "Device info already exists for" << key;
        return true;
    }

    // 从文件中获取设备信息
    qCDebug(appLog) << "Getting device info from file for" << debugFile;
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly)) {
        qCWarning(appLog) << "Failed to open device info file:" << inputDeviceFile.fileName();
        return false;
    }

    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();

    qCDebug(appLog) << "Successfully get device info from file for" << debugFile;
    return true;
}

static bool isModeM900(void)
{
    qCDebug(appLog) << "Checking if mode is M900";
    QFile file("/proc/cpuinfo");   // Hardware        : PANGU M900
    if (file.open(QIODevice::ReadOnly)){
        QString cpuInfo = file.readAll();
        file.close();
        if(cpuInfo.contains("Hardware") && cpuInfo.contains("PANGU M900")){
            qCDebug(appLog) << "Mode is M900";
            return true;
        }
    } else {
        qCWarning(appLog) << "Failed to open /proc/cpuinfo";
    }
    qCDebug(appLog) << "Mode is not M900";
    return false;
}
/*   dmidecode | grep -i "String 4"中的值来区分主板类型,PWC30表示PanguW（也就是W525）*/
static bool isModeW525(void)
{
    qCDebug(appLog) << "Checking if mode is W525";
    QString outInfo = Common::executeClientCmd("dmidecode");
    if(outInfo.isEmpty()) {
        qCWarning(appLog) << "dmidecode command output is empty";
        return false;
    }

    // 使用正则表达式进行匹配
    QRegularExpression regex("String 4: (.*)");
    QRegularExpressionMatch match = regex.match(outInfo);
    if (match.hasMatch()) {
        bool contains = match.captured(1).contains("PWC30", Qt::CaseInsensitive);
        qCDebug(appLog) << "Mode is W525:" << contains;
        return contains;  // 返回匹配的内容
    } else {
        qCDebug(appLog) << "No match found for W525 mode";
       return false; // 返回空字符串，表示没有找到
    }
}

static QString readDmidecode11_String4(void)
{
    qCDebug(appLog) << "Reading dmidecode -t 11 String 4";
    QString outInfo = Common::executeClientCmd("dmidecode", QStringList() << "-t" << "11");
    if(outInfo.isEmpty()) {
        qCWarning(appLog) << "dmidecode -t 11 output is empty";
        return QString("");
    }

    // 使用正则表达式进行匹配
    QRegularExpression regex("String 4: (.*)");
    QRegularExpressionMatch match = regex.match(outInfo);
    if (match.hasMatch()) {
        QString result = match.captured(1).trimmed();
        qCDebug(appLog) << "Found String 4:" << result;
        return result; // 返回匹配的内容
    } else {
        qCDebug(appLog) << "No String 4 found in dmidecode -t 11 output";
        return QString(); // 返回空字符串，表示没有找到
    }
}

QString Common::checkBoardVendorFlag()
{
    qCDebug(appLog) << "Checking board vendor flag, specialComType:" << specialComType;
    if(specialComType != -1){
        switch (specialComType) {
        case NormalCom:
            boardVendorKey = "";
            break;
        case PGUW:
            boardVendorKey = "PGUW";
            break;
        case KLVV:
            boardVendorKey = "KLVV";
            break;
        case KLVU:
            boardVendorKey = "KLVU";
            break;
        case PGUV:
            boardVendorKey = "PGUV";
            break;
        case PGUX:
            boardVendorKey = "PGUX";
            break;
        default:
            boardVendorKey = "PGUW";
            break;
        }
    }else{
        QProcess process;
        process.start("dmidecode", QStringList() << "-s" << "system-product-name");
        process.waitForFinished(-1);
        QString info = process.readAllStandardOutput();
        if (info.isEmpty()) {
            qCDebug(appLog) << "dmidecode output is empty, getting info from file";
            getDeviceInfo(info, "dmidecode_spn.txt");
        }
        if (info.contains("KLVV", Qt::CaseInsensitive) || info.contains("L540", Qt::CaseInsensitive)) {
            boardVendorKey = "KLVV";
        } else if (info.contains("KLVU", Qt::CaseInsensitive)) {
            boardVendorKey = "KLVU";
        } else if (info.contains("PGUV", Qt::CaseInsensitive) || info.contains("W585", Qt::CaseInsensitive)) {
            boardVendorKey = "PGUV";
        } else if (info.contains("PGUW", Qt::CaseInsensitive)) {
            boardVendorKey = "PGUW";
        } else if (readDmidecode11_String4().contains("PGUX", Qt::CaseInsensitive)) {
            boardVendorKey = "PGUX";
        }
        process.close();

        if(boardVendorKey.isEmpty() && (isModeM900() || isModeW525())){
            qCDebug(appLog) << "Board vendor key is empty, checking for M900 or W525 mode";
            boardVendorKey = "PGUW";
        }
        qCInfo(appLog) << "boardVendorKey:" <<  boardVendorKey;
    }
    qCInfo(appLog) << "Current special computer type is " << boardVendorKey;
    initBoardVendorFlag = true;
    return boardVendorKey;
}

QString Common::boardVendorType()
{
    qCDebug(appLog) << "Getting board vendor type, initBoardVendorFlag:" << initBoardVendorFlag;
    return initBoardVendorFlag ? boardVendorKey : checkBoardVendorFlag();
}

QString Common::specialVendorType()
{
    QString type = Common::boardVendorType();
    if (type == "KLVV" || type == "KLVU" || type == "PGUV" || type == "PGUW")
        return specialHString();
    return QString("normalmagical");
}

QString Common::specialHString()
{
    int asciiValues[] = {72, 85, 65, 87, 69, 73}; // ASCII values for 'H' .... 'I'
    QString result;

    // Convert ASCII values to characters and append to QString
    for (int ascii : asciiValues) {
        result.append(QChar(ascii)); // Convert ASCII to QChar and append to QString
    }
    return result;
}

QByteArray Common::executeClientCmd(const QString &cmd, const QStringList &args, const QString &workPath, int msecsWaiting/* = 30000*/, bool useEnv/* = true*/)
{
    qCDebug(appLog) << "Executing command:" << cmd << "args:" << args;

    QProcess process;
    if (!workPath.isEmpty()) {
        qCDebug(appLog) << "Setting working directory to" << workPath;
        process.setWorkingDirectory(workPath);
    }

    if (useEnv) {
        qCDebug(appLog) << "Setting process environment";
        QStringList env = QProcess::systemEnvironment();
        bool hasLang = false;
        bool hasLanguage = false;
        for (const QString &e : env) {
            if (e.startsWith("LANG=")) {
                hasLang = true;
            } else if (e.startsWith("LANGUAGE=")) {
                hasLanguage = true;
            }
        }
        if (hasLang) {
            env.replaceInStrings(QRegularExpression("^LANG=.*"), "LANG=en_US.UTF-8");
        } else {
            env.append("LANG=en_US.UTF-8");
        }
        if (hasLanguage) {
            env.replaceInStrings(QRegularExpression("^LANGUAGE=.*"), "LANGUAGE=en_US");
        } else {
            env.append("LANGUAGE=en_US");
        }
        process.setEnvironment(env);
    }
    process.start(cmd, args);
    // Wait for process to finish without timeout.
    process.waitForFinished(msecsWaiting);
    QByteArray outPut = process.readAllStandardOutput();
    int nExitCode = process.exitCode();
    qCDebug(appLog) << "Command execution completed, exit code:" << nExitCode;
    bool bRet = (process.exitStatus() == QProcess::NormalExit && nExitCode == 0);
    if (!bRet) {
        qCWarning(appLog) << "run cmd error, caused by:" << process.errorString() << "output:" << outPut;
        return QByteArray();
    }
    return outPut;
}
