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

#include "cdromwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CDRomWidget::CDRomWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("CD-ROM"))
{
    initWidget();
}

void CDRomWidget::initWidget()
{
    QStringList cdromList = DeviceInfoParser::Instance().getLshwCDRomList();

    if( cdromList.size() < 1)
    {
        setCentralInfo(tr("No CD-ROM found"));
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, cdromList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct model(tr("Model"));
        model.queryData( "lshw", device, "product");
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData( "lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description(tr("Description"));
        description.queryData( "lshw", device, "description");
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

        ArticleStruct logicalName(tr("Logical Name"));
        logicalName.queryData( "lshw", device, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct Type(tr("Type"));
        description.queryData( "lshw", device, "Type");
        articles.push_back(Type);
        existArticles.insert("Type");

        ArticleStruct ansiversion(tr("ANSI Version"));
        busInfo.queryData( "lshw", device, "ansiversion");
        articles.push_back(ansiversion);
        existArticles.insert("ansiversion");

        ArticleStruct capabilities(tr("Capabilities"));
        version.queryData( "lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct physicalId(tr("Physical ID"));
        logicalName.queryData( "lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct status(tr("Status"));
        status.queryData("lshw",device,"status");
        articles.push_back(status);
        existArticles.insert("status");

//        DeviceInfoParser::Instance().queryRemainderDeviceInfo( "lshw", device, articles, existArticles,
//                                                               "ManulTrack__CDROM","CDROM infomation from lshw");

        addDevice( device, articles, cdromList.size() );

        if( cdromList.size() > 1 )
        {
            QStringList tab =
            {
                model.value,
                vendor.value,
                description.value
            };

            tabList.push_back(tab);
        }

        if(overviewInfo_.value.isEmpty() )
        {
            if( false == model.value.contains(vendor.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value = vendor.value;
                overviewInfo_.value += " ";
            }

            overviewInfo_.value += model.value;

            if( false == overviewInfo_.value.contains(description.value, Qt::CaseInsensitive) )
            {
                overviewInfo_.value += " ";
                overviewInfo_.value += description.value;
            }
        }
    }

    if( cdromList.size() > 1 )
    {
        QStringList headers = { "Model", "Vendor", "Description" };
        addTable( headers, tabList);
    }
}
