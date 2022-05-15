/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef SINGLEDEVICEMANAGER_H
#define SINGLEDEVICEMANAGER_H

#include "MainWindow.h"

#include <DApplication>
#include <DMainWindow>

#include <QCommandLineParser>
#include <QString>

DWIDGET_USE_NAMESPACE

class SingleDeviceManager : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DeviceManagerNotify")

public:
    enum TaskState {
        kTaskStarted,
        kTaskRunning,
        kTaskFinished
    };
    Q_ENUM(SingleDeviceManager::TaskState)

    explicit SingleDeviceManager(int &argc, char **argv);

    void activateWindow();

    bool parseCmdLine();

signals:
    void backgroundTaskStateChanged(SingleDeviceManager::TaskState state);

public slots:
    Q_SCRIPTABLE void startDeviceManager(QString pageDescription);

private:
    QString m_PageDescription;
    QScopedPointer<MainWindow> m_qspMainWnd;  // MainWindow ptr
};

#endif // SINGLEDEVICEMANAGER_H
