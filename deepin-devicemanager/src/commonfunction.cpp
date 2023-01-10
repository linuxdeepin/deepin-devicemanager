// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonfunction.h"
#include "commondefine.h"
#include "DBusInterface.h"

// 其它头文件
#include <QString>
#include <QMap>
#include <QProcess>
#include <QFile>

#include <sys/utsname.h>

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

QString Common::checkBoardVendorFlag()
{
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
    } else if (info.contains("PGUV", Qt::CaseInsensitive)) {
        boardVendorKey = "PGUV";
    } else if (info.contains("PGUW", Qt::CaseInsensitive)) {
        boardVendorKey = "PGUW";
    }
    process.close();

    initBoardVendorFlag = true;
    return boardVendorKey;
}

QString Common::boardVendorType()
{
    return initBoardVendorFlag ? boardVendorKey : checkBoardVendorFlag();
}
