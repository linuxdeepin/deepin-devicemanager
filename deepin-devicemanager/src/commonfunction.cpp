// Copyright (C) 2019 - 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonfunction.h"
#include "commondefine.h"
#include "DBusInterface.h"
#include "DDLog.h"

#include <QRegularExpression>

// 其它头文件
#include <QString>
#include <QMap>
#include <QProcess>
#include <QFile>
#include <QLoggingCategory>
#include <QRegularExpression>

#include <sys/utsname.h>
#include <cmath>

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
Common::SpecialVRAMType Common::curVRAMType = Common::SpecialVRAMType::kNormalVRAMType;
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

bool Common::isShowScreenSize()
{
    bool showScreenSize { true };
#ifdef DTKCORE_CLASS_DConfigFile
    DConfig *dconfig = DConfig::create("org.deepin.devicemanager","org.deepin.devicemanager");
    if(dconfig && dconfig->isValid() && dconfig->keyList().contains("showScreenSize")){
        showScreenSize = dconfig->value("showScreenSize").toBool();
    }
#endif
    return showScreenSize;
}

QString Common::formatTotalCache(const QString &perThreadCache, int coreCount)
{
    // 1. 清理并分离数字与单位
    QString s = perThreadCache.trimmed();
    if (s.isEmpty())
        return QString();

    int i = s.length() - 1;
    while (i >= 0 && !s[i].isDigit() && s[i] != '.')
        --i;

    QString numStr = s.left(i + 1);
    QString unitStr = s.mid(i + 1).toUpper();

    bool ok;
    double num = numStr.toDouble(&ok);
    if (!ok)
        return QString();

    // 2. 将单核/单线程缓存转换为 KiB
    double perCoreKiB = 0.0;
    if (unitStr.startsWith("K") || unitStr == "KB" || unitStr == "KIB") {
        perCoreKiB = num;
    } else if (unitStr.startsWith("M") || unitStr == "MB" || unitStr == "MIB") {
        perCoreKiB = num * 1024.0;
    } else if (unitStr.startsWith("G") || unitStr == "GB" || unitStr == "GIB") {
        perCoreKiB = num * 1024.0 * 1024.0;
    } else if (unitStr.startsWith("T") || unitStr == "TB" || unitStr == "TIB") {
        perCoreKiB = num * 1024.0 * 1024.0 * 1024.0;
    } else if (unitStr.isEmpty() || unitStr == "B") {
        // 无单位或纯字节，视为字节并转为 KiB
        perCoreKiB = num / 1024.0;
    } else {
        // 未知单位，按原数值当作 KiB 处理
        perCoreKiB = num;
    }

    double totalKiB = perCoreKiB * coreCount;

    // 3. 选择最合适的单位并格式化数值
    double value;
    QString unit;
    if (totalKiB >= 1024.0 * 1024.0) {
        value = totalKiB / (1024.0 * 1024.0);
        unit = "GiB";
    } else if (totalKiB >= 1024.0) {
        value = totalKiB / 1024.0;
        unit = "MiB";
    } else {
        value = totalKiB;
        unit = "KiB";
    }

    // 4. 数值显示：若为整数则无小数位，否则保留一位小数
    double intPart;
    if (std::abs(std::modf(value, &intPart)) < 1e-6) {
        return QString::number(static_cast<long long>(value)) + " " + unit;
    } else {
        return QString::number(value, 'f', 1) + " " + unit;
    }
}

int Common::parseSharedCpuCount(const QString &sharedCpuList)
{
    QString s = sharedCpuList.trimmed();
    if (s.isEmpty())
        return 0;

    int count = 0;
    QStringList parts = s.split(',', QString::SkipEmptyParts);
    for (const QString &part : parts) {
        QString trimmed = part.trimmed();
        if (trimmed.contains('-')) {
            QStringList range = trimmed.split('-');
            if (range.size() == 2) {
                bool ok1 = false, ok2 = false;
                int start = range[0].trimmed().toInt(&ok1);
                int end = range[1].trimmed().toInt(&ok2);
                if (ok1 && ok2 && end >= start) {
                    count += end - start + 1;
                }
            }
        } else {
            bool ok = false;
            if (trimmed.toInt(&ok) || ok) {
                count += 1;
            }
        }
    }
    return count;
}

QString Common::formatNetworkSpeed(const QString& speed)
{
    // 1. Trim whitespace; return original if empty
    QString s = speed.trimmed();
    if (s.isEmpty())
        return speed;

    // 2. Convert "Gbit/s" -> "Mbps" (1Gbit/s = 1000Mbps), case-sensitive
    const QString gbitSuffix = "Gbit/s";
    if (s.endsWith(gbitSuffix, Qt::CaseSensitive)) {
        QString numStr = s.left(s.length() - gbitSuffix.length()).trimmed();
        bool ok = false;
        double num = numStr.toDouble(&ok);
        if (ok) {
            double mbps = num * 1000.0;
            // Format: integer values get no decimal point, otherwise keep one decimal
            double intPart;
            if (std::abs(std::modf(mbps, &intPart)) < 1e-6) {
                return QString::number(static_cast<long long>(mbps)) + "Mbps";
            } else {
                return QString::number(mbps, 'f', 1) + "Mbps";
            }
        }
    }

    // 3. Normalize "Mbit/s" -> "Mbps" (value unchanged, case-sensitive)
    const QString mbitSuffix = "Mbit/s";
    if (s.endsWith(mbitSuffix, Qt::CaseSensitive)) {
        QString numStr = s.left(s.length() - mbitSuffix.length()).trimmed();
        bool ok = false;
        double num = numStr.toDouble(&ok);
        if (ok) {
            // Format: integer values get no decimal point, otherwise keep one decimal
            double intPart;
            if (std::abs(std::modf(num, &intPart)) < 1e-6) {
                return QString::number(static_cast<long long>(num)) + "Mbps";
            } else {
                return QString::number(num, 'f', 1) + "Mbps";
            }
        }
    }

    // 4. Return original string if no known suffix matches
    return speed;
}
