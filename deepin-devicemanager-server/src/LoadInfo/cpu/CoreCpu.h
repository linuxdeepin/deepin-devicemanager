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

#ifndef CORECPU_H
#define CORECPU_H

#include <QMap>

class LogicalCpu;
class CoreCpu
{
public:
    CoreCpu();
    explicit CoreCpu(int id);

    /**
     * @brief setCoreId : set core id
     * @param id
     */
    void setCoreId(int id);

    /**
     * @brief addLogicalCpu
     * @param id
     * @param lc
     */
    void addLogicalCpu(int id, const LogicalCpu& lc);

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
     * @brief appendKeyValue
     * @param info
     * @param key
     * @param value
     */
    void appendKeyValue(QString &info, const QString &key, const QString &value);
    void appendKeyValue(QString &info, const QString &key, int value);

    /**
     * @brief coreId
     * @return
     */
    int coreId();

private:
    int m_CoreId;                //<! core id
    QMap<int, LogicalCpu> m_MapLogicalCpu; //<! logical info
};

#endif // CORECPU_H
