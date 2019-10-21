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
    setTitle( DApplication::translate("Main", "DisplayAdapter")  + DApplication::translate("Main", " Info") );

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList displayadapterList = DeviceInfoParserInstance.getDiaplayadapterList();

    foreach(const QString& displayadapter, displayadapterList)
    {
        QString pci_bus = DeviceInfoParserInstance.queryData("lshw", displayadapter, "bus info");
        QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");
        if(reg.exactMatch(pci_bus))
        {
            pci_bus= reg.cap(1);
        }

        QString lspciDeviceName;
        DeviceInfoParserInstance.fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

        QString lspciName = DeviceInfoParserInstance.fuzzyQueryData("lspci", lspciDeviceName, "bus info");
        ArticleStruct name("Name");
        name.queryData("lspci", lspciDeviceName, "Name");
        name.value.remove( " Corporation", Qt::CaseInsensitive );

        int index = name.value.indexOf('(');
        if(index > 0)
        {
            name.value = name.value.mid(0, index);
        }
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct memory("Graphic Memory");
        memory.queryData("lspci", lspciDeviceName, "Memory");
        articles.push_back(memory);
        existArticles.insert("Memory");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", displayadapter, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct driver("Driver");
        driver.queryData("lspci", lspciDeviceName, "Kernel modules");
        articles.push_back(driver);
        existArticles.insert("driver");

//        ArticleStruct interface("Support Interface");
//        interface.value = DeviceInfoParserInstance.getDisplayInterfaceList().join(", ");
//        articles.push_back(interface);

        ArticleStruct version("Version");
        version.queryData("lshw", displayadapter, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData("lshw", displayadapter, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData("lshw", displayadapter, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData("lshw", displayadapter, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", displayadapter, articles, existArticles);

        addSubInfo( "", articles);

        QStringList tab =
        {
            name.value,
            vendor.value
        };

        tabList.push_back(tab);

        if( overviewInfo_.value.isEmpty() == true)
        {
            overviewInfo_.value = name.value;

            overviewInfo_.value += " (";
            overviewInfo_.value += memory.value;

            if(DApplication::translate("Main", "Unknown") != vendor.value)
            {
                overviewInfo_.value += " / ";
                overviewInfo_.value += vendor.value;
            }

            overviewInfo_.value += ")";
        }
    }

    if(displayadapterList.size() > 1)
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

