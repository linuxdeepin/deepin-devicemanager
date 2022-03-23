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

#ifndef PHYSICALCPU_H
#define PHYSICALCPU_H

#include <QMap>

class CoreCpu;
class LogicalCpu;
class PhysicalCpu
{
public:
    PhysicalCpu();
    explicit PhysicalCpu(int id);

    /**
     * @brief addCoreCpu
     * @param id
     * @param cpu
     */
    void addCoreCpu(int id, const CoreCpu& cpu);

    /**
     * @brief coreIsExisted
     * @return
     */
    bool coreIsExisted(int id);

    /**
     * @brief coreCpu
     * @param id
     * @return
     */
    CoreCpu& coreCpu(int id);

    /**
     * @brief logicalIsExisted
     * @param id
     * @return
     */
    bool logicalIsExisted(int id);

    /**
     * @brief logicalCpu
     * @param id
     * @return
     */
    LogicalCpu& logicalCpu(int id);

    /**
     * @brief getInfo
     * @param info
     */
    void getInfo(QString& info);

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
    int m_PhysicalCpu;                   //<! physical id
    QMap<int, CoreCpu> m_MapCoreCpu;      //<! core cpu
};

#endif // PHYSICALCPU_H
