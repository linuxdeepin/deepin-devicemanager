/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef DRIVERSCANNER_H
#define DRIVERSCANNER_H

#include "MacroDefinition.h"

#include <QThread>

class DriverScanner : public QThread
{
    Q_OBJECT
public:
    explicit DriverScanner(QObject *parent = nullptr);

    /**
     * @brief run
     */
    void run();

    /**
     * @brief setDriverList
     * @param lstInfo
     */
    void setDriverList(QList<DriverInfo *> lstInfo);

signals:
    void scanInfo(const QString &info, int progress);
    void scanFinished(ScanResult sr);

private:
    QList<DriverInfo *>     m_ListDriverInfo;
    bool m_IsStop;
};

#endif // DRIVERSCANNER_H
