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

#include "printerwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PrinterWidget::PrinterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Printer"))
{
    initWidget();
}

void PrinterWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Other Inputdevice")  + DApplication::translate("Main", " Info"));
    //QStringList printerList = DeviceInfoParser::Instance().getCupsPrinterList();

    QStringList lshwPrinterList = DeviceInfoParser::Instance().getLshwPrinterList();

    if( lshwPrinterList.size() < 1)
    {
        setCentralInfo("No Printer found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

//    foreach(const QString& device, printerList)
//    {
//        articles.clear();
//        existArticles.clear();

//        ArticleStruct printerInfo("printer-info");
//        printerInfo.queryData( "Cups", device, "printer-info");
//        articles.push_back(printerInfo);
//        existArticles.insert("printer-info");

//        ArticleStruct printerMakeAndModel("printer-make-and-model");
//        printerMakeAndModel.queryData( "Cups", device, "printer-make-and-model");
//        articles.push_back(printerMakeAndModel);
//        existArticles.insert("printer-make-and-model");

//        DeviceInfoParser::Instance().queryRemainderDeviceInfo( "Cups", device, articles, existArticles);

//        addDevice( device, articles, printerList.size() );

//        if( printerList.size() > 1 )
//        {
//            QStringList tab =
//            {
//                device,
//                printerMakeAndModel.value
//            };

//            tabList.push_back(tab);
//        }

//        if(overviewInfo_.value.isEmpty() )
//        {
//            overviewInfo_.value = printerMakeAndModel.value;
//            if( false == overviewInfo_.value.contains(printerInfo.value, Qt::CaseInsensitive) )
//            {
//                overviewInfo_.value += " ";
//                overviewInfo_.value += printerInfo.value;
//            }
//        }
//    }

    foreach(const QString& device, lshwPrinterList)
    {
        articles.clear();
        existArticles.clear();

        existArticles.insert("product");
        ArticleStruct name("Name");
        name.queryData("lshw", device, "product");
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

        ArticleStruct busInfo("Bus info");
        busInfo.queryData( "lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version("Version");
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", device, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", device, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        addDevice( device, articles, lshwPrinterList.size() );

        if( lshwPrinterList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() )
        {
            overviewInfo_.value = name.value;
            if( false == overviewInfo_.value.contains(vendor.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value = vendor.value + " " + name.value;
            }
        }
    }


    if( lshwPrinterList.size() > 1 )
    {
        QStringList headers = { "printer-info", "printer-make-and-model" };
        addTable( headers, tabList);
    }
}
