#include "printerwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PrinterWidget::PrinterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Printer"))
{
    initWidget();
}

void PrinterWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));
    QList<ArticleStruct> articles;

    QStringList printerList = DeviceInfoParserInstance.getPrinterList();

    if( printerList.size() < 1)
    {
        QStringList emptyList;
        addInfo("No Printer found!", emptyList, emptyList);
        addStrecch();
        return;
    }

    foreach(const QString& device, printerList)
    {
        if( false == DeviceInfoParserInstance.queryDeviceInfo( "Cups", device, articles))
        {
            continue;
        }

        addSubInfo( device, articles);
        articles.clear();
    }
}
