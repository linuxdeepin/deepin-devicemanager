#include "displayadapterwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DisplayadapterWidget::DisplayadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "DisplayAdapter"))
{
    initWidget();
}

void DisplayadapterWidget::initWidget()
{
    setTitle(DApplication::translate("Main", "DisplayAdapter")  + DApplication::translate("Main", " Info"));

    QStringList displayadapterNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Graphic Memory"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Driver"),
                            //DApplication::translate("Main", "Max display resolution"),
                            DApplication::translate("Main", "Support Interface")
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

