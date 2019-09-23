#include "bluetoothwidget.h"
#include "deviceinfoparser.h"

BluetoothWidget::BluetoothWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeBluetooth)
{
    setTitle(DeviceAttributeBluetooth + " " + DeviceAttributeInfo);

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

