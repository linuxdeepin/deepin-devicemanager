/*
* Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd.
*
* Author:     XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: LiBang.Liu <liulibang@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

#ifndef COMMONFUNCTION_H
#define COMMONFUNCTION_H

// 其它头文件
#include <QString>
#include <QMap>

#include <sys/utsname.h>

static QMap<QString, QString> mapArch = {   {"aarch64", "arm64"}
    , {"x86_64", "amd64"}
    , {"mips64", "mips64el"}
    , {"i386", "i386"}
    , {"sw_64", "sw_64"}
    , {"loongarch", "loongarch"}
    , {"loongarch64", "loongarch64"}
};

/**
 * @brief The DDeviceType enum 可更新驱动类型
 */
enum DriverType {
    DR_Null = 0,
    DR_Bluetooth = 1,
    DR_Camera = 2,
    DR_Gpu = 3,
    DR_Keyboard = 4,
    DR_Sound = 5,
    DR_Mouse = 6,
    DR_Network = 7,
    DR_Printer = 8,
    DR_Scaner = 9,
    DR_Tablet = 10,
    DR_WiFi = 11
};

typedef struct STR_DriverInfo {
    DriverType type;
    QString    vendorId;
    QString    vendorName;
    QString    modelId;
    QString    modelName;
    QString    driverName;
    QString    version;

    QString    debVersion;//包版本
    QString    packages;//包名
} strDriverInfo;

typedef struct STR_WAREHOUSE_DRIVER_INFO {
    QString strDebManufacturer;//包厂商
    QString strDebVersion;//包版本
    QString strPackages;//包名
    int iLevel;
} strRepoDriverInfo;


class Common
{
public:
    Common();
    ~Common();
    static QString getArch()
    {
        QString arch;
        struct utsname utsbuf;
        if (-1 != uname(&utsbuf)) {
            arch = QString::fromLocal8Bit(utsbuf.machine);
        }
        return arch;
    }

    static QString getArchStore()
    {
        return mapArch[getArch()];
    }

};

#define EC_NULL 0
#define EC_NETWORK 1
#define EC_CANCEL 2
#define EC_3 3
#define EC_4 4
#define EC_5 5
#define EC_6 6

#endif // COMMONFUNCTION_H
