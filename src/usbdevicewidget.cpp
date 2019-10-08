#include "usbdevicewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

UsbdeviceWidget::UsbdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Usb Device"))
{
    initWidget();
}

void UsbdeviceWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Usb Device"));

    QStringList names = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities"),
                            DApplication::translate("Main", "configuration")
                        };

    QStringList usbdeviceList = DeviceInfoParserInstance.getUsbdeviceList();

    if(usbdeviceList.size() < 1)
    {
        setTitle("No " + DApplication::translate("Main", "Usb Device") + " found!");
        addStrecch();
        return;
    }

    foreach(const QString& device, usbdeviceList)
    {
        QString name = DeviceInfoParserInstance.qureyData("lshw", device, "product");
        if(name == DApplication::translate("Main", "Unknown"))
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

