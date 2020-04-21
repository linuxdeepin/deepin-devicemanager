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
    m_articlesmap.clear();
    m_tabList.clear();
    m_heads.clear();
    m_heads << tr("Name") << tr("Vendor");
    loadWidget();
}

void KeyboardWidget::initWidget()
{
    bool ok1 = findKeyboardFromLshw();
    bool ok2 = true;
    if (!ok1) {
        ok2 = findKeyboardFromCatInput();
    }
    bool found = (ok1 || ok2);
    int keyboard_count = m_articlesmap.count();
    if (found && keyboard_count > 0) {
        for (auto index = m_articlesmap.begin(); index != m_articlesmap.end(); index++) {
            addDevice(index.key(), index.value(), keyboard_count);
        }
        if (keyboard_count > 1) {
            addTable(m_heads, m_tabList);
        }
        return;
    }
    return foundNokeyboards();
}

void KeyboardWidget::loadWidget()
{
    bool found = findKeyboardFromHwinfo();
    int keyboard_count = m_articlesmap.count();

    if (found && keyboard_count >= 1) {
        for (auto index = m_articlesmap.begin(); index != m_articlesmap.end(); index++) {
            addDevice(index.key(), index.value(), keyboard_count);
        }
        if (keyboard_count > 1) {
            addTable(m_heads, m_tabList);
        }
        return;
    }

    if (keyboard_count < 1) {
        m_articlesmap.clear();
        initWidget();
        return;
    }
    return foundNokeyboards();
}

bool KeyboardWidget::findKeyboardFromHwinfo()
{
    QStringList keyboardList = DeviceInfoParser::Instance().getHwinfoKeyboardList();

    if (keyboardList.empty()) {
        return false;
    }

    foreach (const QString &keyboard, keyboardList) {
        m_articles.clear();
        m_existArticles.clear();

        // 添加Keyboard相关信息
        ArticleStruct name = addArticleStruct(tr("Name"), "Keyboard", keyboard, "Device");
        ArticleStruct vendor = addArticleStruct(tr("Vendor"), "Keyboard", keyboard, "Vendor");
        addArticleStruct(tr("Model"), "Keyboard", keyboard, "Model");
        addArticleStruct(tr("Serial ID"), "Keyboard", keyboard, "Serial ID");
        addArticleStruct(tr("Version"), "Keyboard", keyboard, "Revision");
        addArticleStruct(tr("Status"), "Keyboard", keyboard, "Config Status");
        addArticleStruct(tr("Driver"), "Keyboard", keyboard, "Driver");
        addArticleStruct(tr("Speed"), "Keyboard", keyboard, "Speed");
        addArticleStruct(tr("BusID"), "Keyboard", keyboard, "SysFS BusID");
        ArticleStruct description = addArticleStruct(tr("Description"), "Keyboard", keyboard, "description");
        addArticleStruct(tr("Unique ID"), "Keyboard", keyboard, "Unique ID");
        addArticleStruct(tr("SysFS ID"), "Keyboard", keyboard, "SysFS ID");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", keyboard, m_articles, m_existArticles);

        QString title = name.isValid() ? name.value : description.value;
        m_articlesmap.insert(title, m_articles);

        QStringList tab = {
            title,
            vendor.value
        };

        m_tabList.push_back(tab);

        if (overviewInfo_.isValid()) {
            overviewInfo_.value += " / ";
        }

        QList<ArticleStruct> overArticle;
        overArticle << vendor << name;
        if (name.isValid() == false) {
            overArticle << description;
        }
        // 此句会重复添加概况的键盘信息。
        overviewInfo_.value += joinArticle(overArticle);
    }

    return true;
}

/**
  *@author   yaobin
  *@date     2020-02-20
  *@brief    find keyboard from lshw cmd
  */
bool KeyboardWidget::findKeyboardFromLshw()
{
    QStringList foundKeyboards = DeviceInfoParser::Instance().getLshwUsbKeyboardDeviceList();
    if (foundKeyboards.isEmpty()) {
        return false;
    }
//    QList<QStringList> tabList;
//    QList<ArticleStruct> articles;
//    QSet<QString> existArticles;

    foreach (const QString &keyboard, foundKeyboards) {
        m_articles.clear();
        m_existArticles.clear();

        ArticleStruct name = addArticleStruct(tr("Name"), "lshw", keyboard, "product");
        ArticleStruct description = addArticleStruct(tr("Description"), "lshw", keyboard, "description");
        ArticleStruct vendor = addArticleStruct(tr("Vendor"), "lshw", keyboard, "vendor");
        addArticleStruct(tr("Bus Info"), "lshw", keyboard, "bus info");
        addArticleStruct(tr("Physical ID"), "lshw", keyboard, "physical id");
        addArticleStruct(tr("Logical Name"), "lshw", keyboard, "logical name");
        addArticleStruct(tr("Version"), "lshw", keyboard, "version");
        addArticleStruct(tr("Width"), "lshw", keyboard, "width");
        addArticleStruct(tr("Clock"), "lshw", keyboard, "clock");
        addArticleStruct(tr("Capabilities"), "lshw", keyboard, "capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", keyboard, m_articles, m_existArticles);

        QString title = name.isValid() ? name.value : description.value;
        m_articlesmap.insert(title, m_articles);

        QStringList tab = {
            title,
            vendor.value
        };

        m_tabList.push_back(tab);

        if (overviewInfo_.isValid()) {
            overviewInfo_.value += " / ";
        }

        QList<ArticleStruct> overArticle;
        overArticle << vendor << name;
        if (name.isValid() == false) {
            overArticle << description;
        }
        overviewInfo_.value += joinArticle(overArticle);
    }
    return  true;
}


/**
  *@author   yaobin
  *@date     2020-03-04
  *@brief    find keyboard from cmd: cat /proc/bus/input/devices,
  * how ot exclude repeatable Ps/2 keyboards is still remained to do
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
    foreach (const QString &device, inputdeviceList) {
        //屏蔽了名称中含有USB的keyboard
//        if (device.contains("usb",Qt::CaseInsensitive)) {
//            continue;
//        }
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct type(tr("Type"));

        name.queryData("catinput", device, "Name");
        existArticles.insert("Name");

        name.value.remove("\"");

        articles.push_back(name);
        articles.push_back(vendor);

        ArticleStruct uniq(tr("Uniq", "Keyboard Info"));
        uniq.queryData("catinput", device, "Uniq");

        if (uniq.isValid()) {
            if (existKeyboard.contains(uniq.value)) {
                continue;
            }

            existKeyboard.insert(uniq.value);
            QString blueTooth = DeviceInfoParser::Instance().getCorrespondBluetoothKeyboard(uniq.value);

            if (blueTooth.isEmpty() == false) {
                type.value = "Bluetooth";
                existArticles.insert("Name");

                ArticleStruct connected(tr("Connected", "Keyboard Info"));
                connected.queryData("paired-devices", blueTooth, "Connected");
                //articles.push_back(connected);

                if (connected.isValid() && connected.value.compare("yes", Qt::CaseInsensitive) != 0) {
                    continue;
                }

                //existArticles.insert("Connected");

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("paired-devices", blueTooth, articles, existArticles);
            }
        }

        ArticleStruct phys(tr("Phys", "Keyboard Info"));
        phys.queryData("catinput", device, "Phys");

        if (phys.isValid()) {
            if (existPhys.contains(phys.value)) {
                continue;
            }

            existPhys.insert(phys.value);
        }

        if (type.isValid() == false) {
            if (phys.value.contains("usb", Qt::CaseInsensitive)) {
                type.value = "Usb";
                //continue 的屏蔽了 usb借口的keyboard
//                continue;
            } else if (phys.value.contains("/serio", Qt::CaseInsensitive)) {
                type.value = "PS/2";
            } else {
                type.value = "Other";
            }
        }

        //articles.push_back(type);

        if (vendor.isValid() == false) {
            vendor.queryData("catinput", device, "Vendor");
            articles.push_back(vendor);
        }
        existArticles.insert("Vendor");

        articles.push_back(uniq);
        existArticles.insert("Uniq");

        ArticleStruct sysfs(tr("Sysfs", "Keyboard Info,better if dont translate，my personal opinion"));
        sysfs.queryData("catinput", device, "Sysfs");
        articles.push_back(sysfs);
        existArticles.insert("Sysfs");

        articles.push_back(phys);
        existArticles.insert("Phys");

        ArticleStruct vesion(tr("Version"));
        vesion.queryData("catinput", device, "Version");
        articles.push_back(vesion);
        existArticles.insert("Version");

        ArticleStruct product(tr("Product"));
        product.queryData("catinput", device, "Product");
        articles.push_back(product);
        existArticles.insert("Product");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", device, articles, existArticles);

        if (uniq.isValid()) {
            auto upower = DeviceInfoParser::Instance().getCorrespondUpower(uniq.value);

            if (upower.isEmpty() == false) {
                ArticleStruct power(tr("Power"));
                power.value = " ";
                articles.push_back(power);

                DeviceInfoParser::Instance().queryRemainderDeviceInfo("upower", upower, articles);
            }
        }
        m_articlesmap.insert(name.value, articles);

        if (overviewInfo_.value.isEmpty() == false) {
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


