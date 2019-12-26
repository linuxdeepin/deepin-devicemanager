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

#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DLog>
#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"
#include "DApplicationSettings"
#include "deviceinfoparser.h"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    DApplication::loadDXcbPlugin();
    DApplication app(argc, argv);

    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-devicemanager");
    app.setApplicationDisplayName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationVersion(DApplication::buildVersion(QDate::currentDate().toString("yyyyMMdd")));

    app.setProductName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationDescription(DApplication::translate("Main", "Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");

    const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
    app.setApplicationAcknowledgementPage(acknowledgementLink);

    DApplicationSettings settinAgs;

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    MainWindow w(nullptr);
    w.titlebar()->setTitle("");

    QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");

    if(appIcon.isNull() == false)
    {
        app.setProductIcon(appIcon);
        app.setWindowIcon(appIcon);
        w.titlebar()->setIcon(appIcon);
    }

    w.show();
    return app.exec();
}
