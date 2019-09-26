#include "portwidget.h"
#include "deviceinfoparser.h"

PortWidget::PortWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributePorts)
{
    //setTitle(DeviceAttributeOtherInputdevice + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributePortType,
                            DeviceAttributeConnectorType,
                            DeviceAttributeReferenceDesignator,
                            DeviceAttributeCapabilities
                        };

    QStringList portsList = DeviceInfoParserInstance.getPortsList();

    foreach(const QString& device, portsList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("dmidecode", device, "Port Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "External Connector Type"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "External Reference Designator"),
            DeviceInfoParserInstance.qureyData("dmidecode", device, "capabilities"),
        };

        addSubInfo( "", names, contents);
    }
}

