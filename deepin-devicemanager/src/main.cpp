// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Page/MainWindow.h"
#include "DTitlebar"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "DApplicationSettings"
#endif

#include "environments.h"
#include "DebugTimeManager.h"
#include "SingleDeviceManager.h"
#include "DDLog.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DLog>
#include <DSysInfo>

#include <QDBusConnection>
#include <QDBusInterface>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <DLog>
#include "LogConfigread.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <polkit-qt5-1/PolkitQt1/Authority>
#else
#include <polkit-qt6-1/PolkitQt1/Authority>
#endif

using namespace DDLog;

DWIDGET_USE_NAMESPACE

const QString SERVICE_NAME_V23 = "org.deepin.dde.Notification1";
const QString DEVICE_SERVICE_PATH_V23 = "/org/deepin/dde/Notification1";
const QString DEVICE_SERVICE_INTERFACE_V23 = "org.deepin.dde.Notification1";

const QString SERVICE_NAME_V20 = "com.deepin.dde.Notification";
const QString DEVICE_SERVICE_PATH_V20 = "/com/deepin/dde/Notification";
const QString DEVICE_SERVICE_INTERFACE_V20 = "com.deepin.dde.Notification";

inline bool isV20() { return Dtk::Core::DSysInfo::majorVersion() == "20"; }
const QString SERVICE_NAME = isV20() ? SERVICE_NAME_V20 : SERVICE_NAME_V23;
const QString DEVICE_SERVICE_PATH = isV20() ? DEVICE_SERVICE_PATH_V20 : DEVICE_SERVICE_PATH_V23;
const QString DEVICE_SERVICE_INTERFACE = isV20() ? DEVICE_SERVICE_INTERFACE_V20 : DEVICE_SERVICE_INTERFACE_V23;

DCORE_USE_NAMESPACE
using namespace PolkitQt1;

void notify(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    MLogger();

    // /usr/bin/devicemanager notify
    if (argc > 2 && QString(argv[1]).contains("notify")) {
        qCDebug(appLog) << "Starting notification process";
        notify(argc, argv);
        return -1;
    }
    #if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
        Dtk::Core::DLogManager::registerJournalAppender();
    #else
        Dtk::Core::DLogManager::registerFileAppender();
    #endif
    #ifdef QT_DEBUG
        Dtk::Core::DLogManager::registerConsoleAppender();
    #endif

    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        qCDebug(appLog) << "XDG_CURRENT_DESKTOP is not deepin, setting it to Deepin";
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    PERF_PRINT_BEGIN("POINT-01", "");

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    SingleDeviceManager app(argc, argv);
    app.setAutoActivateWindows(true);

    // 保证进程唯一性
    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");
    /*if (DGuiApplicationHelper::instance()->setSingleInstance("deepin-devicemanager",
                                                             DGuiApplicationHelper::UserScope))*/ {
        app.loadTranslator();
        app.setOrganizationName("deepin");
        app.setApplicationName("deepin-devicemanager");
        app.setApplicationDisplayName(QObject::tr("Device Manager"));
        app.setApplicationVersion(VERSION);
        app.setProductName(QObject::tr("Device Manager"));
        app.setApplicationDescription(QObject::tr("Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");
        const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
        app.setApplicationAcknowledgementPage(acknowledgementLink);

        qCDebug(appLog) << "Checking single instance...";
        if (!DGuiApplicationHelper::instance()->setSingleInstance(app.applicationName(), DGuiApplicationHelper::UserScope)) {
            qCDebug(appLog) << "Another instance is running, exiting";
            exit(0);
        }
#ifndef DISABLE_POLKIT
        Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                                SystemBusNameSubject(QDBusConnection::systemBus().baseService()),
                                                                                Authority::AllowUserInteraction);
        qCDebug(appLog) << "Authorization check result:" << result;
        if (result != Authority::Yes) {
            qCDebug(appLog) << "Authorization failed, exiting";
            return 0;
        }
#endif
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        DApplicationSettings settinAgs;
#endif
        Dtk::Core::DLogManager::registerFileAppender();

        QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");

        if (false == appIcon.isNull()) {
            qCDebug(appLog) << "Setting application icon";
            app.setProductIcon(appIcon);
            app.setWindowIcon(appIcon);
        }

        QDBusConnection dbus = QDBusConnection::sessionBus();
        qCDebug(appLog) << "Registering DBus service...";
        if (dbus.registerService("com.deepin.DeviceManagerNotify")) {
            qCDebug(appLog) << "DBus service registered successfully";
            dbus.registerObject("/com/deepin/DeviceManagerNotify", &app, QDBusConnection::ExportScriptableSlots);
            app.parseCmdLine();
            app.activateWindow();
            return app.exec();
        } else {
            qCDebug(appLog) << "DBus service already registered, activating existing instance";
            QCommandLineParser parser;
            parser.process(app);

            QVariant var = "";
            if (parser.positionalArguments().size() > 0) {
                var = parser.positionalArguments().at(0);
            }
            qCInfo(appLog) << var;
            QDBusInterface notification("com.deepin.DeviceManagerNotify", "/com/deepin/DeviceManagerNotify", "com.deepin.DeviceManagerNotify", QDBusConnection::sessionBus());
            QDBusMessage msg = notification.call(QDBus::AutoDetect, "startDeviceManager", var);
            return 0;
        }
    }
}

void notify(int argc, char *argv[])
{
    qCDebug(appLog) << "Starting notification process";
    // 1. 连接到dbus
    qCDebug(appLog) << "Starting notification process";
    if (!QDBusConnection::sessionBus().isConnected()) {
        qCWarning(appLog) << "Cannot connect to D-Bus session bus";
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. 加载翻译文件
    QString body = QObject::tr("New drivers available! Install or update them now.");
    QString view = QObject::tr("View");
    QString l = QString(argv[2]);
    if ("zh_CN" == l) {
        qCDebug(appLog) << "Setting notification language to zh_CN";
        body = QString("您有驱动可进行安装/更新");
        view = QString("查 看");
    } else if ("zh_HK" == l) {
        qCDebug(appLog) << "Setting notification language to zh_HK";
        body = QString("您有驅動可進行安裝/更新");
        view = QString("查 看");
    } else if ("zh_TW" == l) {
        qCDebug(appLog) << "Setting notification language to zh_TW";
        body = QString("您有驅動可進行安裝/更新");
        view = QString("查 看");
    }

    // 3. create interface
    QDBusInterface *mp_Iface = new QDBusInterface(SERVICE_NAME, DEVICE_SERVICE_PATH, DEVICE_SERVICE_INTERFACE, QDBusConnection::sessionBus());

    QString appname("deepin-devicemanager");
    uint replaces_id = 0;
    QString appicon("deepin-devicemanager");
    QString title = "";

    QStringList actionlist;
    actionlist << "view" << view;
    QVariantMap hints;
    hints.insert(QString("x-deepin-action-view"),
                 QVariant(QString("/usr/bin/deepin-devicemanager,driver")));  //实现查看2按钮点击打开控制中心账户界面)
    int timeout = 3000;

    int count = 0;
    qCDebug(appLog) << "Attempting to send notification...";
    while (count < 10) {
        QDBusReply<uint32_t> reply  = mp_Iface->call("Notify", appname, replaces_id, appicon, title, body, actionlist, hints, timeout);
        if (reply.isValid()) {
            qCDebug(appLog) << "Notification sent successfully";
            return;
        }
        qCDebug(appLog) << "Notification attempt" << count << "failed";
        count++;
    }
}
