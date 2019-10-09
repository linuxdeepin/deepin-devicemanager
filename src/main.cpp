#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include "version.h"
#include <DLog>
#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"

#include "DApplicationSettings"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName(DApplication::translate("Main", "DeviceManager"));
    app.setApplicationDisplayName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationVersion(VERSION);
    app.setProductIcon(QIcon(":images/cpu.svg"));
    app.setWindowIcon(QIcon(":images/cpu.svg"));
    app.setProductName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationDescription(DApplication::translate("Main", "Device Manager is a gadget for viewing device information.") + "\n");

    const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
    app.setApplicationAcknowledgementPage(acknowledgementLink);

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    //app.setStyle("chameleon");

    DApplicationSettings settings;

    MainWindow w;
    w.titlebar()->setIcon(QIcon(":images/cpu.svg"));
    Dtk::Widget::moveToCenter(&w);
    w.show();

    return app.exec();
}
