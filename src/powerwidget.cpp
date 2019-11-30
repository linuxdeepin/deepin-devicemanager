/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    QStringList lshwSwitchingpowerList = DeviceInfoParserInstance.getLshwSwitchingpowerList();
    QStringList demidecodeSwitchingpowerList = DeviceInfoParserInstance.getDemidecodeSwitchingpowerList();
    QStringList upowerSwitchingList = DeviceInfoParserInstance.getUpowerSwitchingList();

    QStringList lshwBatteryList = DeviceInfoParserInstance.getLshwBatteryList();
    QStringList demidecodebatteryList = DeviceInfoParserInstance.getDemidecodeBatteryList();
    QStringList UpowerBatteryList = DeviceInfoParserInstance.getUpowerBatteryList();

    int maxSwitchingSize = maxDeviceSize(lshwSwitchingpowerList, demidecodeSwitchingpowerList, upowerSwitchingList);
    int maxBatterySize = maxDeviceSize(lshwBatteryList, demidecodebatteryList, UpowerBatteryList);

    if(maxSwitchingSize + maxBatterySize < 1)
    {
        setCentralInfo("Get Power Infomation failed!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles1;
    QSet<QString> existArticles2;
    QSet<QString> existArticles3;

    for(int i = 0; i < maxSwitchingSize; ++i )
    {
        articles.clear();
        existArticles1.clear();
        existArticles2.clear();
        existArticles3.clear();

//        ArticleStruct ssdelay("Screen Suspend Delay");
//        ssdelay.value = DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.switchingpowerScreenSuspendDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(ssdelay);

//        ArticleStruct csdelay("Computer Suspend Delay");
//        csdelay.value = DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.switchingpowerComputerSuspendDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(csdelay);

//        ArticleStruct asdelay("AutoLock Screen Delay");
//        asdelay.value = DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.switchingpowerAutoLockScreenDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(asdelay);

        ArticleStruct name("Name");
        ArticleStruct vendor("Vendor");
        ArticleStruct device("Device");
        ArticleStruct serial("Serial");

        if(i < upowerSwitchingList.size())
        {
            device.value = upowerSwitchingList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor", existArticles1, articles);

            serial.queryData( "upower", device.value, "serial", existArticles1, articles);

            ArticleStruct model("Model");
            model.queryData( "upower", device.value, "model", existArticles1, articles);

            ArticleStruct powerSupply("Power Supply");
            powerSupply.queryData( "upower", device.value, "power supply", existArticles1, articles);
        }

        if(i < lshwSwitchingpowerList.size())
        {
            QString dname = lshwSwitchingpowerList[i];

            name.queryData( "lshw", dname, "product", existArticles2, articles);

            vendor.queryData( "lshw", dname, "vendor", existArticles2, articles);

            ArticleStruct description("Description");
            description.queryData("lshw", dname, "description", existArticles2, articles);

            serial.queryData( "upower", dname, "serial", existArticles2, articles);

            ArticleStruct physicalId("Physical ID");
            physicalId.queryData( "lshw", dname, "physical id", existArticles2, articles);

            ArticleStruct version("Version");
            version.queryData( "lshw", dname, "version", existArticles2, articles);

            ArticleStruct capacity("Capacity");
            capacity.queryData( "lshw", dname, "capacity", existArticles2, articles);
        }

        if(i < demidecodeSwitchingpowerList.size())
        {
            QString demideSwitchingpower = demidecodeSwitchingpowerList[i];

            name.queryData( "dmidecode", demideSwitchingpower, "Name", existArticles3, articles);

            vendor.queryData( "dmidecode", demideSwitchingpower, "Manufacturer", existArticles3, articles);

            serial.queryData( "dmidecode", demideSwitchingpower, "Serial Number", existArticles3, articles);

            ArticleStruct maxcapacity("Max Power Capacity");
            maxcapacity.queryData( "dmidecode", demideSwitchingpower, "Max Power Capacity", existArticles3, articles);


            ArticleStruct location("Location");
            location.queryData( "dmidecode", demideSwitchingpower, "Location", existArticles3, articles);

            ArticleStruct assetTag("Asset Tag");
            assetTag.queryData( "dmidecode", demideSwitchingpower, "Asset Tag", existArticles3, articles);
        }

        if(i < upowerSwitchingList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("upower", device.value, articles, existArticles1);
        }

        if(i < lshwSwitchingpowerList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", lshwSwitchingpowerList[i], articles, existArticles2);
        }

        if(i < demidecodeSwitchingpowerList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demidecodeSwitchingpowerList[i], articles, existArticles3);
        }

        addDevice( "Switching Power", articles, maxSwitchingSize + maxBatterySize, true );

        if( maxSwitchingSize + maxBatterySize > 1 )
        {
            QStringList tab =
            {
                name.value,
                DApplication::translate("Main", "Switching Power"),
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += DApplication::translate("Main", "Switching Power") + " " + name.value;
    }


    for( int i = 0; i < maxBatterySize; ++i )
    {
        existArticles1.clear();
        existArticles2.clear();
        existArticles3.clear();
        articles.clear();
//        ArticleStruct ssdelay("Screen Suspend Delay");
//        ssdelay.value = DeviceInfoParserInstance.batteryScreenSuspendDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.batteryScreenSuspendDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(ssdelay);

//        ArticleStruct csdelay("Computer Suspend Delay");
//        csdelay.value = DeviceInfoParserInstance.batteryComputerSuspendDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.batteryComputerSuspendDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(csdelay);

//        ArticleStruct asdelay("AutoLock Screen Delay");
//        asdelay.value = DeviceInfoParserInstance.batteryAutoLockScreenDelay_==0? DApplication::translate("Main", "Never")
//            : QString::number(DeviceInfoParserInstance.batteryAutoLockScreenDelay_) + DApplication::translate("Main", " Secs later");
//        articles.push_back(asdelay);

        ArticleStruct vendor("Vendor");
        ArticleStruct de("Design Energy");
        ArticleStruct device("Device");
        ArticleStruct model("Model");
        ArticleStruct name("Name");
        ArticleStruct serial("Serial");

        if(i < UpowerBatteryList.size())
        {
            device.value = UpowerBatteryList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor", existArticles1, articles);

            model.queryData( "upower", device.value, "model", existArticles1, articles);

            serial.queryData( "upower", device.value, "serial", existArticles1, articles);

            ArticleStruct state("State");
            state.queryData( "upower", device.value, "state", existArticles1, articles);

            ArticleStruct percentage("Battery Percentage");
            percentage.queryData( "upower", device.value, "percentage", existArticles1, articles);

            ArticleStruct ce("Current Energy");
            ce.queryData( "upower", device.value, "energy", existArticles1, articles);

            ArticleStruct fe("Full Energy");
            fe.queryData( "upower", device.value, "energy-full", existArticles1, articles);

            de.queryData( "upower", device.value, "energy-full-design", existArticles1, articles);

            ArticleStruct bh("Battery Healthy");
            bh.queryData( "upower", device.value, "capacity", existArticles1, articles);

            ArticleStruct powerSupply("Power Supply");
            powerSupply.queryData( "upower", device.value, "power supply", existArticles1, articles);
        }

        if( i < lshwBatteryList.size())
        {
            QString device = lshwBatteryList[i];

            name.queryData( "lshw", device, "product", existArticles2, articles);
            articles.push_back(name);

            vendor.queryData( "lshw", device, "vendor", existArticles2, articles);

            ArticleStruct description("Description");
            description.queryData("lshw", device, "description", existArticles2, articles);

            ArticleStruct physicalId("Physical ID");
            physicalId.queryData( "lshw", device, "physical id", existArticles2, articles);

            ArticleStruct version("Version");
            version.queryData( "lshw", device, "version", existArticles2, articles);


            ArticleStruct capacity("Capacity");
            capacity.queryData( "lshw", device, "capacity", existArticles2, articles);


            ArticleStruct slot("Location");
            slot.queryData( "dmidecode", device, "slot");
            articles.push_back(slot);
            if(slot.value.isEmpty() == false || slot.value != DApplication::translate("Main", "Unknown"))
            {
                existArticles2.insert("Location");
            }
            if(slot.isValid())
            {
                existArticles2.insert("slot");
            }
        }

        if(i < demidecodebatteryList.size())
        {
            QString demideBattery = demidecodebatteryList[i];

            name.queryData( "dmidecode", demideBattery, "Name", existArticles3, articles);

            vendor.queryData( "dmidecode", demideBattery, "Manufacturer", existArticles3, articles);

            serial.queryData( "dmidecode", demideBattery, "Serial Number", existArticles3, articles);

//            existArticles.insert("Name");
//            existArticles.insert("Manufacturer");
//            existArticles.insert("Serial Number");
//            existArticles.insert("Name");

            ArticleStruct location("Location");
            location.queryData( "dmidecode", demideBattery, "Location", existArticles3, articles);

            ArticleStruct assetTag("Asset Tag");
            assetTag.queryData( "dmidecode", demideBattery, "Asset Tag", existArticles3, articles);

            de.queryData( "dmidecode", demideBattery, "Design Capacity", existArticles3, articles);
        }

        if(i < UpowerBatteryList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("upower", device.value, articles, existArticles1);
        }

        if(i < lshwBatteryList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", lshwBatteryList[i], articles, existArticles2);
        }

        if(i < demidecodebatteryList.size())
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", demidecodebatteryList[i], articles, existArticles3);
        }

        QString d_name = device.value;

        if( maxSwitchingSize + maxBatterySize > 1 )
        {
            if(device.isValid() == false)
            {
                d_name = name.value;
            }

            QStringList tab =
            {
                d_name,
                DApplication::translate("Main", "Battery"),
                vendor.value
            };

            tabList.push_back(tab);
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

        addDevice( DApplication::translate("Main", "Battery") + " " + overName, articles, maxSwitchingSize + maxBatterySize, true );

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += DApplication::translate("Main", "Battery") + " " + overName;
    }

    if( maxSwitchingSize + maxBatterySize > 1 )
    {
        QStringList headers = { "Name", "Type", "Vendor" };
        addTable( headers, tabList);
    }
}
