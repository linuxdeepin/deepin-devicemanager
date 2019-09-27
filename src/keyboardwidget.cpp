#include "keyboardwidget.h"
#include "deviceinfoparser.h"

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeKeyboard)
{
    initWidget();
}

void KeyboardWidget::initWidget()
{
    setTitle(DeviceAttributeKeyboard + " " + DeviceAttributeInfo);

    QStringList names = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeInterface
                        };

    QStringList inputdeviceList = DeviceInfoParserInstance.getInputdeviceList();
    foreach(const QString& device, inputdeviceList)
    {
        QString name = DeviceInfoParserInstance.fuzzyQueryData("catinput", device, "Name");
        if(false == name.contains("keyboard", Qt::CaseInsensitive) )
        {
            continue;
        }

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
        if(false == description.contains("keyboard", Qt::CaseInsensitive))
        {
            continue;
        }

        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", device, "product"),
            DeviceInfoParserInstance.qureyData("lshw", device, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", device, "capabilities")
        };

        addSubInfo( "", names, contents);
    }
}

