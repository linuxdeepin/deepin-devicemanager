#include "cdromwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CDRomWidget::CDRomWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "CD-ROM"))
{
    initWidget();
}

void CDRomWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));
    QStringList cdromList = DeviceInfoParserInstance.getCDRomList();

    if( cdromList.size() < 1)
    {
        setCentralInfo("No CD-ROM found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, cdromList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct model("Model");
        model.queryData( "lshw", device, "product");
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description("Description");
        description.queryData( "lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct busInfo("Bus Info");
        busInfo.queryData( "lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct logicalName("Logical Name");
        logicalName.queryData( "lshw", device, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");


        DeviceInfoParserInstance.queryRemainderDeviceInfo( "lshw", device, articles, existArticles);

        addDevice( device, articles, cdromList.size() );

        if( cdromList.size() > 1 )
        {
            QStringList tab =
            {
                model.value,
                vendor.value,
                description.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() )
        {
            if( false == model.value.contains(vendor.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value = vendor.value;
                overviewInfo_.value += " ";
            }

            overviewInfo_.value += model.value;

            if( false == overviewInfo_.value.contains(description.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value += " ";
                overviewInfo_.value += description.value;
            }
        }
    }

    if( cdromList.size() > 1 )
    {
        QStringList headers = { "Model", "Vendor", "Description" };
        addTable( headers, tabList);
    }
}
