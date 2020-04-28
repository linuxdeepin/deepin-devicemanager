/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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

PowerWidget::PowerWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Power"))
{
    initWidget();
}

void PowerWidget::initWidget()
{
    QStringList lshwSwitchingpowerList = DeviceInfoParser::Instance().getLshwSwitchingpowerList();
    QStringList demidecodeSwitchingpowerList = DeviceInfoParser::Instance().getDemidecodeSwitchingpowerList();
    QStringList upowerSwitchingList = DeviceInfoParser::Instance().getUpowerSwitchingList();

    QStringList lshwBatteryList = DeviceInfoParser::Instance().getLshwBatteryList();
    QStringList demidecodebatteryList = DeviceInfoParser::Instance().getDemidecodeBatteryList();
    QStringList UpowerBatteryList = DeviceInfoParser::Instance().getUpowerBatteryList();

    int maxSwitchingSize = maxDeviceSize(lshwSwitchingpowerList, demidecodeSwitchingpowerList, upowerSwitchingList);
    int maxBatterySize = maxDeviceSize(lshwBatteryList, demidecodebatteryList, UpowerBatteryList);

    if(maxSwitchingSize + maxBatterySize < 1)
    {
        setCentralInfo(tr("Failed to get power information"));
        return ;
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

        ArticleStruct name(tr("Name"));
        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct device(tr("Device"));
        ArticleStruct serial(tr("Serial Number"));

        if(i < upowerSwitchingList.size())
        {
            device.value = upowerSwitchingList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor", existArticles1, articles);

            serial.queryData( "upower", device.value, "serial", existArticles1, articles);

            ArticleStruct model(tr("Model"));
            model.queryData( "upower", device.value, "model", existArticles1, articles);

            ArticleStruct powerSupply(tr("Power Supply"));
            powerSupply.queryData( "upower", device.value, "power supply", existArticles1, articles);

            ArticleStruct device1(tr("Device"));
            device1.queryData( "upower", device.value, "Device", existArticles1, articles);

            ArticleStruct criticalAction(tr("Critical Action"));
            criticalAction.queryData( "upower", device.value, "critical-action", existArticles1, articles);

            ArticleStruct daemonVersion(tr("Daemon Version"));
            daemonVersion.queryData( "upower", device.value, "daemon-version", existArticles1, articles);

            ArticleStruct hasHistory(tr("Has History"));
            hasHistory.queryData( "upower", device.value, "has history", existArticles1, articles);

            ArticleStruct hasStatistics(tr("Has Statistics"));
            hasStatistics.queryData( "upower", device.value, "has statistics", existArticles1, articles);

            ArticleStruct iconName(tr("Icon Name"));
            iconName.queryData( "upower", device.value, "icon-name", existArticles1, articles);

            ArticleStruct lidIsClosed(tr("Lid Is Closed"));
            lidIsClosed.queryData( "upower", device.value, "lid-is-closed", existArticles1, articles);

            ArticleStruct lidIsPresent(tr("Lid Is Present"));
            lidIsPresent.queryData( "upower", device.value, "lid-is-present", existArticles1, articles);

            ArticleStruct onbattery(tr("On Battery"));
            onbattery.queryData( "upower", device.value, "on-battery", existArticles1, articles);

            ArticleStruct updated(tr("Updated"));
            updated.queryData( "upower", device.value, "updated", existArticles1, articles);

            ArticleStruct warningLevel(tr("Warning Level"));
            warningLevel.queryData( "upower", device.value, "warning-level", existArticles1, articles);
        }

        if(i < lshwSwitchingpowerList.size())
        {
            QString dname = lshwSwitchingpowerList[i];

            name.queryData( "lshw", dname, "product", existArticles2, articles);

            vendor.queryData( "lshw", dname, "vendor", existArticles2, articles);

            ArticleStruct description(tr("Description"));
            description.queryData("lshw", dname, "description", existArticles2, articles);

            serial.queryData( "upower", dname, "serial", existArticles2, articles);

            ArticleStruct physicalId(tr("Physical ID"));
            physicalId.queryData( "lshw", dname, "physical id", existArticles2, articles);

            ArticleStruct version(tr("Version"));
            version.queryData( "lshw", dname, "version", existArticles2, articles);

            ArticleStruct capacity(tr("Capacity"));
            capacity.queryData( "lshw", dname, "capacity", existArticles2, articles);

        }

        if(i < demidecodeSwitchingpowerList.size())
        {
            QString demideSwitchingpower = demidecodeSwitchingpowerList[i];

            name.queryData( "dmidecode", demideSwitchingpower, "Name", existArticles3, articles);

            vendor.queryData( "dmidecode", demideSwitchingpower, "Manufacturer", existArticles3, articles);

            serial.queryData( "dmidecode", demideSwitchingpower, "Serial Number", existArticles3, articles);

            ArticleStruct maxcapacity(tr("Max Power Capacity"));
            maxcapacity.queryData( "dmidecode", demideSwitchingpower, "Max Power Capacity", existArticles3, articles);


            ArticleStruct location(tr("Location"));
            location.queryData( "dmidecode", demideSwitchingpower, "Location", existArticles3, articles);

            ArticleStruct assetTag(tr("Asset Tag","Power Info from DMIDecode"));
            assetTag.queryData( "dmidecode", demideSwitchingpower, "Asset Tag", existArticles3, articles);
        }

        if(i < upowerSwitchingList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", device.value, articles, existArticles1,"ManulTrack__POWER","Power information");
        }

        if(i < lshwSwitchingpowerList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", lshwSwitchingpowerList[i], articles, existArticles2,"ManulTrack__POWER","Power information");
        }

        if(i < demidecodeSwitchingpowerList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", demidecodeSwitchingpowerList[i], articles, existArticles3,"ManulTrack__POWER","Power information");
        }

        addDevice( "Switching Power", articles, maxSwitchingSize + maxBatterySize, true );

        if( maxSwitchingSize + maxBatterySize > 1 )
        {
            QStringList tab =
            {
                name.value,
                tr("Switching Power"),
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += tr("Switching Power") + " " + name.value;
    }


    for( int i = 0; i < maxBatterySize; ++i )
    {
        existArticles1.clear();
        existArticles2.clear();
        existArticles3.clear();
        articles.clear();

        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct de(tr("Design Energy","Power Info from Upower"));
        ArticleStruct device(tr("Device"));
        ArticleStruct model(tr("Model"));
        ArticleStruct name(tr("Name"));
        ArticleStruct serial(tr("Serial Number"));

        if(i < UpowerBatteryList.size())
        {
            device.value = UpowerBatteryList[i];
            articles.push_back(device);

            vendor.queryData( "upower", device.value, "vendor", existArticles1, articles);

            model.queryData( "upower", device.value, "model", existArticles1, articles);

            serial.queryData( "upower", device.value, "serial", existArticles1, articles);

            ArticleStruct state(tr("State"));
            state.queryData( "upower", device.value, "state", existArticles1, articles);

            ArticleStruct percentage(tr("Battery Percentage"));
            percentage.queryData( "upower", device.value, "percentage", existArticles1, articles);

            ArticleStruct ce(tr("Current Energy"));
            ce.queryData( "upower", device.value, "energy", existArticles1, articles);

            ArticleStruct fe(tr("Full Energy"));
            fe.queryData( "upower", device.value, "energy-full", existArticles1, articles);

            de.queryData( "upower", device.value, "energy-full-design", existArticles1, articles);

            ArticleStruct bh(tr("Battery Healthy"));
            bh.queryData( "upower", device.value, "capacity", existArticles1, articles);

            ArticleStruct powerSupply(tr("Power Supply"));
            powerSupply.queryData( "upower", device.value, "power supply", existArticles1, articles);
        }

        if( i < lshwBatteryList.size())
        {
            QString device = lshwBatteryList[i];

            name.queryData( "lshw", device, "product", existArticles2, articles);
            articles.push_back(name);

            vendor.queryData( "lshw", device, "vendor", existArticles2, articles);

            ArticleStruct description(tr("Description"));
            description.queryData("lshw", device, "description", existArticles2, articles);

            ArticleStruct physicalId(tr("Physical ID"));
            physicalId.queryData( "lshw", device, "physical id", existArticles2, articles);

            ArticleStruct version(tr("Version"));
            version.queryData( "lshw", device, "version", existArticles2, articles);


            ArticleStruct capacity(tr("Capacity"));
            capacity.queryData( "lshw", device, "capacity", existArticles2, articles);


            ArticleStruct slot(tr("Location"));
            slot.queryData( "dmidecode", device, "slot");
            articles.push_back(slot);
            if(slot.value.isEmpty() == false || slot.value != tr("Unknown"))
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

            ArticleStruct location(tr("Location"));
            location.queryData( "dmidecode", demideBattery, "Location", existArticles3, articles);

            ArticleStruct assetTag(tr("Asset Tag"));
            assetTag.queryData( "dmidecode", demideBattery, "Asset Tag", existArticles3, articles);

            de.queryData( "dmidecode", demideBattery, "Design Capacity", existArticles3, articles);
        }

        if(i < UpowerBatteryList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", device.value, articles, existArticles1,"ManulTrack__POWER","Power information");
        }

        if(i < lshwBatteryList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", lshwBatteryList[i], articles, existArticles2,"ManulTrack__POWER","Power information");
        }

        if(i < demidecodebatteryList.size())
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", demidecodebatteryList[i], articles, existArticles3,"ManulTrack__POWER","Power information");
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
                tr("Battery"),
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

        addDevice(tr("Battery") + " " + overName, articles, maxSwitchingSize + maxBatterySize, true );

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += tr("Battery") + " " + overName;
    }

    if( maxSwitchingSize + maxBatterySize > 1 )
    {
        QStringList headers = { tr("Name"), tr("Type"), tr("Vendor") };
        addTable( headers, tabList);
    }
}

bool PowerWidget::infoIsEmpty()
{
    // 查看lshw命令中能不能获取电源信息(交流电和电源)
    QStringList lshwSwitchingpowerList = DeviceInfoParser::Instance().getLshwSwitchingpowerList();
    QStringList lshwBatteryList = DeviceInfoParser::Instance().getLshwBatteryList();

    // 查看dmidecode命令能不能获取电源信息(交流电和电源)
    QStringList demidecodeSwitchingpowerList = DeviceInfoParser::Instance().getDemidecodeSwitchingpowerList();
    QStringList demidecodebatteryList = DeviceInfoParser::Instance().getDemidecodeBatteryList();

    // 查看upower命令能不能获取电源信息(交流电和电源)
    QStringList upowerSwitchingList = DeviceInfoParser::Instance().getUpowerSwitchingList();
    QStringList UpowerBatteryList = DeviceInfoParser::Instance().getUpowerBatteryList();

    int maxSwitchingSize = maxDeviceSize(lshwSwitchingpowerList, demidecodeSwitchingpowerList, upowerSwitchingList);
    int maxBatterySize = maxDeviceSize(lshwBatteryList, demidecodebatteryList, UpowerBatteryList);

    return (maxSwitchingSize + maxBatterySize < 1);
}

