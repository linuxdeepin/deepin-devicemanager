#include "mousewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MouseWidget::MouseWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Mouse"))
{
    initWidget();
}

void MouseWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Mouse")  + DApplication::translate("Main", " Info"));

    QStringList names = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Interface")
                        };

    int mouseCount = 0;
    QStringList inputdeviceList = DeviceInfoParserInstance.getInputdeviceList();

    foreach(const QString& device, inputdeviceList)
    {
        QString name = DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Name");
        if(false == name.contains("mouse", Qt::CaseInsensitive) )
        {
            continue;
        }

        ++mouseCount;
        name.remove("\"");

        QStringList contents = {
                                name,
                                DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Vendor"),
                                "PS/2",
                                };

        addSubInfo("", names, contents);
    }


    QStringList usbdeviceList = DeviceInfoParserInstance.getUsbdeviceList();
    foreach(const QString& device, usbdeviceList)
    {
        QString description =DeviceInfoParserInstance.qureyData("lshw", device, "description");
        if(false == description.contains("mouse", Qt::CaseInsensitive))
        {
            continue;
        }
        ++mouseCount;
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capabilities")
        };

        addSubInfo( "", names, contents);
    }

    if( mouseCount < 1)
    {
        setTitle("No " + DApplication::translate("Main", "Mouse") + " found!");
        addStrecch();
        return;
    }

}
