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

#include "keyboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>
#include <linux/input.h>

DWIDGET_USE_NAMESPACE

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Keyboard"))
{
    initWidget();
}

void KeyboardWidget::initWidget()
{
    QStringList inputdeviceList = DeviceInfoParserInstance.getInputdeviceKeyboardList();

    if( inputdeviceList.size() < 1 )
    {
        setCentralInfo("No Keyboard found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    QSet<QString> existKeyboard;
    QSet<QString> existPhys;

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "catinput", device, "Name");
        name.value.remove("\"");
        articles.push_back(name);
        existArticles.insert("Name");

        ArticleStruct vendor("Vendor");
        ArticleStruct type("Type");

        QString lshwKeyboard = DeviceInfoParserInstance.getCorrespondLshwKeyboard(name.value);
        if( lshwKeyboard.isEmpty() == false )
        {
            existArticles.insert("product");

            ArticleStruct description("Description");
            description.queryData("lshw", lshwKeyboard, "description");
            articles.push_back(description);
            existArticles.insert("description");

            vendor.queryData( "lshw", lshwKeyboard, "vendor");
            articles.push_back(vendor);
            existArticles.insert("vendor");

            ArticleStruct busInfo("Bus info");
            busInfo.queryData( "lshw", lshwKeyboard, "bus info");
            articles.push_back(busInfo);
            existArticles.insert("bus info");

            ArticleStruct version("Version");
            version.queryData( "lshw", lshwKeyboard, "version");
            articles.push_back(version);
            existArticles.insert("version");

            ArticleStruct width("Width");
            width.queryData( "lshw", lshwKeyboard, "width");
            articles.push_back(width);
            existArticles.insert("width");

            ArticleStruct clock("Clock");
            clock.queryData( "lshw", lshwKeyboard, "clock");
            articles.push_back(clock);
            existArticles.insert("clock");

            ArticleStruct capabilities("Capabilities");
            capabilities.queryData( "lshw", lshwKeyboard, "capabilities");
            articles.push_back(capabilities);
            existArticles.insert("capabilities");

            DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", lshwKeyboard, articles, existArticles);
        }

        ArticleStruct uniq("Uniq");
        uniq.queryData( "catinput", device, "Uniq");

        if(uniq.isValid())
        {
            if(existKeyboard.contains(uniq.value))
            {
                continue;
            }

            existKeyboard.insert(uniq.value);
            QString blueTooth = DeviceInfoParserInstance.getCorrespondBluetoothKeyboard(uniq.value);

            if( blueTooth.isEmpty() == false )
            {
                type.value = "Bluetooth";
                existArticles.insert("Name");

                ArticleStruct connected("Connected");
                connected.queryData( "paired-devices", blueTooth, "Connected");
                //articles.push_back(connected);

                if(connected.value.compare("yes", Qt::CaseInsensitive) != 0)
                {
                    continue;
                }

                //existArticles.insert("Connected");

                DeviceInfoParserInstance.queryRemainderDeviceInfo("paired-devices", blueTooth, articles, existArticles);
            }
        }

        ArticleStruct phys("Phys");
        phys.queryData( "catinput", device, "Phys");

        if(phys.isValid())
        {
            if(existPhys.contains(phys.value))
            {
                continue;
            }

            existPhys.insert(phys.value);
        }

        if(type.isValid() == false)
        {
            if(phys.value.contains("usb", Qt::CaseInsensitive))
            {
                type.value = "Usb";
            }
            else if(phys.value.contains("/serio", Qt::CaseInsensitive))
            {
                type.value = "PS/2";
            }
            else
            {
                type.value = "Other";
            }
        }

        //articles.push_back(type);

        if(vendor.isValid() == false)
        {
            vendor.queryData( "catinput", device, "Vendor");
            articles.push_back(vendor);
        }
        existArticles.insert("Vendor");

        articles.push_back(uniq);
        existArticles.insert("Uniq");

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

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

        if( uniq.isValid() )
        {
            auto upower = DeviceInfoParserInstance.getCorrespondUpower(uniq.value);

            if(upower.isEmpty() == false )
            {
                ArticleStruct power("Power");
                power.value = " ";
                articles.push_back(power);

                DeviceInfoParserInstance.queryRemainderDeviceInfo("upower", upower, articles );
            }
        }

        addDevice( name.value , articles , inputdeviceList.size() );

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += name.value;

        if( inputdeviceList.size()  > 1 )
        {
            QStringList tab =
            {
                name.value,
                type.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if( inputdeviceList.size() > 1 )
    {
        QStringList headers = { "Name",  "Type", "Vendor" };
        addTable( headers, tabList);
    }
}

