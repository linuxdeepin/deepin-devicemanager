#include "otherpcidevice.h"
#include "deviceinfoparser.h"

OtherPciDeviceWidget::OtherPciDeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeOtherPciDevice)
{
    initWidget();
}

void OtherPciDeviceWidget::initWidget()
{
    //setTitle(DeviceAttributeOtherInputdevice + " " + DeviceAttributeInfo);

    QStringList names = {   DeviceAttributeDescription,
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeCapabilities
                        };

    QStringList otherPcideviceList = DeviceInfoParserInstance.getOtherPciDeviceList();

    if(otherPcideviceList.size() == 0)
    {
        setTitle("No " + DeviceAttributeOtherPciDevice + " found!");
        addStrecch();
        return;
    }

    foreach(const QString& device, otherPcideviceList)
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

