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
//#include "singletondef.h"
//#include "deviceattributedefine.h"
//#include"DeviceManager/DeviceInfo.h"
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


//    /**
//     * @brief:命令的执行
//     */
    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));
    bool executeProcess(const QString &cmd, QString &deviceInfo);
    bool runCmd(const QString &cmd, QString &deviceInfo);
    QString getPKStr(const QString &dtStr, const QString &dtInt);
    void getPKStr(QString &dtStr, QString &dtInt, const QString &cStr);

public:
    QString loadGeneratorKey();
    bool getRootPassword();

    QString standOutput_;
};

