#include "otherpcidevice.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

OtherPciDeviceWidget::OtherPciDeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Other PCI Device"))
{
    initWidget();
}

void OtherPciDeviceWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Description"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities")
                        };

    QStringList otherPcideviceList = DeviceInfoParserInstance.getOtherPciDeviceList();

    if(otherPcideviceList.size() == 0)
    {
        setTitle("No " + DApplication::translate("Main", "Other PCI Device") + " found!");
        addStrecch();
    }

    foreach(const QString& device, otherPcideviceList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.queryData("lshw", device, "product"),
            DeviceInfoParserInstance.queryData("lshw", device, "description"),
            DeviceInfoParserInstance.queryData("lshw", device, "vendor"),
            DeviceInfoParserInstance.queryData("lshw", device, "capabilities"),
        };

        addSubInfo( "", names, contents);
    }
}

