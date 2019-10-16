#include "networkadapterwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

NetworkadapterWidget::NetworkadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "NetworkAdapter"))
{
    initWidget();
}

void NetworkadapterWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "NetworkAdapter")  + DApplication::translate("Main", " Info"));

    QStringList networkadapterNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Mac Address"),
                            DApplication::translate("Main", "Speed")
                        };

    QStringList networkadapterList = DeviceInfoParserInstance.getNetworkadapterList();

    foreach(const QString& networkadapter, networkadapterList)
    {
        QStringList contents = {
            DeviceInfoParserInstance.queryData("lshw", networkadapter, "description"),
            DeviceInfoParserInstance.queryData("lshw", networkadapter, "product"),
            DeviceInfoParserInstance.queryData("lshw", networkadapter, "vendor"),
            DeviceInfoParserInstance.queryData("lshw", networkadapter, "serial"),
            DeviceInfoParserInstance.queryData("lshw", networkadapter, "capacity")
        };

        addSubInfo( "", networkadapterNames, contents);
    }
}
