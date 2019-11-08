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

#include "bluetoothwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE


BluetoothWidget::BluetoothWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Bluetooth"))
{
    initWidget();
}

void BluetoothWidget::initWidget()
{
    QStringList bluetoothList = DeviceInfoParserInstance.getBluetoothList();

    if( bluetoothList.size() < 1 )
    {
        setCentralInfo("No Bluetooth Device found!");
        return;
    }

    QStringList hciconfigBluetoothList = DeviceInfoParserInstance.getHciconfigBluetoothList();
    //setTitle(DApplication::translate("Main", "Bluetooth") + " " + DApplication::translate("Main", " Info"));

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    QSet<QString> existArticles2;

    int i = 0;
    foreach(const QString& device, bluetoothList)
    {
        articles.clear();
        existArticles.clear();
        existArticles2.clear();

        ArticleStruct name("Name");
        ArticleStruct vendor("Vendor");
        ArticleStruct product("Product");
        ArticleStruct description("Description");

        if( i < hciconfigBluetoothList.size() )
        {
            QString hciconfigName = hciconfigBluetoothList.at(i);

            name.queryData("hciconfig", hciconfigName, "Name");
            name.value.remove("\'");
            articles.push_back(name);
            existArticles.insert("Name");

            vendor.queryData("hciconfig", hciconfigName, "Manufacturer");
            articles.push_back(vendor);
            existArticles.insert("Manufacturer");

            ArticleStruct mac("Mac Address");
            mac.queryData("hciconfig", hciconfigName, "BD Address");
            articles.push_back(mac);
            existArticles.insert("BD Address");

            if(vendor.value.isEmpty())
            {
                vendor.queryData("lshw", device, "vendor");
            }
            existArticles2.insert("vendor");

            product.queryData( "lshw", device, "product");
            articles.push_back(product);
            existArticles2.insert("product");

            description.queryData("lshw", device, "description");
            articles.push_back(description);
            existArticles2.insert("description");

            ArticleStruct serial("Serial");
            serial.queryData( "lshw", device, "serial");
            articles.push_back(serial);
            existArticles2.insert("serial");

            ArticleStruct linkPolicy("Link Policy");
            linkPolicy.queryData("hciconfig", hciconfigName, "Link policy");
            articles.push_back(linkPolicy);
            existArticles.insert("Link policy");

            ArticleStruct linkMode("Link Mode");
            linkMode.queryData("hciconfig", hciconfigName, "Link mode");
            articles.push_back(linkMode);
            existArticles.insert("Link mode");

            if(mac.isValid())
            {
                ArticleStruct powered("Powered");
                powered.queryData("bluetoothctl", mac.value, "Powered");
                articles.push_back(powered);
                existArticles.insert("Powered");

                ArticleStruct discoverable("Discoverable");
                discoverable.queryData("bluetoothctl", mac.value, "Discoverable");
                articles.push_back(discoverable);
                existArticles.insert("Discoverable");

                ArticleStruct pairable("Pairable");
                pairable.queryData("bluetoothctl", mac.value, "Pairable");
                articles.push_back(pairable);
                existArticles.insert("Pairable");

                ArticleStruct modalias("Modalias");
                modalias.queryData("bluetoothctl", mac.value, "Modalias");
                articles.push_back(modalias);
                existArticles.insert("Modalias");

                ArticleStruct discovering("Discovering");
                discovering.queryData("bluetoothctl", mac.value, "Discovering");
                articles.push_back(discovering);
                existArticles.insert("Discovering");
            }
        }

        //existArticles.clear();



        ArticleStruct physicalId("Physical id");
        physicalId.queryData("lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles2.insert("physical id");

        ArticleStruct busInfo("Bus Info");
        busInfo.queryData("lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles2.insert("bus info");

        ArticleStruct version("Version");
        version.queryData("lshw", device, "version");
        articles.push_back(version);
        existArticles2.insert("version");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData("lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles2.insert("capabilities");

        if( i < hciconfigBluetoothList.size() )
        {
            QString hciconfigName = hciconfigBluetoothList.at(i);

            DeviceInfoParserInstance.queryRemainderDeviceInfo("hciconfig", hciconfigName, articles, existArticles);
        }

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", device, articles, existArticles2);

        addDevice( name.value, articles,  bluetoothList.size() );

        if( bluetoothList.size() > 1 )
        {
            QStringList tab =
            {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if(i == 0)
        {
            overviewInfo_.value = vendor.value;
            overviewInfo_.value += " ";
            if( product.value.isEmpty() == false && product.value != DApplication::translate("Main", "Unknown") )
            {
                overviewInfo_.value += product.value;
            }
            else
            {
                overviewInfo_.value += description.value;
            }
        }

        ++i;
    }

    if( bluetoothList.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
}

