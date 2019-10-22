#include "powerwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PowerWidget::PowerWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Power"))
{
    initWidget();
}

void PowerWidget::initWidget()
{
    QStringList switchingpowerList = DeviceInfoParserInstance.getSwitchingpowerList();
    QStringList demidecodeSwitchingpowerList = DeviceInfoParserInstance.getDemidecodeSwitchingpowerList();

    QStringList batteryList = DeviceInfoParserInstance.getBatteryList();
    QStringList demidecodebatteryList = DeviceInfoParserInstance.getDemidecodeBatteryList();

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    for(int i = 0; i < switchingpowerList.size(); ++i )
    {
        QString device = switchingpowerList[i];
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "lshw", device, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description("Description");
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct ssdelay("Screen Suspend Delay");
        ssdelay.value = DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_) + " Secs";
        articles.push_back(ssdelay);

        ArticleStruct csdelay("Computer Suspend Delay");
        csdelay.value = DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_) + " Secs";
        articles.push_back(csdelay);

        ArticleStruct asdelay("AutoLock Screen Delay");
        asdelay.value = DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_) + " Secs";
        articles.push_back(asdelay);

        ArticleStruct physicalId("Physical ID");
        physicalId.queryData( "lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct version("Version");
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capacity("Capacity");
        capacity.queryData( "lshw", device, "capacity");
        articles.push_back(capacity);
        existArticles.insert("capacity");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        if(i < demidecodeSwitchingpowerList.size())
        {
            QString demideSwitchingpower = demidecodeSwitchingpowerList[i];
            existArticles.insert("Name");
            existArticles.insert("Manufacturer");
            existArticles.insert("Serial Number");
            existArticles.insert("Name");

            ArticleStruct location("Location");
            location.queryData( "dmidecode", demideSwitchingpower, "Location");
            articles.push_back(location);
            existArticles.insert("Location");

            ArticleStruct assetTag("Asset Tag");
            assetTag.queryData( "dmidecode", demideSwitchingpower, "Asset Tag");
            articles.push_back(assetTag);
            existArticles.insert("Asset Tag");
            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demideSwitchingpower, articles, existArticles);
        }

        addSubInfo( "Switching Power", articles );

        if( switchingpowerList.size() + batteryList.size() > 1 )
        {
            QStringList tab =
            {
                DApplication::translate("Main", "Switching Power"),
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }


    for( int i = 0; i < batteryList.size(); ++i )
    {
        QString device = batteryList[i];

        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "lshw", device, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description("Description");
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct ssdelay("Screen Suspend Delay");
        ssdelay.value = DeviceInfoParserInstance.batteryScreenSuspendDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.batteryScreenSuspendDelay_) + " Secs";
        articles.push_back(ssdelay);

        ArticleStruct csdelay("Computer Suspend Delay");
        csdelay.value = DeviceInfoParserInstance.batteryComputerSuspendDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.batteryComputerSuspendDelay_) + " Secs";
        articles.push_back(csdelay);

        ArticleStruct asdelay("AutoLock Screen Delay");
        asdelay.value = DeviceInfoParserInstance.batteryAutoLockScreenDelay_==0? "Never" \
            : QString::number(DeviceInfoParserInstance.batteryAutoLockScreenDelay_) + " Secs";
        articles.push_back(asdelay);

        ArticleStruct physicalId("Physical ID");
        physicalId.queryData( "lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct version("Version");
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capacity("Capacity");
        capacity.queryData( "lshw", device, "capacity");
        articles.push_back(capacity);
        existArticles.insert("capacity");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        if(i < demidecodeSwitchingpowerList.size())
        {
            QString demideSwitchingpower = demidecodeSwitchingpowerList[i];
            existArticles.insert("Name");
            existArticles.insert("Manufacturer");
            existArticles.insert("Serial Number");
            existArticles.insert("Name");

            ArticleStruct location("Location");
            location.queryData( "dmidecode", demideSwitchingpower, "Location");
            articles.push_back(location);
            existArticles.insert("Location");

            ArticleStruct assetTag("Asset Tag");
            assetTag.queryData( "dmidecode", demideSwitchingpower, "Asset Tag");
            articles.push_back(assetTag);
            existArticles.insert("Asset Tag");
            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demideSwitchingpower, articles, existArticles);
        }

        addSubInfo( "Battery", articles );

        if( switchingpowerList.size() + batteryList.size() > 1 )
        {
            QStringList tab =
            {
                DApplication::translate("Main", "Battery"),
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if( switchingpowerList.size() + batteryList.size() > 1 )
    {
        QStringList headers = { "Type", "Name", "Vendor" };
        addTable( headers, tabList);
    }
}
