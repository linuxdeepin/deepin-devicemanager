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

#include "keyboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>
#include <linux/input.h>

DWIDGET_USE_NAMESPACE

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Keyboard"))
{
    initWidget();
}

void KeyboardWidget::initWidget()
{
    QStringList inputdeviceList = DeviceInfoParser::Instance().getInputdeviceKeyboardList();

    int validKeyboardSize = 0;

    if( inputdeviceList.size() < 1 )
    {
        if(false == findUsbKeyboard()){
            setCentralInfo(tr("No keyboard found"));
            return;
        }
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    QSet<QString> existKeyboard;
    QSet<QString> existPhys;

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        ArticleStruct vendor("Vendor");
        ArticleStruct type("Type");


        name.queryData( "catinput", device, "Name");
        existArticles.insert("Name");

        name.value.remove("\"");
        QString lshwKeyboard = DeviceInfoParser::Instance().getCorrespondLshwKeyboard(name.value);

        if( lshwKeyboard.isEmpty() == false )
        {
            vendor.queryData( "lshw", lshwKeyboard, "vendor");

            existArticles.insert("vendor");

            if(name.value.count(vendor.value) > 1)
            {
                name.value = vendor.value + " " + name.value.remove(vendor.value).trimmed();
            }
        }

        articles.push_back(name);
        articles.push_back(vendor);

        //QString lshwKeyboard = DeviceInfoParser::Instance().getCorrespondLshwKeyboard(name.value);
        if( lshwKeyboard.isEmpty() == false )
        {
            existArticles.insert("product");

            ArticleStruct description("Description");
            description.queryData("lshw", lshwKeyboard, "description");
            articles.push_back(description);
            existArticles.insert("description");

            ArticleStruct busInfo("Bus info");
            busInfo.queryData( "lshw", lshwKeyboard, "bus info");
            articles.push_back(busInfo);
            existArticles.insert("bus info");

            ArticleStruct version("Version");
            version.queryData( "lshw", lshwKeyboard, "version");
            articles.push_back(version);
            existArticles.insert("version");

            ArticleStruct width("Width");
            width.queryData( "lshw", lshwKeyboard, "width");
            articles.push_back(width);
            existArticles.insert("width");

            ArticleStruct clock("Clock");
            clock.queryData( "lshw", lshwKeyboard, "clock");
            articles.push_back(clock);
            existArticles.insert("clock");

            ArticleStruct capabilities("Capabilities");
            capabilities.queryData( "lshw", lshwKeyboard, "capabilities");
            articles.push_back(capabilities);
            existArticles.insert("capabilities");

            DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", lshwKeyboard, articles, existArticles);
        }

        ArticleStruct uniq("Uniq");
        uniq.queryData( "catinput", device, "Uniq");

        if(uniq.isValid())
        {
            if(existKeyboard.contains(uniq.value))
            {
                continue;
            }

            existKeyboard.insert(uniq.value);
            QString blueTooth = DeviceInfoParser::Instance().getCorrespondBluetoothKeyboard(uniq.value);

            if( blueTooth.isEmpty() == false )
            {
                type.value = "Bluetooth";
                existArticles.insert("Name");

                ArticleStruct connected("Connected");
                connected.queryData( "paired-devices", blueTooth, "Connected");
                //articles.push_back(connected);

                if(connected.isValid() && connected.value.compare("yes", Qt::CaseInsensitive) != 0)
                {
                    continue;
                }

                //existArticles.insert("Connected");

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("paired-devices", blueTooth, articles, existArticles);
            }
        }

        ArticleStruct phys("Phys");
        phys.queryData( "catinput", device, "Phys");

        if(phys.isValid())
        {
            if(existPhys.contains(phys.value))
            {
                continue;
            }

            existPhys.insert(phys.value);
        }

        if(type.isValid() == false)
        {
            if(phys.value.contains("usb", Qt::CaseInsensitive))
            {
                type.value = "Usb";
            }
            else if(phys.value.contains("/serio", Qt::CaseInsensitive))
            {
                type.value = "PS/2";
            }
            else
            {
                type.value = "Other";
            }
        }

        //articles.push_back(type);

        if(vendor.isValid() == false)
        {
            vendor.queryData( "catinput", device, "Vendor");
            articles.push_back(vendor);
        }
        existArticles.insert("Vendor");

        articles.push_back(uniq);
        existArticles.insert("Uniq");

        ArticleStruct sysfs("Sysfs");
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        articles.push_back(phys);
        existArticles.insert("Phys");

        ArticleStruct vesion("Version");
        vesion.queryData( "catinput", device, "Version");
        articles.push_back(vesion);
        existArticles.insert("Version");

        ArticleStruct product("Product");
        product.queryData( "catinput", device, "Product");
        articles.push_back(product);
        existArticles.insert("Product");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", device, articles, existArticles);

        if( uniq.isValid() )
        {
            auto upower = DeviceInfoParser::Instance().getCorrespondUpower(uniq.value);

            if(upower.isEmpty() == false )
            {
                ArticleStruct power("Power");
                power.value = " ";
                articles.push_back(power);

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", upower, articles );
            }
        }

        ++validKeyboardSize;
        addDevice( name.value , articles , inputdeviceList.size() );

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += name.value;

        if( inputdeviceList.size()  > 1 )
        {
            QStringList tab =
            {
                name.value,
                type.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if( validKeyboardSize > 1 )
    {
        QStringList headers = { "Name",  "Type", "Vendor" };
        addTable( headers, tabList);
    }
}
/*
    *@author yaobin
    *@date 2020-01-03
    *输入设备中没有找到键盘,尝试从usb设备中寻找
    */
bool KeyboardWidget::findUsbKeyboard()
{
    QStringList findKeyboards = DeviceInfoParser::Instance().getLshwUsbKeyboardDeviceList();
    if( findKeyboards.size() < 1)
    {
        return false;
    }
    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& keyboard, findKeyboards)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name("Name");
        name.queryData( "lshw", keyboard, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description("Description");
        description.queryData("lshw", keyboard, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor("Vendor");
        vendor.queryData( "lshw", keyboard, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo("Bus info");
        busInfo.queryData( "lshw", keyboard, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct physicalId("Physical ID");
        physicalId.queryData( "lshw", keyboard, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct logicalName("Logical Name");
        logicalName.queryData( "lshw", keyboard, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct version("Version");
        version.queryData( "lshw", keyboard, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width("Width");
        width.queryData( "lshw", keyboard, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock("Clock");
        clock.queryData( "lshw", keyboard, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData( "lshw", keyboard, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", keyboard, articles, existArticles);

        QString title = name.isValid()? name.value: description.value;
        addDevice( title, articles, findKeyboards.size() );

        QStringList tab =
        {
            title,
            vendor.value
        };

        tabList.push_back(tab);

        if(overviewInfo_.isValid())
        {
            overviewInfo_.value += " / ";
        }

        QList<ArticleStruct> overArticle;
        overArticle << vendor << name;
        if(name.isValid() == false)
        {
            overArticle << description;
        }
        overviewInfo_.value += joinArticle(overArticle);
    }
    if( findKeyboards.size() > 1 )
    {
        QStringList headers = { "Name",  "Vendor" };
        addTable( headers, tabList);
    }
    return  true;
}


