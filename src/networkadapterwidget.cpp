#include "networkadapterwidget.h"
#include "deviceinfoparser.h"

NetworkadapterWidget::NetworkadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeNetworkAdapter)
{
    initWidget();
}

void NetworkadapterWidget::initWidget()
{
    setTitle(DeviceAttributeNetworkAdapter + " " + DeviceAttributeInfo);

    QStringList networkadapterNames = {
                            DeviceAttributeName,
                            DeviceAttributeModel,
                            DeviceAttributeVendor,
                            DeviceAttributeMac,
                            DeviceAttributeSpeed
                        };

    QStringList networkadapterList = DeviceInfoParserInstance.getNetworkadapterList();

    foreach(const QString& networkadapter, networkadapterList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.qureyData("lshw", networkadapter, "description"),
            DeviceInfoParserInstance.qureyData("lshw", networkadapter, "product"),
            DeviceInfoParserInstance.qureyData("lshw", networkadapter, "vendor"),
            DeviceInfoParserInstance.qureyData("lshw", networkadapter, "serial"),
            DeviceInfoParserInstance.qureyData("lshw", networkadapter, "capacity")
        };

        addSubInfo( "", networkadapterNames, contents);
    }
}
