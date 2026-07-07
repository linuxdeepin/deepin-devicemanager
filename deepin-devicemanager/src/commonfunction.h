// Copyright (C) 2019 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
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

    static QByteArray executeClientCmd(const QString& cmd, const QStringList& args = QStringList(), const QString& workPath = QString(), int msecsWaiting = 30000, bool useEnv = true);

    static bool isShowScreenSize();

    static QString formatTotalCache(const QString& perThreadCache, int coreCount);

    static int parseSharedCpuCount(const QString &sharedCpuList);

    /**
     * @brief safeReadSysFsFile: 安全读取 sysfs 下指定文件内容
     * @param baseDir 根目录，如 /sys/devices/xxx/mmc0:0001
     * @param fileName 待读取的文件名（禁止包含路径分隔符）
     * @param errOut 错误输出参数，成功时为空
     * @return 文件内容（去换行空格），失败返回空字符串
     */
    static QString safeReadSysFsFile(const QString &baseDir, const QString &fileName, QString &errOut);
};
#endif // COMMONFUNCTION_H
