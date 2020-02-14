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

#include "computeroverviewwidget.h"
#include "deviceinfoparser.h"
#include "QRegExp"
#include "math.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Overview"))
{
    
}

void ComputerOverviewWidget::setOverviewInfos( const QList<ArticleStruct>& others )
{
    //setTitle( "Computer Overview" );

    QList<ArticleStruct> articles;

    ArticleStruct model(tr("Device"));

    ArticleStruct vendor;
    vendor.queryData("dmidecode", "System Information", "Manufacturer");
    if(vendor.isValid() == false)
    {
        vendor.queryData("catbaseboard", "Base Board Information", "Manufacturer");
    }
    if(vendor.value.contains("System manufacturer"))
    {
        vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    }

    ArticleStruct pName;
    pName.queryData("dmidecode", "System Information", "Product Name");
    if(pName.isValid() == false)
    {
        pName.queryData("catbaseboard", "Base Board Information", "Board name");
    }
    if( pName.value.contains("System Product Name"))
    {
        pName.queryData("dmidecode", "Base Board Information", "Board name");
    }

    ArticleStruct family;
    family.queryData("dmidecode", "System Information", "Family");
    family.value = family.value.remove(vendor.value);
    if(family.isValid() == false || family.value.contains("System Version") \
        || family.value.contains("Not Specified", Qt::CaseInsensitive) || family.value.contains("x.x", Qt::CaseInsensitive) || family.value.contains("Not Applicable", Qt::CaseInsensitive))
    {
        family.value = "";
    }

    ArticleStruct ver;
    ver.queryData("dmidecode", "System Information", "Version");
    if(ver.isValid() == false || ver.value.contains("System Version") \
            || ver.value.contains("Not Specified", Qt::CaseInsensitive) || ver.value.contains("x.x", Qt::CaseInsensitive) || ver.value.contains("Not Applicable", Qt::CaseInsensitive))
    {
        ver.value = "";
    }

    ArticleStruct chassisType("Type");
    chassisType.transContextName = "Dmidecode";
    chassisType.queryData("dmidecode", "Chassis Information", "Type");

    pName.value = pName.value.remove( vendor.value, Qt::CaseInsensitive);
    ver.value = ver.value.remove(vendor.value, Qt::CaseInsensitive);
    ver.value = ver.value.remove(family.value, Qt::CaseInsensitive);
    family.value = family.value.remove(vendor.value, Qt::CaseInsensitive);
    family.value = family.value.remove(ver.value, Qt::CaseInsensitive);

    pName.value.remove(ver.value);

    QList<ArticleStruct> acList;

    acList << vendor << family << ver << model << pName << chassisType;

    model.value = joinArticle(acList);
//    else if(false == pName.contains(" ") && ver.contains(" "))  //	Product Name: 10N9CTO1WW  Version: ThinkCentre M910t-N000
//    {
//        model.value = vendor + " " + pName + " " + ver;
//    }

    articles.push_back(model);

    ArticleStruct os(tr("Operating System"));
    os.value = DeviceInfoParser::Instance().getLsbRelease() + " " + DeviceInfoParser::Instance().getOsInfo();

    if( DeviceInfoParser::Instance().getHomeUrl().isEmpty() == false)
    {
        os.externalLinks = true;
    }
    articles.push_back(os);

    articles.append(others);

    addInfo("", articles);
}
