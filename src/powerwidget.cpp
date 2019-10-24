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

        ArticleStruct ssdelay("Screen Suspend Delay");
        ssdelay.value = DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(ssdelay);

        ArticleStruct csdelay("Computer Suspend Delay");
        csdelay.value = DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(csdelay);

        ArticleStruct asdelay("AutoLock Screen Delay");
        asdelay.value = DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(asdelay);

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

            ArticleStruct maxcapacity("Max Power Capacity");
            maxcapacity.queryData( "dmidecode", demideSwitchingpower, "Max Power Capacity");
            articles.push_back(maxcapacity);
            existArticles.insert("Max Power Capacity");

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

        overviewInfo_.value = DApplication::translate("Main", "Switching Power") + " " + name.value;
    }


    for( int i = 0; i < batteryList.size(); ++i )
    {
        QString device = batteryList[i];

        articles.clear();
        existArticles.clear();

        ArticleStruct ssdelay("Screen Suspend Delay");
        ssdelay.value = DeviceInfoParserInstance.batteryScreenSuspendDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.batteryScreenSuspendDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(ssdelay);

        ArticleStruct csdelay("Computer Suspend Delay");
        csdelay.value = DeviceInfoParserInstance.batteryComputerSuspendDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.batteryComputerSuspendDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(csdelay);

        ArticleStruct asdelay("AutoLock Screen Delay");
        asdelay.value = DeviceInfoParserInstance.batteryAutoLockScreenDelay_==0? DApplication::translate("Main", "Never") \
            : QString::number(DeviceInfoParserInstance.batteryAutoLockScreenDelay_) + DApplication::translate("Main", " Secs later");
        articles.push_back(asdelay);

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

        ArticleStruct slot("Location");
        slot.queryData( "dmidecode", device, "slot");
        articles.push_back(slot);
        if(slot.value.isEmpty() == false || slot.value != DApplication::translate("Main", "Unknown"))
        {
            existArticles.insert("Location");
        }
        existArticles.insert("slot");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        if(i < demidecodebatteryList.size())
        {
            QString demideBattery = demidecodebatteryList[i];
            existArticles.insert("Name");
            existArticles.insert("Manufacturer");
            existArticles.insert("Serial Number");
            existArticles.insert("Name");

            ArticleStruct location("Location");
            location.queryData( "dmidecode", demideBattery, "Location");
            articles.push_back(location);
            existArticles.insert("Location");

            ArticleStruct assetTag("Asset Tag");
            assetTag.queryData( "dmidecode", demideBattery, "Asset Tag");
            articles.push_back(assetTag);
            existArticles.insert("Asset Tag");
            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demideBattery, articles, existArticles);
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
        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += DApplication::translate("Main", "Battery") + " " + name.value;
    }

    if( switchingpowerList.size() + batteryList.size() > 1 )
    {
        QStringList headers = { "Type", "Name", "Vendor" };
        addTable( headers, tabList);
    }
}
