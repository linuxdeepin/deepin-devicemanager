#include "memorywidget.h"
#include "deviceinfoparser.h"
#include "DTableWidget"
#include <QVBoxLayout>
#include "tablewidgetalwaysfocus.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Memory"))
{
    initWidget();    
}

void MemoryWidget::initWidget()
{
    initTableWdiget();
    updateWholeDownWidget();
}

void MemoryWidget::initTableWdiget()
{
    QStringList memList = DeviceInfoParserInstance.getMemorynameList();

    if( memList.size() < 2 )
    {
        return;
    }

    canUpgrade_ = false;
    QStringList headers = { "Bank",  "Size", "Type", "Speed", "Statu"};

    QList<QStringList> tabList;

    foreach(const QString& mem, memList)
    {
        QString size = DeviceInfoParserInstance.queryData("dmidecode", mem, "Size");
        if( size == DApplication::translate("Main", "Unknown") || size == "No Module Installed" )
        {
            canUpgrade_ = true;
        }

        QStringList tab = {
            DeviceInfoParserInstance.queryData("dmidecode", mem, "Locator"),
            DeviceInfoParserInstance.queryData("dmidecode", mem, "Size"),
            DeviceInfoParserInstance.queryData("dmidecode", mem, "Type"),
            DeviceInfoParserInstance.queryData("dmidecode", mem, "Speed"),
            (  size == DApplication::translate("Main", "Unknown")    \
            || size ==  "No Module Installed" ) ? DApplication::translate("Main", "Bad"):DApplication::translate("Main", "Good")
        };

        tabList.push_back(tab);
    }

    addTable(headers, tabList);
}

void MemoryWidget::updateWholeDownWidget()
{
    setTitle(DApplication::translate("Main", "Memory")  + DApplication::translate("Main", " Info"));

    QStringList names = {   "Slot Count",
                            "Size",
                            "Maximum Capacity",
                            "Upgradeable"
                        };

    QString memSize = DeviceInfoParserInstance.queryData("lshw", "Computer_core_memory", "size");
    memSize.replace( "GiB", " GB" );

    QStringList contents = {
        DeviceInfoParserInstance.queryData("dmidecode", "Physical Memory Array", "Number Of Devices"),
        memSize,
        DeviceInfoParserInstance.queryData("dmidecode", "Physical Memory Array", "Maximum Capacity"),
        canUpgrade_ ? DApplication::translate("Main", "Yes") : DApplication::translate("Main", "No")
    };

    addInfo(names, contents);

    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList memList = DeviceInfoParserInstance.getMemorynameList();
    QStringList detailMem;
    foreach(const QString& mem, memList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct vendor("Vendor");
        vendor.queryData("dmidecode", mem, "Manufacturer");
        articles.push_back(vendor);
        existArticles.insert("Manufacturer");

        ArticleStruct size("Size");
        size.queryData("dmidecode", mem, "Size");
        articles.push_back(size);
        existArticles.insert("Size");

        ArticleStruct type("Type");
        type.queryData("dmidecode", mem, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct speed("Speed");
        speed.queryData("dmidecode", mem, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct serial("Serial Number");
        serial.queryData("dmidecode", mem, "Serial Number");
        articles.push_back(serial);
        existArticles.insert("Serial Number");

        ArticleStruct model("Model");
        model.queryData("dmidecode", mem, "Part Number");
        articles.push_back(model);
        existArticles.insert("Part Number");

        ArticleStruct configVoltage("Configured Voltage");
        configVoltage.queryData("dmidecode", mem, "Configured Voltage");
        articles.push_back(configVoltage);
        existArticles.insert("Configured Voltage");

        ArticleStruct minVoltage("Minimum Voltage");
        minVoltage.queryData("dmidecode", mem, "Minimum Voltage");
        articles.push_back(minVoltage);
        existArticles.insert("Minimum Voltage");

        ArticleStruct maxVoltage("Maximum Voltage");
        maxVoltage.queryData("dmidecode", mem, "Maximum Voltage");
        articles.push_back(maxVoltage);
        existArticles.insert("Maximum Voltage");

        ArticleStruct rank("Rank");
        rank.queryData("dmidecode", mem, "Rank");
        articles.push_back(rank);
        existArticles.insert("Rank");

        ArticleStruct configSpeed("Configured Speed");
        configSpeed.queryData("dmidecode", mem, "Configured Memory Speed");
        articles.push_back(configSpeed);
        existArticles.insert("Configured Memory Speed");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);

        addSubInfo(DeviceInfoParserInstance.queryData("dmidecode", mem, "Locator"), articles );

        if(rank.value == DApplication::translate("Main", "Unknown")|| rank.value == "Unknown" )
        {
            continue;
        }

        QString overviewVendor = vendor.value;
        overviewVendor += " ";
        overviewVendor += type.value;
        overviewVendor += " ";
        overviewVendor += speed.value;

        if( false == detailMem .contains(overviewVendor) )
        {
            detailMem.push_back(overviewVendor);
        }
    }

    overviewInfo_.value = memSize;
    if( detailMem.size() > 0 )
    {
        overviewInfo_.value += " (";
        overviewInfo_.value += detailMem.join("/");
        overviewInfo_.value += ")";
    }
}
