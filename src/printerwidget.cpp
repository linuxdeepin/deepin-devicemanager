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
    QStringList printerList = DeviceInfoParserInstance.getPrinterList();

    if( printerList.size() < 1)
    {
        QStringList emptyList;
        addInfo("No Printer found!", emptyList, emptyList);
        addStrecch();
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, printerList)
    {
        ArticleStruct printerInfo("printer-info");
        printerInfo.queryData( "Cups", device, "printer-info");
        articles.push_back(printerInfo);
        existArticles.insert("capacity");

        ArticleStruct printerMakeAndModel("printer-make-and-model");
        printerMakeAndModel.queryData( "Cups", device, "printer-make-and-model");
        articles.push_back(printerMakeAndModel);
        existArticles.insert("printer-make-and-model");



        if( false == DeviceInfoParserInstance.queryDeviceInfo( "Cups", device, articles))
        {
            continue;
        }

        addSubInfo( device, articles);
        articles.clear();

        if(overviewInfo_.value.isEmpty() )
        {
            overviewInfo_.value = printerMakeAndModel.value;
            overviewInfo_.value += " ";
            overviewInfo_.value += printerInfo.value;
        }
    }
}
