#include "otherinputdevicewidget.h"
#include "deviceinfoparser.h"

OtherInputdeviceWidget::OtherInputdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeOtherInputdevice)
{
    initWidget();
}

void OtherInputdeviceWidget::initWidget()
{
    //setTitle(DeviceAttributeOtherInputdevice + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributeDescription,
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeCapabilities
                        };

    QStringList otherInputdeviceList = DeviceInfoParserInstance.getOtherInputdeviceList();

    if(otherInputdeviceList.size() == 0)
    {
        setTitle("No " + DeviceAttributeOtherInputdevice + " found!");
        addStrecch();
    }

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

