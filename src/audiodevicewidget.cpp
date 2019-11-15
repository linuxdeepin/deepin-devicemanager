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

#include "audiodevicewidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>

DWIDGET_USE_NAMESPACE

AudiodeviceWidget::AudiodeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Audio Adapter"))
{
    initWidget();
}

void AudiodeviceWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Audio Device")  + DApplication::translate("Main", " Info"));
    QStringList multimediaList = DeviceInfoParserInstance.getLshwMultimediaList();
    QStringList inputdeviceList = DeviceInfoParserInstance.getCatinputAudioDeviceList();

    if( multimediaList.size() + inputdeviceList.size() < 1 )
    {
        setCentralInfo("No Audio Device found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(auto multimedia, multimediaList)
    {
        articles.clear();
        existArticles.clear();

//        ArticleStruct type("Type");
//        type.value = "AudioAdapter";
//        articles.push_back(type);

        ArticleStruct busInfo("Bus info");
        busInfo.queryData("lshw", multimedia, "bus info", existArticles);
        QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");

        QString pci_bus;
        if(reg.exactMatch(busInfo.value))
        {
            pci_bus= reg.cap(1);
        }

        QString lspciDeviceName;
        DeviceInfoParserInstance.fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

        ArticleStruct name("Name");
        name.queryData("lspci", lspciDeviceName, "Name");
        name.queryData( "lshw", multimedia, "product", existArticles);

        name.value.remove( " Corporation", Qt::CaseInsensitive );
        int index = name.value.indexOf('(');
        if(index > 0)
        {
            name.value = name.value.mid(0, index);
        }

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", multimedia, "vendor", existArticles);

        ArticleStruct description("Description");
        description.queryData("lshw", multimedia, "description");

        existArticles.insert("description");

        if(name.value == vendor.value)
        {
            name.value = description.value;
        }

        articles.push_back(name);
        articles.push_back(vendor);
        articles.push_back(description);
        articles.push_back(busInfo);

        ArticleStruct version("Version");
        version.queryData( "lshw", multimedia, "version", existArticles, articles);


        ArticleStruct width("Width");
        width.queryData( "lshw", multimedia, "width", existArticles, articles);


        ArticleStruct clock("Clock");
        clock.queryData( "lshw", multimedia, "clock", existArticles, articles);


        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", multimedia, "capabilities", existArticles, articles);

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", multimedia, articles, existArticles);
        addDevice( name.value , articles, multimediaList.size() + inputdeviceList.size() );

        if( multimediaList.size() + inputdeviceList.size() > 1 )
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
        vendor.queryData( "catinput", device, "Vendor" , existArticles, articles);

        ArticleStruct vesion("Version");
        vesion.queryData( "catinput", device, "Version", existArticles, articles);

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs", existArticles, articles);


        DeviceInfoParserInstance.queryRemainderDeviceInfo("catinput", device, articles, existArticles);
        addSubInfo( name.value , articles );

        if( multimediaList.size() + inputdeviceList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if( multimediaList.size() + inputdeviceList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor"};
        addTable( headers, tabList);
    }
}
