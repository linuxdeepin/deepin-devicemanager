// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONFUNCTION_H
#define COMMONFUNCTION_H

// 其它头文件
#include <QString>
#include <QStringList>

class Common
{
public:
    Common();
    ~Common();
    enum SpecialComputerType{
        Unknow = -1,
        NormalCom,
        PGUW,
        KLVV,
        KLVU,
        PGUV,
        kSpecialType5,
        kSpecialType6,
        kSpecialType7,
        kCustomType
    };

    enum SpecialCpuType {
        kUnknowCpuType = 0,
        kSpecialCpuType1
    };

    static QString getArch();

    static QString getArchStore();

    static QString checkBoardVendorFlag();

    static QString boardVendorType();
    static QString specialVendorType();
    static QString specialHString();
    static QString tomlFilesNameGet();
    static void tomlFilesNameSet(const QString &name);
    static bool isHwPlatform();

    /**
     * @brief specialComType
     * special computer type:PGUW(value:1),KLVV/L540(value:2),KLVU(value:3),PGUV/W585(value:4)
     */
    static int specialComType;
    static SpecialCpuType curCpuType;

    static QByteArray executeClientCmd(const QString& cmd, const QStringList& args = QStringList(), const QString& workPath = QString(), int msecsWaiting = 30000);
};
#endif // COMMONFUNCTION_H
