#include "otherpcidevice.h"
#include "deviceinfoparser.h"

OtherPciDeviceWidget::OtherPciDeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeOtherPciDevice)
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
        setTitle("No " + DeviceAttributeOtherPciDevice + " found!");
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

