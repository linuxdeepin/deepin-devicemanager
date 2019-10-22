#include "usbdevicewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

UsbdeviceWidget::UsbdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Usb Device"))
{
    initWidget();
}

void UsbdeviceWidget::initWidget()
{
    QStringList names = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities"),
                            DApplication::translate("Main", "configuration")
                        };

    QStringList usbdeviceList = DeviceInfoParserInstance.getOtherUsbdeviceList();

    if( usbdeviceList.size() < 1)
    {
        QStringList emptyList;
        addInfo("No Usb Device found!", emptyList, emptyList);
        addStrecch();
        return;
    }

//    if( usbdeviceList.size() == 1 )
//    {
//        setTitle(DApplication::translate("Main", "Usb Device"));
//    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, usbdeviceList)
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

        ArticleStruct busInfo("Bus info");
        busInfo.queryData( "lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct physicalId("Physical ID");
        physicalId.queryData( "lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct logicalName("Logical Name");
        logicalName.queryData( "lshw", device, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

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

        QString title = (name.value.isEmpty() == false && name.value != DApplication::translate("Main", "Unknown"))? name.value: description.value;
        addSubInfo( title, articles );

        QStringList tab =
        {
            title,
            vendor.value
        };

        tabList.push_back(tab);
    }

    if( usbdeviceList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

