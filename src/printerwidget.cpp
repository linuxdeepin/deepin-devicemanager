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

PrinterWidget::PrinterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Printer"))
{
    initWidget();
}

void PrinterWidget::loadWidget()
{
    QStringList hwinfoPrinterList = DeviceInfoParser::Instance().getHwinfoPrinterList();

    if ( hwinfoPrinterList.size() < 1) {
        setCentralInfo(tr("No printer found"));
        return;
    }

    QList<QStringList> tabList;
//    QList<ArticleStruct> articles;
//    QSet<QString> existArticles;

    foreach (const QString &device, hwinfoPrinterList) {
        m_articles.clear();
        m_existArticles.clear();

        // 添加printer相关信息
        ArticleStruct name = addArticleStruct(tr("Name"), "printer", device, "Device");
        ArticleStruct vendor = addArticleStruct(tr("Vendor"), "printer", device, "Vendor");
        addArticleStruct(tr("Model"), "printer", device, "Model");
        addArticleStruct(tr("Serial ID"), "printer", device, "Serial ID");
        addArticleStruct(tr("Status"), "printer", device, "Config Status");
        addArticleStruct(tr("Driver"), "printer", device, "Driver");
        addArticleStruct(tr("Speed"), "printer", device, "Speed");
        addArticleStruct(tr("BusID"), "printer", device, "SysFS BusID");
        ArticleStruct description = addArticleStruct(tr("Description"), "printer", device, "description");

//        DeviceInfoParser::Instance().queryRemainderDeviceInfo("printer", device, articles, existArticles);

        addDevice( device, m_articles, hwinfoPrinterList.size() );

        if ( hwinfoPrinterList.size() > 1 ) {
            QStringList tab = {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if (overviewInfo_.value.isEmpty() ) {
            overviewInfo_.value = name.value;
            if ( false == overviewInfo_.value.contains(vendor.value, Qt::CaseInsensitive) ) {
                overviewInfo_.value = vendor.value + " " + name.value;
            }
        }
    }

    if ( hwinfoPrinterList.size() > 1 ) {
        QStringList headers = { tr("printer-info", "Printer Info,shown on first column of table's head"),
                                tr("printer-make-and-model", "Printer Info,shown on second column of table's head ")
                              };
        addTable( headers, tabList);
    }
}

void PrinterWidget::initWidget()
{
    //setTitle(tr("Other Inputdevice")  + tr(" Info"));
    //QStringList printerList = DeviceInfoParser::Instance().getCupsPrinterList();
    QStringList lshwPrinterList = DeviceInfoParser::Instance().getLshwPrinterList();

    if ( lshwPrinterList.size() < 1) {
        QStringList hwinfoPrinterList = DeviceInfoParser::Instance().getHwinfoPrinterList();
        if (hwinfoPrinterList.size() < 1) {
            setCentralInfo(tr("No printer found"));
            return;
        } else {
            loadWidget();
        }

    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach (const QString &device, lshwPrinterList) {
        articles.clear();
        existArticles.clear();

        existArticles.insert("product");
        ArticleStruct name(tr("Name"));
        name.queryData("lshw", device, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description(tr("Description"));
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct busInfo(tr("Bus Info"));
        busInfo.queryData( "lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct version(tr("Version"));
        version.queryData( "lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width(tr("Width"));
        width.queryData( "lshw", device, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock(tr("Clock"));
        clock.queryData( "lshw", device, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities(tr("Capabilities"));
        capabilities.queryData( "lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        addDevice( device, articles, lshwPrinterList.size() );

        if ( lshwPrinterList.size() > 1 ) {
            QStringList tab = {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if (overviewInfo_.value.isEmpty() ) {
            overviewInfo_.value = name.value;
            if ( false == overviewInfo_.value.contains(vendor.value, Qt::CaseInsensitive) ) {
                overviewInfo_.value = vendor.value + " " + name.value;
            }
        }
    }

    if ( lshwPrinterList.size() > 1 ) {
        QStringList headers = { tr("printer-info", "Printer Info,shown on first column of table's head"),
                                tr("printer-make-and-model", "Printer Info,shown on second column of table's head ")
                              };
        addTable( headers, tabList);
    }
}

