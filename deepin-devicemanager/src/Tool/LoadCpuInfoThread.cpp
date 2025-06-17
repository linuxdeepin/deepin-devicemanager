// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "LoadCpuInfoThread.h"
#include "DDLog.h"

#include <QProcess>
#include <QLoggingCategory>

#include "DeviceManager.h"
#include "DeviceCpu.h"

using namespace DDLog;

LoadCpuInfoThread::LoadCpuInfoThread()
{
    qCDebug(appLog) << "LoadCpuInfoThread constructor called";
}

void LoadCpuInfoThread::run()
{
    qCDebug(appLog) << "Starting CPU info loading thread";
    getCpuInfoFromLscpu();
}

void LoadCpuInfoThread::runCmd(QString &info, const QString &cmd)
{
    qCDebug(appLog) << "Executing command:" << cmd;

    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void LoadCpuInfoThread::loadCpuInfo(QMap<QString, QString> &mapInfo, const QString &cmd)
{
    qCDebug(appLog) << "Loading CPU info with command:" << cmd;

    QString cpuInfo;
    runCmd(cpuInfo, cmd);
    getMapInfoFromCmd(cpuInfo, mapInfo, ": ");
}

void LoadCpuInfoThread::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    qCDebug(appLog) << "Parsing command output with separator:" << ch;

    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2)
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }
}

void LoadCpuInfoThread::getCpuInfoFromLscpu()
{
    qCDebug(appLog) << "Getting CPU info from lscpu";

    // 生成CPU
    const QList<QMap<QString, QString>> &lstCatCpu = DeviceManager::instance()->cmdInfo("lscpu");
    if (lstCatCpu.size() == 0) {
        qCWarning(appLog) << "No CPU info found in cmdInfo";
        return;
    }
    QMap<QString, QString> mapInfo;
    loadCpuInfo(mapInfo, "lscpu");
    DeviceManager::instance()->setCpuRefreshInfoFromlscpu(mapInfo);
}
