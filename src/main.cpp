#include "mainwindow.h"
#include <DApplication>
#include <DWidgetUtil>
#include "version.h"
#include <DLog>
#include <stdio.h>
#include <stdlib.h>
#include "DTitlebar"

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    const char *descriptionText = QT_TRANSLATE_NOOP("MainWindow", "Deepin DeviceManager.");

    const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";

    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("DeviceManager");
    app.setApplicationDisplayName(QObject::tr("Device Manager"));
    app.setApplicationVersion("0.0.1");
    app.setProductIcon(QIcon(":/cpu.svg"));
    app.setWindowIcon(QIcon(":/cpu.svg"));
    app.setProductName(DApplication::translate("MainWindow", "Device Manager"));
    app.setApplicationDescription(DApplication::translate("MainWindow", descriptionText) + "\n");
    app.setApplicationAcknowledgementPage(acknowledgementLink);

    Dtk::Core::DLogManager::registerConsoleAppender();
    Dtk::Core::DLogManager::registerFileAppender();

    //app.setStyle("chameleon");

    MainWindow w;
    w.titlebar()->setIcon(QIcon(":/cpu.svg"));
    Dtk::Widget::moveToCenter(&w);
    w.show();

    return app.exec();
}
