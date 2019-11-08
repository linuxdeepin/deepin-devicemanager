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
    QStringList printerList = DeviceInfoParserInstance.getPrinterList();

    if( printerList.size() < 1)
    {
        setCentralInfo("No Printer found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, printerList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct printerInfo("printer-info");
        printerInfo.queryData( "Cups", device, "printer-info");
        articles.push_back(printerInfo);
        existArticles.insert("printer-info");

        ArticleStruct printerMakeAndModel("printer-make-and-model");
        printerMakeAndModel.queryData( "Cups", device, "printer-make-and-model");
        articles.push_back(printerMakeAndModel);
        existArticles.insert("printer-make-and-model");

        DeviceInfoParserInstance.queryRemainderDeviceInfo( "Cups", device, articles, existArticles);

        addDevice( device, articles, printerList.size() );

        if( printerList.size() > 1 )
        {
            QStringList tab =
            {
                printerInfo.value,
                printerMakeAndModel.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() )
        {
            overviewInfo_.value = printerMakeAndModel.value;
            if( false == overviewInfo_.value.contains(printerInfo.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value += " ";
                overviewInfo_.value += printerInfo.value;
            }
        }
    }

    if( printerList.size() > 1 )
    {
        QStringList headers = { "printer-info", "printer-make-and-model" };
        addTable( headers, tabList);
    }
}
