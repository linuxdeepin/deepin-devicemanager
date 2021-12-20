/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
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

#ifndef CPUINFO_H
#define CPUINFO_H

#include<QMap>
#include<QDir>

#include "PhysicalCpu.h"
#include "CoreCpu.h"
#include "LogicalCpu.h"

class CpuInfo
{
public:
    CpuInfo();
    ~CpuInfo();

    /**
     * @brief diagPrintInfo
     */
    void diagPrintInfo();
    /**
     * @brief loadCpuInfo
     * @return
     */
    bool loadCpuInfo();

    /**
     * @brief arch
     * @return
     */
    const QString &arch() const;

    /**
     * @brief logicalCpus
     * @param info
     */
    void logicalCpus(QString &info);

    /**
     * @brief physicalNum
     * @return
     */
    int physicalNum();

    /**
     * @brief coreNum
     * @return
     */
    int coreNum();

    /**
     * @brief logicalNum
     * @return
     */
    int logicalNum();

private:
    /**
     * @brief readCpuArchitecture
     */
    void readCpuArchitecture();

    /**
     * @brief readProcCpuinfo
     * @return
     */
    bool readProcCpuinfo();

    /**
     * @brief parseInfo
     * @param info
     * @return
     */
    bool parseInfo(const QString &info);

    /**
     * @brief logicalCpu
     * @param logical_id
     * @param core_id
     * @param physical_id
     * @return
     */
    LogicalCpu &logicalCpu(int logical_id);

    /**
     * @brief setProcCpuinfo
     * @param logical
     * @param mapInfo
     */
    void setProcCpuinfo(LogicalCpu &logical, const QMap<QString, QString> &mapInfo);

    /**
     * @brief readSysCpu : /sys/devices/system/cpu
     */
    void readSysCpu();

    /**
     * @brief readSysCpuN : /sys/devices/system/cpu/cpu* (cpu0 cpu1 cpu2 cpu3 cpu4)
     * @param path : /sys/devices/system/cpu
     */
    void readSysCpuN(int N, const QString &path);

    /**
     * @brief readPhysicalID
     * @param path
     * @return
     */
    int readPhysicalID(const QString &path);

    /**
     * @brief readCoreID
     * @param path
     * @return
     */
    int readCoreID(const QString &path);

    /**
        * @brief readThreadSiblingsListPath
        * @param path
        * @return
        */
    int readThreadSiblingsListPath(const QString &path);

    /**
     * @brief readCpuCache : /sys/devices/system/cpu/cpu0/cache
     * @param path : /sys/devices/system/cpu/cpu0
     * @param lcpu
     */
    void readCpuCache(const QString &path, LogicalCpu &lcpu);

    /**
     * @brief readCpuCacheIndex : /sys/devices/system/cpu/cpu0/cache/index* (index0 index1 index2 index3)
     * @param path : /sys/devices/system/cpu/cpu0/cache/index0
     * @param lcpu
     */
    void readCpuCacheIndex(const QString &path, LogicalCpu &lcpu);

    /**
     * @brief readCpuFreq
     * @param path
     * @param lcpu
     */
    void readCpuFreq(const QString &path, LogicalCpu &lcpu);


private:
    QMap<int, PhysicalCpu>     m_MapPhysicalCpu;
    QString                    m_Arch;
};

#endif // CPUINFO_H
