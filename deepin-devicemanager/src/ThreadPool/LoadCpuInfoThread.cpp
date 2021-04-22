/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "LoadCpuInfoThread.h"

#include <QProcess>
#include <QDebug>

#include "DeviceManager.h"
#include "DeviceCpu.h"

LoadCpuInfoThread::LoadCpuInfoThread()
{
}

void LoadCpuInfoThread::run()
{
    getCpuInfoFromLscpu();
}

void LoadCpuInfoThread::runCmd(QString &info, const QString &cmd)
{
    QProcess process;
    process.start(cmd);
    process.waitForFinished(-1);
    info = process.readAllStandardOutput();
}

void LoadCpuInfoThread::loadCpuInfo(QList<QMap<QString, QString>> &lstMap, const QString &cmd)
{
    QString cpuInfo;
    runCmd(cpuInfo, cmd);
    QStringList items = cpuInfo.split("\n");
    //    qInfo() << items;
    foreach (const QString &item, items) {
        if (item.isEmpty())
            continue;
        QMap<QString, QString> mapInfo;

        getMapInfoFromCmd(item, mapInfo, ":");
        lstMap.append(mapInfo);
    }
}

void LoadCpuInfoThread::getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch)
{
    QStringList infoList = info.split("\n");
    for (QStringList::iterator it = infoList.begin(); it != infoList.end(); ++it) {
        QStringList words = (*it).split(ch);
        if (words.size() == 2)
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
    }
}

void LoadCpuInfoThread::getCpuInfoFromLscpu()
{
    // 生成CPU
    const QList<QMap<QString, QString>> &lstCatCpu = DeviceManager::instance()->cmdInfo("cat_cpuinfo");
    if (lstCatCpu.size() == 0)
        return;
    QList<QMap<QString, QString>> lstMap;
    loadCpuInfo(lstMap, "lscpu");
    QList<QMap<QString, QString>>::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1)
            continue;
        DeviceManager::instance()->setCpuRefreshInfoFromlscpu(*it);
    }
}
