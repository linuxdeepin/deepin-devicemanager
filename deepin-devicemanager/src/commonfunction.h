// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONFUNCTION_H
#define COMMONFUNCTION_H

// 其它头文件
#include <QString>

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
        PGUX,
        M009
    };
    static QString getArch();

    static QString getArchStore();

    static QString checkBoardVendorFlag();

    static QString boardVendorType();

    /**
     * @brief specialComType
     * special computer type:PGUW(value:1),KLVV/L540(value:2),KLVU(value:3),PGUV/W585(value:4),PGUX(value:5),M009(value:6)
     */
    static int specialComType;
};
#endif // COMMONFUNCTION_H
