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

static QMap<QString, QString> mapArch = {
    {"aarch64", "arm64"}
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
Common::SpecialCpuType Common::curCpuType = Common::SpecialCpuType::kUnknowCpuType;
static QString tomlFilesName = "tomlFilesName";
QString Common::getArch()
{
    QString arch;
    struct utsname utsbuf;
    if (-1 != uname(&utsbuf)) {
        arch = QString::fromLocal8Bit(utsbuf.machine);
    }
    return arch;
}

QString Common::getArchStore()
{
    return mapArch[getArch()];
}

static bool getDeviceInfo(QString &deviceInfo, const QString &debugFile)
{
    QString key = debugFile;
    key.replace(".txt", "");
    if (DBusInterface::getInstance()->getInfo(key, deviceInfo))
        return true;

    // deviceInfo 不为空时信息已读取
    if (!deviceInfo.isEmpty())
        return true;

    // 从文件中获取设备信息
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + debugFile);
    if (false == inputDeviceFile.open(QIODevice::ReadOnly))
        return false;

    deviceInfo = inputDeviceFile.readAll();
    inputDeviceFile.close();

    return true;
}

static bool isModeM900(void)
{
    QFile file("/proc/cpuinfo");   // Hardware        : PANGU M900
    if (file.open(QIODevice::ReadOnly)){
        QString cpuInfo = file.readAll();
        file.close();
        if(cpuInfo.contains("Hardware") && cpuInfo.contains("PANGU M900")){
            return true;
        }
    }
    return false;
}
/*   dmidecode | grep -i “String 4”中的值来区分主板类型,PWC30表示PanguW（也就是W525）*/
static bool isModeW525(void)
{
    QString outInfo = Common::executeClientCmd("dmidecode");
    if(outInfo.isEmpty())
        return false;

    // 使用正则表达式进行匹配
    QRegularExpression regex("String 4: (.*)");
    QRegularExpressionMatch match = regex.match(outInfo);
    if (match.hasMatch()) {
        return match.captured(1).contains("PWC30", Qt::CaseInsensitive);  // 返回匹配的内容
    } else {
       return false; // 返回空字符串，表示没有找到
    }
}

static QString readDmidecode11_String4(void)
{
    QString outInfo = Common::executeClientCmd("dmidecode", QStringList() << "-t" << "11");
    if(outInfo.isEmpty())
        return QString("");

    // 使用正则表达式进行匹配
    QRegularExpression regex("String 4: (.*)");
    QRegularExpressionMatch match = regex.match(outInfo);
    if (match.hasMatch()) {
        return match.captured(1).trimmed(); // 返回匹配的内容
    } else {
        return QString(); // 返回空字符串，表示没有找到
    }
}

QString Common::checkBoardVendorFlag()
{
    if(specialComType != -1){
        switch (specialComType) {
        case NormalCom:
            boardVendorKey = "";
            break;
        case kSpecialType1:
            boardVendorKey = "PGUW";
            break;
        case kSpecialType2:
            boardVendorKey = "KLVV";
            break;
        case kSpecialType3:
            boardVendorKey = "KLVU";
            break;
        case kSpecialType4:
            boardVendorKey = "PGUV";
            break;
        case kSpecialType5:
            boardVendorKey = "PGUX";
            break;
        case kSpecialType8:
            boardVendorKey = "CustomType";
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

void Common::tomlFilesNameSet(const QString &name)
{
    tomlFilesName = name;
}

bool Common::isHwPlatform()
{
    if (specialComType == Unknow || specialComType == NormalCom || specialComType == kSpecialType8)
        return false;
    return true;
}

QString Common::tomlFilesNameGet()
{
    return tomlFilesName;
}

QByteArray Common::executeClientCmd(const QString &cmd, const QStringList &args, const QString &workPath, int msecsWaiting/* = 30000*/)
{
    QProcess process;
    if (!workPath.isEmpty())
        process.setWorkingDirectory(workPath);

    process.setProgram(cmd);
    process.setArguments(args);
    process.setEnvironment({"LANG=en_US.UTF-8", "LANGUAGE=en_US"});
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(msecsWaiting);
    QByteArray outPut = process.readAllStandardOutput();
    int nExitCode = process.exitCode();
    bool bRet = (process.exitStatus() == QProcess::NormalExit && nExitCode == 0);
    if (!bRet) {
        qCWarning(appLog) << "run cmd error, caused by:" << process.errorString() << "output:" << outPut;
        return QByteArray();
    }
    return outPut;
}
