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

#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>
#include <QDBusContext>

class MainJob;
class DBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.devicemanager")
public:
    explicit DBusInterface(QObject *parent = nullptr);

    /**
     * @brief setMainJob 设置主线job
     * @param job
     */
    void setMainJob(MainJob *job);

signals:

public slots:
    /**
     * @brief getInfo : Obtain hardware information through the DBus
     * @param key
     * @return : Hardware info
     */
    Q_SCRIPTABLE QString getInfo(const QString &key);

    /**
     * @brief exceCmd : Exec cmd through the DBus
     * @param cmd : the cmd
     * @return : result
     */
    Q_SCRIPTABLE QString execCmd(const QString &cmd);

private:
    MainJob *mp_MainJob;       //<!         主线job
};

#endif // DBUSINTERFACE_H
