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

DWIDGET_USE_NAMESPACE

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Keyboard"))
{
    initWidget();
}

void KeyboardWidget::initWidget()
{
    QStringList inputdeviceList = DeviceInfoParserInstance.getPS_2KeyboardInputdeviceList();
    QStringList keyboardList = DeviceInfoParserInstance.getLshwKeyboardList();

    if( inputdeviceList.size() + keyboardList.size() < 1 )
    {
        setCentralInfo("No Keyboard found!");
        return;
    }

//    if( inputdeviceList.size() + keyboardList.size() == 1)
//    {
//        setTitle(DApplication::translate("Main", "Keyboard")  + DApplication::translate("Main", " Info"));
//    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "catinput", device, "Name");
        name.value.remove("\"");
        articles.push_back(name);
        existArticles.insert("Name");

        ArticleStruct type("Type");
        type.value = "PS/2";
        articles.push_back(type);

        ArticleStruct vendor("Vendor");
        vendor.queryData( "catinput", device, "Vendor");
        articles.push_back(vendor);
        existArticles.insert("Vendor");

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        ArticleStruct phys("Phys");
        phys.queryData( "catinput", device, "Phys");
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

        addDevice( name.value, articles, inputdeviceList.size() + keyboardList.size() );

//        overviewInfo_.value = "PS/2 ";
//        overviewInfo_.value += vendor.value;
//        overviewInfo_.value += " ";
        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }

        overviewInfo_.value += name.value;

        if( inputdeviceList.size() + keyboardList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                "PS/2",
                vendor.value
            };

            tabList.push_back(tab);
        }
    }


    foreach(const QString& keyboard, keyboardList)
    {
        articles.clear();
        existArticles.clear();

//        ArticleStruct type("Type");
//        type.value = "AudioAdapter";
//        articles.push_back(type);

        ArticleStruct name("Name");
        name.queryData( "lshw", keyboard, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description("Description");
        description.queryData("lshw", keyboard, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", keyboard, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo("Bus info");
        busInfo.queryData( "lshw", keyboard, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData( "lshw", keyboard, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", keyboard, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", keyboard, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", keyboard, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", keyboard, articles, existArticles);
        addDevice( name.value , articles, inputdeviceList.size() + keyboardList.size() );

        QStringList tab =
        {
            name.value,
            description.value,
            vendor.value
        };

        tabList.push_back(tab);

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }

        overviewInfo_.value += name.value;
    }

    if( inputdeviceList.size() + keyboardList.size() > 1 )
    {
        QStringList headers = { "Name",  "Type", "Vendor" };
        addTable( headers, tabList);
    }
}

