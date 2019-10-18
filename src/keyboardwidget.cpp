#include "keyboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Keyboard"))
{
    initWidget();
}

void KeyboardWidget::initWidget()
{
    QStringList inputdeviceList = DeviceInfoParserInstance.getKeyboardInputdeviceList();
    QStringList keyboardList = DeviceInfoParserInstance.getKeyboardList();

    if( inputdeviceList.size() + keyboardList.size() < 1 )
    {
        setTitle("No Keyboard found!");
        addStrecch();
        return;
    }

//    if( inputdeviceList.size() + keyboardList.size() == 1)
//    {
//        setTitle(DApplication::translate("Main", "Keyboard")  + DApplication::translate("Main", " Info"));
//    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "catinput", device, "Name");
        name.value.remove("\"");
        articles.push_back(name);
        existArticles.insert("Name");

        ArticleStruct type("Type");
        type.value = "PS/2";
        articles.push_back(type);

        ArticleStruct vendor("Vendor");
        vendor.queryData( "catinput", device, "Vendor");
        articles.push_back(vendor);
        existArticles.insert("Vendor");

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        ArticleStruct phys("Phys");
        phys.queryData( "catinput", device, "Phys");
        articles.push_back(phys);
        existArticles.insert("Phys");

        ArticleStruct vesion("Version");
        vesion.queryData( "catinput", device, "Version");
        articles.push_back(vesion);
        existArticles.insert("Version");

        ArticleStruct product("Product");
        product.queryData( "catinput", device, "Product");
        articles.push_back(product);
        existArticles.insert("Product");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("catinput", device, articles, existArticles);

        addSubInfo( name.value, articles);

//        overviewInfo_.value = "PS/2 ";
//        overviewInfo_.value += vendor.value;
//        overviewInfo_.value += " ";
        overviewInfo_.value += name.value;

        if( inputdeviceList.size() + keyboardList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                "PS/2",
                vendor.value
            };

            tabList.push_back(tab);
        }
    }


    foreach(const QString& keyboard, keyboardList)
    {
        articles.clear();
        existArticles.clear();

//        ArticleStruct type("Type");
//        type.value = "AudioAdapter";
//        articles.push_back(type);

        ArticleStruct name("Name");
        name.queryData( "lshw", keyboard, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description("Description");
        description.queryData("lshw", keyboard, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", keyboard, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo("Bus info");
        busInfo.queryData( "lshw", keyboard, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData( "lshw", keyboard, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", keyboard, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", keyboard, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", keyboard, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", keyboard, articles, existArticles);
        addSubInfo( name.value , articles );

        QStringList tab =
        {
            name.value,
            description.value,
            vendor.value
        };

        tabList.push_back(tab);

        overviewInfo_.value += " / ";
        overviewInfo_.value += name.value;
    }

    if( inputdeviceList.size() + keyboardList.size() > 1 )
    {
        QStringList headers = { "Name",  "Type", "Vendor" };
        addTable( headers, tabList);
    }
}

