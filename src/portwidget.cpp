#include "portwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PortWidget::PortWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Ports"))
{
    initWidget();
}

void PortWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Reference Designator"),
                            DApplication::translate("Main", "Port Type"),
                            DApplication::translate("Main", "Connector Type"),
                            DApplication::translate("Main", "Capabilities")
                        };

    QStringList portsList = DeviceInfoParserInstance.getPortsList();

    foreach(const QString& device, portsList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("dmidecode", device, "External Reference Designator"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "Port Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "External Connector Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "capabilities"),
        };

        addSubInfo( "", names, contents);
    }
}
