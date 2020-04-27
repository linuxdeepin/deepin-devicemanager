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

#include "mousewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MouseWidget::MouseWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Mouse"))
{
    initWidget();
}

bool MouseWidget::isSameMouse(QString &physStr)
{
    QStringList strList = physStr.split("/");
    if (strList.size() > 1) {
        int index = m_physList.indexOf(strList[0]);
        if (index != -1) {
            return true;
        }
        else {
            m_physList.append(strList[0]);
        }
    }

    return false;
}

void MouseWidget::initWidget()
{
    QStringList inputdeviceList = DeviceInfoParser::Instance().getInputdeviceMouseList();
    //QStringList mouseList = DeviceInfoParser::Instance().getLshwMouseList();

    if( inputdeviceList.size() < 1 )
    {
        setCentralInfo(tr("No mouse found"));
        return;
    }

    int validMouseSize = 0;

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& device, inputdeviceList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct type(tr("Type"));

        name.queryData( "catinput", device, "Name");
        name.value.remove("\"");
        existArticles.insert("Name");

        QString lshwMouse = DeviceInfoParser::Instance().getCorrespondLshwMouse(name.value);
        if( lshwMouse.isEmpty() == false )
        {
            vendor.queryData( "lshw", lshwMouse, "vendor");

            existArticles.insert("vendor");

            if(name.value.count(vendor.value) > 1)
            {
                name.value = vendor.value + " " + name.value.remove(vendor.value).trimmed();
            }
        }

        articles.push_back(name);

        if( lshwMouse.isEmpty() == false )
        {
            existArticles.insert("product");

            ArticleStruct description(tr("Description"));
            description.queryData("lshw", lshwMouse, "description");
            articles.push_back(description);
            existArticles.insert("description");

            ArticleStruct busInfo(tr("Bus Info"));
            busInfo.queryData( "lshw", lshwMouse, "bus info");
            articles.push_back(busInfo);
            existArticles.insert("bus info");

            ArticleStruct version(tr("Version"));
            version.queryData( "lshw", lshwMouse, "version");
            articles.push_back(version);
            existArticles.insert("version");

            ArticleStruct width(tr("Width"));
            width.queryData( "lshw", lshwMouse, "width");
            articles.push_back(width);
            existArticles.insert("width");

            ArticleStruct clock(tr("Clock"));
            clock.queryData( "lshw", lshwMouse, "clock");
            articles.push_back(clock);
            existArticles.insert("clock");

            ArticleStruct capabilities(tr("Capabilities"));
            capabilities.queryData( "lshw", lshwMouse, "capabilities");
            articles.push_back(capabilities);
            existArticles.insert("capabilities");

            ArticleStruct driver(tr("driver"));
            driver.queryData("lshw", lshwMouse, "driver");
            articles.push_back(driver);
            existArticles.insert("driver");

            ArticleStruct maxpower(tr("maxpower"));
            maxpower.queryData("lshw", lshwMouse, "maxpower");
            articles.push_back(maxpower);
            existArticles.insert("maxpower");

            ArticleStruct physicalId(tr("physical id"));
            physicalId.queryData("lshw", lshwMouse, "physical id");
            articles.push_back(physicalId);
            existArticles.insert("physical id");

            ArticleStruct speed(tr("speed"));
            speed.queryData("lshw", lshwMouse, "speed");
            articles.push_back(speed);
            existArticles.insert("speed");

            DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", lshwMouse, articles, existArticles,"ManulTrack__Mouse","Mouse information");
        }

        ArticleStruct uniq(tr("Uniq","Mouse Info from catinput"));
        uniq.queryData( "catinput", device, "Uniq");

        if(uniq.isValid())
        {
            QString blueTooth = DeviceInfoParser::Instance().getCorrespondBluetoothMouse(uniq.value);
            if( blueTooth.isEmpty() == false )
            {
                type.value = "Bluetooth";
                existArticles.insert("Name");

                ArticleStruct connected(tr("Connected"));
                connected.queryData( "paired-devices", blueTooth, "Connected");
                //articles.push_back(connected);

                if(connected.isValid() && connected.value.compare("yes", Qt::CaseInsensitive) != 0)
                {
                    continue;
                }

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("paired-devices", blueTooth, articles, existArticles,"ManulTrack__Mouse","Mouse information");
            }
        }

        ArticleStruct phys(tr("Phys","Mouse Info from catinput"));
        phys.queryData( "catinput", device, "Phys");

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
        }
        articles.push_back(vendor);
        existArticles.insert("Vendor");

        articles.push_back(uniq);
        existArticles.insert("Uniq");

        ArticleStruct sysfs(tr("Sysfs","Mouse Info from catinput"));
        sysfs.queryData( "catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        articles.push_back(phys);
        existArticles.insert("Phys");

        ArticleStruct vesion(tr("Version"));
        vesion.queryData( "catinput", device, "Version");
        articles.push_back(vesion);
        existArticles.insert("Version");

        ArticleStruct product(tr("Product"));
        product.queryData( "catinput", device, "Product");
        articles.push_back(product);
        existArticles.insert("Product");

        ArticleStruct bus(tr("Bus"));
        bus.queryData("catinput", device, "Bus");
        articles.push_back(bus);
        existArticles.insert("Bus");

        ArticleStruct ev(tr("EV"));
        ev.queryData("catinput", device, "EV");
        articles.push_back(ev);
        existArticles.insert("EV");

        ArticleStruct handlers(tr("Handlers"));
        handlers.queryData("catinput", device, "Handlers");
        articles.push_back(handlers);
        existArticles.insert("Handlers");

        ArticleStruct key(tr("KEY"));
        key.queryData("catinput", device, "KEY");
        articles.push_back(key);
        existArticles.insert("KEY");

        ArticleStruct msc(tr("MSC"));
        msc.queryData("catinput", device, "MSC");
        articles.push_back(msc);
        existArticles.insert("MSC");

        ArticleStruct prop(tr("PROP"));
        prop.queryData("catinput", device, "PROP");
        articles.push_back(prop);
        existArticles.insert("PROP");

        ArticleStruct rel(tr("REL"));
        rel.queryData("catinput", device, "REL");
        articles.push_back(rel);
        existArticles.insert("REL");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", device, articles, existArticles,"ManulTrack__Mouse","Mouse information");

        if( uniq.isValid() )
        {
            auto upower = DeviceInfoParser::Instance().getCorrespondUpower(uniq.value);

            if(upower.isEmpty() == false )
            {
                ArticleStruct power(tr("Power","Mouse Info from upower"));
                power.value = " ";
                articles.push_back(power);

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", upower, articles );
            }
        }

        if (isSameMouse(phys.value)) {
            continue;
        }
        ++validMouseSize;
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

    if( validMouseSize /*+ mouseList.size()*/ > 1 )
    {
        QStringList headers = { tr("Name"),tr("Type"),tr("Vendor")};
        addTable( headers, tabList);
    }
}
