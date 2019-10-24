#include "otherpcidevice.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

OtherPciDeviceWidget::OtherPciDeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Other PCI Device"))
{
    initWidget();
}

void OtherPciDeviceWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));

    QStringList otherPcideviceList = DeviceInfoParserInstance.getOtherPciDeviceList();

    if(otherPcideviceList.size() < 1)
    {
        setCentralInfo("No Other PCI Device found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, otherPcideviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "lshw", device, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description("Description");
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo("Bus Info");
        busInfo.queryData( "lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct physicalId("Physical ID");
        physicalId.queryData( "lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct version("Version");
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", device, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", device, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        addSubInfo( name.value, articles );

        if( overviewInfo_.value.isEmpty() == true )
        {
            overviewInfo_.value = vendor.value;
            overviewInfo_.value += " ";
            overviewInfo_.value += name.value;
        }
    }
}

