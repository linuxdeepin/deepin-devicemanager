#include "otherinputdevicewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

OtherInputdeviceWidget::OtherInputdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Other Inputdevice"))
{
    initWidget();
}

void OtherInputdeviceWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));

    QStringList names = {   DApplication::translate("Main", "Description"),
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities")
                        };

    QStringList otherInputdeviceList = DeviceInfoParserInstance.getOtherInputdeviceList();

    if(otherInputdeviceList.size() == 0)
    {
        setTitle("No " + DApplication::translate("Main", "Other Inputdevice") + " found!");
        addStrecch();
    }

    foreach(const QString& device, otherInputdeviceList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.queryData("lshw", device, "description"),
            DeviceInfoParserInstance.queryData("lshw", device, "product"),
            DeviceInfoParserInstance.queryData("lshw", device, "vendor"),
            DeviceInfoParserInstance.queryData("lshw", device, "capabilities"),
        };

        addSubInfo( "", names, contents);
    }
}

