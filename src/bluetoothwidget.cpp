#include "bluetoothwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE


BluetoothWidget::BluetoothWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Bluetooth"))
{
    initWidget();
}

void BluetoothWidget::initWidget()
{


    QStringList bluetoothNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Mac Address"),
                            DApplication::translate("Main", "Link Policy"),
                            DApplication::translate("Main", "Link Mode")
                        };

    QStringList bluetoothList = DeviceInfoParserInstance.getBluetoothList();

    if( bluetoothList.size() < 1 )
    {
        setTitle("No Bluetooth Device found!");
        addStrecch();
        return;
    }

    QStringList hciconfigBluetoothList = DeviceInfoParserInstance.getHciconfigBluetoothList();
    setTitle(DApplication::translate("Main", "Bluetooth") + " " + DApplication::translate("Main", " Info"));

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int i = 0;
    foreach(const QString& device, bluetoothList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        ArticleStruct vendor("Vendor");

        if( i < hciconfigBluetoothList.size() )
        {
            QString hciconfigName = hciconfigBluetoothList.at(i);

            name.queryData("hciconfig", hciconfigName, "Name");
            name.value.remove("\'");
            articles.push_back(name);
            existArticles.insert("Name");

            vendor.queryData("hciconfig", hciconfigName, "Manufacturer");
            articles.push_back(vendor);
            existArticles.insert("Manufacturer");

            ArticleStruct mac("Mac Address");
            mac.queryData("hciconfig", hciconfigName, "BD Address");
            articles.push_back(mac);
            existArticles.insert("BD Address");

            ArticleStruct linkPolicy("Link Policy");
            linkPolicy.queryData("hciconfig", hciconfigName, "Link policy");
            articles.push_back(linkPolicy);
            existArticles.insert("Link policy");

            ArticleStruct linkMode("Link Mode");
            linkMode.queryData("hciconfig", hciconfigName, "Link mode");
            articles.push_back(linkMode);
            existArticles.insert("Link mode");

            DeviceInfoParserInstance.queryRemainderDeviceInfo("hciconfig", hciconfigName, articles, existArticles);
        }

        existArticles.clear();

        if(vendor.value.isEmpty())
        {
            vendor.queryData("lshw", device, "vendor");
        }
        existArticles.insert("vendor");

        ArticleStruct product("Product");
        product.queryData( "lshw", device, "product");
        articles.push_back(product);
        existArticles.insert("product");

        ArticleStruct description("Description");
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct physicalId("Physical id");
        physicalId.queryData("lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct busInfo("Bus Info");
        busInfo.queryData("lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData("lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData("lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        addSubInfo( "", articles);

        if( bluetoothList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(i == 0)
        {
            overviewInfo_.value = vendor.value;
            overviewInfo_.value += " ";
            if( product.value.isEmpty() == false && product.value != DApplication::translate("Main", "Unknown") )
            {
                overviewInfo_.value += product.value;
            }
            else
            {
                overviewInfo_.value += description.value;
            }
        }

        ++i;
    }

    if( bluetoothList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

