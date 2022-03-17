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

#ifndef DRIVERDBUSINTERFACE_H
#define DRIVERDBUSINTERFACE_H

#include <QObject>
#include <QDBusContext>

class DriverManager;
class ModCore;
class DriverDBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.drivermanager")
public:
    explicit DriverDBusInterface(QObject *parent = nullptr);

private:
    void initConnects();

signals:
    Q_SCRIPTABLE void sigProgressDetail(int progress, const QString &strDeatils);
    Q_SCRIPTABLE void sigFinished(bool bsuccess, QString mdg);

public slots:
    Q_SCRIPTABLE bool unInstallDriver(const QString &modulename);
    Q_SCRIPTABLE bool installDriver(const QString &filepath);
    Q_SCRIPTABLE QStringList checkModuleInUsed(const QString &modulename);
    Q_SCRIPTABLE bool isDriverPackage(const QString &filepath);
    Q_SCRIPTABLE bool isBlackListed(const QString &modName);
    Q_SCRIPTABLE bool isArchMatched(const QString &filePath);
    Q_SCRIPTABLE bool isDebValid(const QString &filePath);
    Q_SCRIPTABLE bool unInstallPrinter(const QString &vendor, const QString &model);

private:
    DriverManager *mp_drivermanager;
    ModCore *pcore = nullptr; //临时测试使用
};

#endif // DRIVERDBUSINTERFACE_H
