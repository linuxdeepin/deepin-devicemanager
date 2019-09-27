#include "displayadapterwidget.h"
#include "deviceinfoparser.h"

DisplayadapterWidget::DisplayadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DeviceAttributeDisplayAdapter)
{
    initWidget();
}

void DisplayadapterWidget::initWidget()
{
    setTitle(DeviceAttributeDisplayAdapter + " " + DeviceAttributeInfo);

    QStringList displayadapterNames = {
                            DeviceAttributeName,
                            DeviceAttributeGraphicMemory,
                            DeviceAttributeVendor,
                            DeviceAttributeDriver,
                            //DeviceAttributeMaxDisplayResolution,
                            DeviceAttributeSupportInterface
                        };

    QStringList displayadapterList = DeviceInfoParserInstance.getDiaplayadapterList();

    foreach(const QString& displayadapter, displayadapterList)
    {
        //todo more graphic card
        QString name = DeviceInfoParserInstance.qureyData("lspci", displayadapter, "Name");
        name.remove(" Corporation", Qt::CaseInsensitive);

        int index = name.indexOf('(');
        if(index > 0)
        {
            name = name.mid(0, index);
        }

        QString manufacturer = DeviceInfoParserInstance.qureyData("lspci", displayadapter, "Subsystem").split(" ").first();

        QString interface = DeviceInfoParserInstance.getDisplayInterfaceList().join(", ");

        QStringList contents = {
            name,
            DeviceInfoParserInstance.qureyData("lspci", displayadapter, "Memory"),
            manufacturer,
            DeviceInfoParserInstance.qureyData("lspci", displayadapter, "Kernel modules"),
            interface
        };

        addSubInfo( "", displayadapterNames, contents);
    }
}

