/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef DEVICEENABLE_H
#define DEVICEENABLE_H

#include <QString>
#include <QObject>

class DeviceEnable : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DeviceEnable)
public:
    explicit DeviceEnable(QObject *parent = nullptr);
    virtual ~DeviceEnable();

    /**
     * @brief enable 启用设备
     * @return
     */
    virtual bool enable(const QString& key) = 0;

    /**
     * @brief disable 禁用设备
     * @return
     */
    virtual bool disable(const QString& key) = 0;
};

#endif // DEVICEENABLE_H
