#include "camerawidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CameraWidget::CameraWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Camera"))
{
    initWidget();
}

void CameraWidget::initWidget()
{
    QStringList cameraList = DeviceInfoParserInstance.getCameraList();

    if( cameraList.size() < 1 )
    {
        setTitle("No Camera found!");
        addStrecch();
        return;
    }

    setTitle(DApplication::translate("Main", "Camera")  + DApplication::translate("Main", " Info"));

    QStringList cameraNames = {
                            DApplication::translate("Main", "Name"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Capabilities")
                        };

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, cameraList)
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

        if( cameraList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if( overviewInfo_.value.isEmpty() == true )
        {
            overviewInfo_.value = vendor.value;
            overviewInfo_.value += " ";
            if( name.value.isEmpty() == false && name.value != DApplication::translate("Main", "Unknown") )
            {
                overviewInfo_.value += name.value;
            }
            else
            {
                overviewInfo_.value += description.value;
            }
        }
    }

    if( cameraList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

