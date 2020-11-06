/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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

#include "Page/MainWindow.h"

#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"
#include "DApplicationSettings"
#include "environments.h"
#include "DebugTimeManager.h"
#include "application.h"
#include "zmq.h"
#include "zhelpers.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    PERF_PRINT_BEGIN("POINT-01", "");

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Application app(argc, argv);
    app.setAutoActivateWindows(true);


    // 保证进程唯一性
    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");
    if (DGuiApplicationHelper::instance()->setSingleInstance("deepin-devicemanager",
                                                             DGuiApplicationHelper::UserScope)) {
        app.loadTranslator();
        app.setOrganizationName("deepin");
        app.setApplicationName("deepin-devicemanager");
        app.setApplicationDisplayName(QObject::tr("Device Manager"));
        app.setApplicationVersion(VERSION);
        app.setProductName(QObject::tr("Device Manager"));
        app.setApplicationDescription(QObject::tr("Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");
        const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
        app.setApplicationAcknowledgementPage(acknowledgementLink);
        DApplicationSettings settinAgs;
        Dtk::Core::DLogManager::registerConsoleAppender();
        Dtk::Core::DLogManager::registerFileAppender();

        MainWindow w(nullptr);
        gApp->setMainWindow(&w);
        QObject::connect(&app,
                         &DApplication::newInstanceStarted,
                         &w,
                         &MainWindow::activateWindow);
        w.titlebar()->setTitle("");

        QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");

        if (appIcon.isNull() == false) {
            app.setProductIcon(appIcon);
            app.setWindowIcon(appIcon);
            w.titlebar()->setIcon(appIcon);
        }

        Dtk::Widget::moveToCenter(&w);

        w.show();
        return app.exec();
    }
    return 0;
}
