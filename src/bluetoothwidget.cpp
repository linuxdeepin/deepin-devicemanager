#include "bluetoothwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE


BluetoothWidget::BluetoothWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Bluetooth"))
{
    initWidget();
}

void BluetoothWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "Bluetooth") + " " + DApplication::translate("Main", " Info"));

    QStringList bluetoothNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Mac Address"),
                            DApplication::translate("Main", "Link Policy"),
                            DApplication::translate("Main", "Link Mode")
                        };

    QStringList bluetoothList = DeviceInfoParserInstance.getBluetoothList();

    if(bluetoothList.size() < 1)
    {
        setTitle("No " + DApplication::translate("Main", "Bluetooth") + " found!");
        addStrecch();
        return;
    }

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

