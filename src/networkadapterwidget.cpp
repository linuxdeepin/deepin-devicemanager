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

#include "networkadapterwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

NetworkadapterWidget::NetworkadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "NetworkAdapter"))
{
    initWidget();
}

void NetworkadapterWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "NetworkAdapter")  + DApplication::translate("Main", " Info"));


    QStringList networkadapterList = DeviceInfoParserInstance.getNetworkadapterList();

    if( networkadapterList.size() < 1 )
    {
        setCentralInfo("No NetworkAdapter found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    foreach(const QString& networkadapter, networkadapterList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "lshw", networkadapter, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", networkadapter, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description("Description");
        description.queryData( "lshw", networkadapter, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct version("Version");
        version.queryData( "lshw", networkadapter, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct busInfo("Bus Info");
        busInfo.queryData( "lshw", networkadapter, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct logicalName("Logical Name");
        logicalName.queryData( "lshw", networkadapter, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct mac("Mac Address");
        mac.queryData( "lshw", networkadapter, "serial");
        articles.push_back(mac);
        existArticles.insert("serial");

        ArticleStruct speed("Speed");
        speed.queryData( "lshw", networkadapter, "speed");
        articles.push_back(speed);
        existArticles.insert("speed");

        ArticleStruct width("Width");
        width.queryData( "lshw", networkadapter, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", networkadapter, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", networkadapter, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", networkadapter, articles, existArticles);
        addDevice( name.value , articles, networkadapterList.size() );

        if( networkadapterList.size() > 1 )
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
            overviewInfo_.value += name.value;
        }
    }

    if( networkadapterList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}
