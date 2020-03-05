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
    m_articles.clear();
    m_tabList.clear();
    m_heads.clear();
    m_heads << tr("Name") << tr("Vendor");
    initWidget();
}

void KeyboardWidget::initWidget()
{
    bool ok1 = findKeyboardFromLshw();
    bool ok2 = findKeyboardFromCatInput();
    bool found = (ok1 || ok2);
    int keyboard_count = m_articles.count();
    if (found && keyboard_count > 0) {
        for (auto index = m_articles.begin(); index != m_articles.end(); index++) {
            addDevice(index.key(),index.value(),keyboard_count);
        }
        if (keyboard_count > 1) {
            addTable(m_heads,m_tabList);
        }
        return;
    }
    return foundNokeyboards();
}

/**
  *@author   yaobin
  *@date     2020-02-20
  *@brief    find keyboard from lshw cmd
  */
bool KeyboardWidget::findKeyboardFromLshw()
{
    QStringList foundKeyboards = DeviceInfoParser::Instance().getLshwUsbKeyboardDeviceList();
    if (foundKeyboards.isEmpty())
    {
        return false;
    }
    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& keyboard, foundKeyboards)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        name.queryData( "lshw", keyboard, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description(tr("Description"));
        description.queryData("lshw", keyboard, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData( "lshw", keyboard, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo(tr("Bus Info"));
        busInfo.queryData( "lshw", keyboard, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct physicalId(tr("Physical ID"));
        physicalId.queryData( "lshw", keyboard, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct logicalName(tr("Logical Name"));
        logicalName.queryData( "lshw", keyboard, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct version(tr("Version"));
        version.queryData( "lshw", keyboard, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width(tr("Width"));
        width.queryData( "lshw", keyboard, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock(tr("Clock"));
        clock.queryData( "lshw", keyboard, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities(tr("Capabilities"));
        capabilities.queryData( "lshw", keyboard, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", keyboard, articles, existArticles);

        QString title = name.isValid()? name.value: description.value;
        m_articles.insert(title,articles);

        QStringList tab =
        {
            title,
            vendor.value
        };

        m_tabList.push_back(tab);

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
    return  true;
}


/**
  *@author   yaobin
  *@date     2020-03-04
  *@brief    find keyboard from cmd: cat /proc/bus/input/devices, how ot exclude repeatable Ps/2 keyboards is still remained to do
  */
bool KeyboardWidget::findKeyboardFromCatInput()
{
    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    QSet<QString> existKeyboard;
    QSet<QString> existPhys;

    QStringList inputdeviceList = DeviceInfoParser::Instance().getInputdeviceKeyboardList();
    if (inputdeviceList.isEmpty()) {
        return false;
    }
    foreach(const QString& device, inputdeviceList)
    {
        if (device.contains("usb",Qt::CaseInsensitive)) {
            continue;
        }
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct type(tr("Type"));

        name.queryData( "catinput", device, "Name");
        existArticles.insert("Name");

        name.value.remove("\"");

        articles.push_back(name);
        articles.push_back(vendor);

        ArticleStruct uniq(tr("Uniq","Keyboard Info"));
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

                ArticleStruct connected(tr("Connected","Keyboard Info"));
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

        ArticleStruct phys(tr("Phys","Keyboard Info"));
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
//                type.value = "Usb";
                continue;
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

        ArticleStruct sysfs(tr("Sysfs","Keyboard Info,better if dont translate，my personal opinion"));
        sysfs.queryData("catinput", device, "Sysfs");
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

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", device, articles, existArticles);

        if( uniq.isValid() )
        {
            auto upower = DeviceInfoParser::Instance().getCorrespondUpower(uniq.value);

            if(upower.isEmpty() == false )
            {
                ArticleStruct power(tr("Power"));
                power.value = " ";
                articles.push_back(power);

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", upower, articles );
            }
        }
        m_articles.insert(name.value,articles);

        if(overviewInfo_.value.isEmpty() == false)
        {
            overviewInfo_.value += " / ";
        }
        overviewInfo_.value += name.value;

        m_tabList.push_back({name.value, vendor.value });
    }

    return true;
}

/**
  *@author   yaobin
  *@date     2020-02-20
  *@brief
  */
void KeyboardWidget::foundNokeyboards()
{
    setCentralInfo(tr("No keyboard found"));
    return;
}


