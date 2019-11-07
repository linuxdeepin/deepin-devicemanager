#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
//#include "version.h"
#include <DLog>
#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"
#include "DApplicationSettings"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    DApplication::loadDXcbPlugin();

    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-devicemanager");
    app.setApplicationDisplayName(DApplication::translate("Main", "Device Manager"));

    //app.setApplicationVersion(VERSION);
    app.setApplicationVersion(DApplication::buildVersion("3.0"));

    app.setProductIcon(QIcon(":images/cpu.svg"));
    app.setWindowIcon(QIcon(":images/cpu.svg"));
    app.setProductName(DApplication::translate("Main", "Device Manager"));

    app.setApplicationDescription(DApplication::translate("Main", "Deepin Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");

    const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
    app.setApplicationAcknowledgementPage(acknowledgementLink);

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    //app.setStyle("chameleon");

    DApplicationSettings settinAgs;

    MainWindow w(nullptr);
    w.titlebar()->setTitle("");
    w.titlebar()->setIcon(QIcon(":images/cpu.svg"));

    w.show();
    Dtk::Widget::moveToCenter(&w);

    return app.exec();
}
