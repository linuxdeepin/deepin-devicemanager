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
    //setTitle( DApplication::translate("Main", "DisplayAdapter")  + DApplication::translate("Main", " Info") );

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList displayadapterList = DeviceInfoParserInstance.getLshwDiaplayadapterList();
    QStringList screenList = DeviceInfoParserInstance.getXrandrScreenName();

    for(int i =0; i < displayadapterList.size(); ++i )
    {
        articles.clear();
        existArticles.clear();

        QString displayadapter = displayadapterList[i];
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

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", displayadapter, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct memory("Graphic Memory");
        memory.queryData("lspci", lspciDeviceName, "Memory");
        articles.push_back(memory);
        existArticles.insert("Memory");

        if( i < screenList.size() )
        {
            QString screenName = screenList[i];

            ArticleStruct minimum("Minimum Resolution");
            minimum.queryData( "xrandr", screenName, "minimum");
            articles.push_back(minimum);

            ArticleStruct maximum("Maximum Resolution");
            maximum.queryData( "xrandr", screenName, "maximum");
            articles.push_back(maximum);
        }

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

        QString dpName = "";
        if( displayadapterList.size() > 1 )
        {
            dpName = name.value;
        }

        addDevice( dpName, articles,  displayadapterList.size() );

        QStringList tab =
        {
            name.value,
            vendor.value
        };

        tabList.push_back(tab);

        if( overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }

        overviewInfo_.value += name.value.remove(vendor.value);

        overviewInfo_.value += " (";
        overviewInfo_.value += memory.value;

        if(DApplication::translate("Main", "Unknown") != vendor.value)
        {
            overviewInfo_.value += " / ";
            overviewInfo_.value += vendor.value;
        }

        overviewInfo_.value += ")";
    }

    if( displayadapterList.size() > 1)
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

