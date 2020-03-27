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

#include "usbdevicewidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

UsbdeviceWidget::UsbdeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("USB Devices"))
{
    initWidget();
}

void UsbdeviceWidget::loadWidget()
{
    QStringList usbdeviceList = DeviceInfoParser::Instance().getHwinfoOtherUSBList();

    if (usbdeviceList.size() < 1) {
        setCentralInfo(tr("No USB devices found"));
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int usbCount = usbdeviceList.size();

    foreach (const QString &device, usbdeviceList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        name.queryData("USB", device, "Device");
        articles.push_back(name);
        existArticles.insert("Name");

        if (name.value.contains("hub", Qt::CaseInsensitive) ||
                name.value.contains("Keyboard", Qt::CaseInsensitive) ||
                name.value.contains("mouse", Qt::CaseInsensitive) ||
                name.value.contains("Camera", Qt::CaseInsensitive)) {
            --usbCount;
            continue;
        }

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData( "USB", device, "Vendor");
        articles.push_back(vendor);
        existArticles.insert("Vendor");

        ArticleStruct model(tr("Model"));
        model.queryData( "USB", device, "Model");
        articles.push_back(model);
        existArticles.insert("Model");

        ArticleStruct serial(tr("Serial ID"));
        serial.queryData( "USB", device, "Serial ID");
        articles.push_back(serial);
        existArticles.insert("Serial ID");

        ArticleStruct version(tr("Version"));
        version.queryData( "USB", device, "Revision");
        articles.push_back(version);
        existArticles.insert("Version");

        ArticleStruct status(tr("Status"));
        status.queryData( "USB", device, "Config Status");
        articles.push_back(status);
        existArticles.insert("Status");

        ArticleStruct driver(tr("Driver"));
        driver.queryData( "USB", device, "Driver");
        articles.push_back(driver);
        existArticles.insert("Driver");

        ArticleStruct speed(tr("Speed"));
        speed.queryData( "USB", device, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct bus(tr("BusID"));
        bus.queryData( "USB", device, "SysFS BusID");
        articles.push_back(bus);
        existArticles.insert("BusID");

        ArticleStruct description(tr("Description"));
        description.queryData("USB", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct uniqueID(tr("Unique ID"));
        uniqueID.queryData("USB", device, "Unique ID");
        articles.push_back(uniqueID);
        existArticles.insert("Unique ID");

        ArticleStruct sysFSID(tr("SysFS ID"));
        sysFSID.queryData("USB", device, "SysFS ID");
        articles.push_back(sysFSID);
        existArticles.insert("SysFS ID");

        //DeviceInfoParser::Instance().queryRemainderDeviceInfo("USB", device, articles, existArticles);

        QString title = name.isValid() ? name.value : description.value;
        addDevice(title, articles, usbdeviceList.size());

        QStringList tab = {
            title,
            vendor.value
        };

        tabList.push_back(tab);

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

    if (usbCount > 1) {
        QStringList headers = { tr("Name"), tr("Vendor") };
        addTable(headers, tabList);
    }

    if (usbCount < 1) {
        setCentralInfo(tr("No USB devices found"));
        return;
    }
}

void UsbdeviceWidget::initWidget()
{
    QStringList usbdeviceList = DeviceInfoParser::Instance().getLshwOtherUsbdeviceList();

    if (usbdeviceList.size() < 1) {
        QStringList hwinfoUSBList = DeviceInfoParser::Instance().getHwinfoOtherUSBList();
        if (hwinfoUSBList.size() < 1) {
            setCentralInfo(tr("No USB devices found"));
            return;
        } else {
            loadWidget();
        }
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach (const QString &device, usbdeviceList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        name.queryData("lshw", device, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct description(tr("Description"));
        description.queryData("lshw", device, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("lshw", device, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct busInfo(tr("Bus Info"));
        busInfo.queryData("lshw", device, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct physicalId(tr("Physical ID"));
        physicalId.queryData("lshw", device, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct logicalName(tr("Logical Name"));
        logicalName.queryData("lshw", device, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct version(tr("Version"));
        version.queryData("lshw", device, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width(tr("Width"));
        width.queryData("lshw", device, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock(tr("Clock"));
        clock.queryData("lshw", device, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities(tr("Capabilities"));
        capabilities.queryData("lshw", device, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", device, articles, existArticles);

        QString title = name.isValid() ? name.value : description.value;
        addDevice(title, articles, usbdeviceList.size());

        QStringList tab = {
            title,
            vendor.value
        };

        tabList.push_back(tab);

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

    if (usbdeviceList.size() > 1) {
        QStringList headers = { tr("Name"), tr("Vendor") };
        addTable(headers, tabList);
    }
}

