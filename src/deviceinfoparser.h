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

#pragma once

#include <QMap>
#include <QString>
#include "singletondef.h"
#include "deviceattributedefine.h"
#include"DeviceManager/DeviceInfo.h"
#include <QObject>
#include <QSet>

class QDeviceMouse;

class DeviceInfoParser: public QObject
{
    Q_OBJECT
public:

    static DeviceInfoParser &Instance()
    {
        static DeviceInfoParser _instance;
        return _instance;
    }
    DeviceInfoParser();
    ~DeviceInfoParser();


private:

    /**
     * @brief:获取主板信息，获取主板信息的命令有
     *      sudo dmidecode -t 0  // BIOS
     *      lspci | grep ISA
     * @brief[loadBiosInfoFromDmidecode]:从 sudo dmidecode -t 0 中获取信息
     * @brief[loadBiosInfoFromLspci]:从 lspci 中获取信息,用于获取芯片组，但是龙芯的不是在这获取
     * @brief[loadBiosInfoFromCatBoardinfo]:从 cat /proc/boardinfo 中获取信息  这个仅仅用龙芯机器
     */
    void loadBiosInfoFromLspci(QString &chipsetFamliy);
    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));

public:
    bool getRootPassword();
    bool executeProcess(const QString &cmd);
    bool runCmd(const QString &cmd);
    bool runCmd(const QStringList &cmdList);

    QString standOutput_;
};

