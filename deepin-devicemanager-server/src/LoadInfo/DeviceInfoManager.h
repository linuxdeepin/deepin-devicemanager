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

#ifndef DEVICEINFOMANAGER_H
#define DEVICEINFOMANAGER_H

#include <QObject>
#include <QMap>

class DeviceInfoManager : public QObject
{
    Q_OBJECT
public:
    static inline DeviceInfoManager *getInstance()
    {
        if (!s_Instance)
            s_Instance = new DeviceInfoManager;
        return s_Instance;
    }

    /**
     * @brief addInfo
     * @param key
     * @param value
     */
    void addInfo(const QString &key, const QString &value);

    /**
     * @brief getInfo
     * @param key
     * @return
     */
    const QString &getInfo(const QString &key);

    /**
     * @brief isInfoExisted
     * @param key
     * @return
     */
    bool isInfoExisted(const QString &key);

protected:
    explicit DeviceInfoManager(QObject *parent = nullptr);

private:
    static DeviceInfoManager *s_Instance;
    QMap<QString, QString>     m_MapInfo;
};

#endif // DEVICEINFOMANAGER_H
