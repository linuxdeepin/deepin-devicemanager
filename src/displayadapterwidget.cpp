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

#include "displayadapterwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DisplayadapterWidget::DisplayadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Display Adapter"))
{
    initWidget();
}

void DisplayadapterWidget::initWidget()
{
    //setTitle( DApplication::translate("Main", "DisplayAdapter")  + DApplication::translate("Main", " Info") );

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList displayadapterList = DeviceInfoParser::Instance().getLshwDiaplayadapterList();
    QStringList screenList = DeviceInfoParser::Instance().getXrandrScreenName();

    for(int i =0; i < displayadapterList.size(); ++i )
    {
        articles.clear();
        existArticles.clear();

        QString displayadapter = displayadapterList[i];
        QString pci_bus = DeviceInfoParser::Instance().queryData("lshw", displayadapter, "bus info");
        QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");
        if(reg.exactMatch(pci_bus))
        {
            pci_bus= reg.cap(1);
        }

        QString lspciDeviceName;
        DeviceInfoParser::Instance().fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

        QString lspciName = DeviceInfoParser::Instance().fuzzyQueryData("lspci", lspciDeviceName, "bus info");
        ArticleStruct name("Name");
        name.queryData("lspci", lspciDeviceName, "Name");
        name.value.remove( " Corporation", Qt::CaseInsensitive );

        int index = name.value.indexOf('(');
        if(index > 0)
        {
            name.value = name.value.mid(0, index).trimmed();
        }
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", displayadapter, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description("Description");
        description.queryData("lshw", displayadapter, "description", existArticles, articles);

//        ArticleStruct memory("Graphic Memory");
//        memory.queryData("lspci", lspciDeviceName, "Memory");
//        articles.push_back(memory);
//        existArticles.insert("Memory");

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
//        interface.value = DeviceInfoParser::Instance().getDisplayInterfaceList().join(", ");
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

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", displayadapter, articles, existArticles);

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

        overviewInfo_.value += name.value.remove(vendor.value, Qt::CaseInsensitive).trimmed();

        QList<ArticleStruct> subArticles;
        subArticles << memory << vendor;
        QString subValue = joinArticle(subArticles, " / ");

        if( subValue.isEmpty() == false)
        {
            overviewInfo_.value += " (";
            overviewInfo_.value += subValue;
            overviewInfo_.value += ")";
        }
    }

    if( displayadapterList.size() > 1)
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

