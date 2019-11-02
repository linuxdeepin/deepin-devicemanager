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
    QStringList switchingUpowerList = DeviceInfoParserInstance.getSwitchingUpowerList();

    QStringList batteryList = DeviceInfoParserInstance.getBatteryList();
    QStringList demidecodebatteryList = DeviceInfoParserInstance.getDemidecodeBatteryList();
    QStringList batteryUpowerList = DeviceInfoParserInstance.getBatteryUpowerList();

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int maxSwitchingSize = maxDeviceSize(switchingpowerList, demidecodeSwitchingpowerList, switchingUpowerList);
    for(int i = 0; i < maxSwitchingSize; ++i )
    {
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
        ArticleStruct vendor("Vendor");
        ArticleStruct device("Device");

        if(i < switchingUpowerList.size())
        {
            existArticles.clear();

            device.value = switchingUpowerList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor");
            articles.push_back(vendor);
            ArticleStruct Serial("Serial");
            Serial.queryData( "upower", device.value, "serial");
            articles.push_back(Serial);
            existArticles.insert("serial");

            ArticleStruct model("Model");
            model.queryData( "upower", device.value, "model");
            articles.push_back(model);
            existArticles.insert("model");

            ArticleStruct powerSupply("Power Supply");
            powerSupply.queryData( "upower", device.value, "power supply");
            articles.push_back(powerSupply);
            existArticles.insert("power supply");
        }

        if(i < switchingpowerList.size())
        {
            QString device = switchingpowerList[i];

            name.queryData( "lshw", device, "product");
            articles.push_back(name);
            existArticles.insert("product");

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
        }

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
        }

        if(i < switchingUpowerList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("upower", device.value, articles, existArticles);
        }

        if(i < switchingpowerList.size())
        {
            QString device = switchingpowerList[i];
            DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles);
        }

        if(i < demidecodeSwitchingpowerList.size())
        {
            QString demideSwitchingpower = demidecodeSwitchingpowerList[i];

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

    int maxBatterySize = maxDeviceSize(batteryList, demidecodebatteryList, batteryUpowerList);
    for( int i = 0; i < maxBatterySize; ++i )
    {
        articles.clear();
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

        ArticleStruct vendor("Vendor");
        ArticleStruct de("Design Energy");
        ArticleStruct device("Device");
        ArticleStruct model("Model");

        if(i < batteryUpowerList.size())
        {
            existArticles.clear();


            device.value = batteryUpowerList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor");
            articles.push_back(vendor);
            existArticles.insert("vendor");

            model.queryData( "upower", device.value, "model");
            articles.push_back(model);
            existArticles.insert("model");

            ArticleStruct Serial("Serial");
            Serial.queryData( "upower", device.value, "serial");
            articles.push_back(Serial);
            existArticles.insert("serial");

            ArticleStruct state("State");
            state.queryData( "upower", device.value, "state");
            articles.push_back(state);
            existArticles.insert("state");

            ArticleStruct percentage("Battery Percentage");
            percentage.queryData( "upower", device.value, "percentage");
            articles.push_back(percentage);
            existArticles.insert("percentage");

            ArticleStruct ce("Current Energy");
            ce.queryData( "upower", device.value, "energy");
            articles.push_back(ce);
            existArticles.insert("energy");

            ArticleStruct fe("Full Energy");
            fe.queryData( "upower", device.value, "energy-full");
            articles.push_back(fe);
            existArticles.insert("energy-full");

            de.queryData( "upower", device.value, "energy-full-design");
            articles.push_back(de);
            existArticles.insert("energy-full-design");

            ArticleStruct bh("Battery Healthy");
            bh.queryData( "upower", device.value, "capacity");
            articles.push_back(bh);
            existArticles.insert("capacity");

            ArticleStruct powerSupply("Power Supply");
            powerSupply.queryData( "upower", device.value, "power supply");
            articles.push_back(powerSupply);
            existArticles.insert("power supply");

            DeviceInfoParserInstance.queryRemainderDeviceInfo("upower", device.value, articles, existArticles);
        }

        ArticleStruct name("Name");
        if( i < batteryList.size())
        {
            QString device = batteryList[i];

            name.queryData( "lshw", device, "product");
            articles.push_back(name);
            existArticles.insert("product");

            if( vendor.isValid() == false)
            {
                vendor.queryData( "lshw", device, "vendor");
                articles.push_back(vendor);
                existArticles.insert("vendor");
            }

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
        }

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

            if(de.isValid() == false)
            {
                de.queryData( "dmidecode", demideBattery, "Design Capacity");
                articles.push_back(de);
            }
            existArticles.insert("Design Capacity");

            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demideBattery, articles, existArticles);
        }

        QString d_name = device.value;

        if( switchingpowerList.size() + batteryList.size() > 1 )
        {
            if(device.isValid() == false)
            {
                d_name = name.value;
            }

            QStringList tab =
            {
                DApplication::translate("Main", "Battery"),
                d_name,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }

        QString overName;
        if(vendor.isValid())
        {
            overName = vendor.value;
        }
        if(overName.isEmpty() == false)
        {
            overName += " ";
        }
        if(model.isValid())
        {
            overName += model.value;
        }
        else if(name.isValid())
        {
            overName += name.value;
        }
        else if(device.isValid())
        {
            overName += device.value;
        }

        addSubInfo( DApplication::translate("Main", "Battery") + " " + overName, articles );

        overviewInfo_.value += DApplication::translate("Main", "Battery") + " " + overName;
    }

    if( switchingpowerList.size() + batteryList.size() > 1 )
    {
        QStringList headers = { "Type", "Name", "Vendor" };
        addTable( headers, tabList);
    }
}
