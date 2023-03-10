// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlinterface.h"

#include <QCoreApplication>
#include <QDebug>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ControlInterface controlInterface;
    QDBusConnection connection = controlInterface.qDbusConnection();
    if (!connection.registerObject("/org/deepin/DeviceControl", &controlInterface,
                                   QDBusConnection::ExportScriptableContents)) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }
    if (!connection.registerService("org.deepin.DeviceControl")) {
        qWarning() << "failed to register dbus object" << connection.lastError().message();
    }

    return a.exec();
}
