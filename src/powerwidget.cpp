#include "powerwidget.h"
#include "deviceinfoparser.h"

PowerWidget::PowerWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributePower)
{
    setTitle(DeviceAttributePower + " " + DeviceAttributeInfo);

    QStringList bluetoothNames = {
                            DeviceAttributeName,
                            DeviceAttributeVendor,
                            DeviceAttributeMac,
                            DeviceAttributeLinkPolicy,
                            DeviceAttributeLinkMode
                        };

    QStringList bluetoothList = DeviceInfoParserInstance.getBluetoothList();

    foreach(const QString& device, bluetoothList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("hciconfig", device, "Name"),
            DeviceInfoParserInstance.qureyData("hciconfig", device, "Manufacturer"),
            DeviceInfoParserInstance.qureyData("hciconfig", device, "BD Address"),
            DeviceInfoParserInstance.qureyData("hciconfig", device, "Link policy"),
            DeviceInfoParserInstance.qureyData("hciconfig", device, "Link mode")
        };

        addSubInfo( "", bluetoothNames, contents);
    }
}

