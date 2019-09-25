#include "portwidget.h"
#include "deviceinfoparser.h"

PortWidget::PortWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeOtherPciPort)
{
    //setTitle(DeviceAttributeOtherInputdevice + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributeDescription,
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeCapabilities
                        };

    QStringList otherInputdeviceList = DeviceInfoParserInstance.getOtherInputdeviceList();

    foreach(const QString& device, otherInputdeviceList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "description"),
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capabilities"),
        };

        addSubInfo( "", names, contents);
    }
}

