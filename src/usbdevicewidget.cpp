#include "usbdevicewidget.h"
#include "deviceinfoparser.h"

UsbdeviceWidget::UsbdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeUsbdevice)
{
    setTitle(DeviceAttributeUsbdevice);

    QStringList names = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeCapabilities,
                            DeviceAttributeConfiguration
                        };

    QStringList usbdeviceList = DeviceInfoParserInstance.getUsbdeviceList();

    foreach(const QString& device, usbdeviceList)
    {
        QString name = DeviceInfoParserInstance.qureyData("lshw", device, "product");
        if(name == DeviceAttributeUnknown)
        {
            name = DeviceInfoParserInstance.qureyData("lshw", device, "description");
        }
        QStringList contents = {
            name,
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capabilities"),
            DeviceInfoParserInstance.qureyData("lshw", device, "configuration")
        };

        addSubInfo( "", names, contents);
    }
}

