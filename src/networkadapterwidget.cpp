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

#include "networkadapterwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

NetworkadapterWidget::NetworkadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Network Adapter"))
{
    initWidget();
}

void NetworkadapterWidget::loadWidget()
{
    QStringList networkadapterList = DeviceInfoParser::Instance().getLshwNetworkadapterList();

    if (networkadapterList.size() < 1) {
        setCentralInfo(tr("No network adapter found"));
        return;
    }

    QList<QStringList> tabList;
//    QList<ArticleStruct> articles;
//    QSet<QString> existArticles;
    foreach (const QString &networkadapter, networkadapterList) {
        m_articles.clear();
        m_existArticles.clear();

        // 添加网络适配器的属性
        ArticleStruct name = addArticleStruct(tr("Name"), "lshw", networkadapter, "product");
        ArticleStruct vendor = addArticleStruct(tr("Vendor"), "lshw", networkadapter, "vendor");
        ArticleStruct description = addArticleStruct(tr("Description"), "lshw", networkadapter, "description");
        addArticleStruct(tr("Version"), "lshw", networkadapter, "version");
        addArticleStruct(tr("Bus Info"), "lshw", networkadapter, "bus info");
        addArticleStruct(tr("Logical Name"), "lshw", networkadapter, "logical name");
        addArticleStruct(tr("MAC Address"), "lshw", networkadapter, "serial");
        addArticleStruct(tr("Speed"), "lshw", networkadapter, "speed");
        addArticleStruct(tr("Width"), "lshw", networkadapter, "width");
        addArticleStruct(tr("Clock"), "lshw", networkadapter, "clock");
        addArticleStruct(tr("Capabilities"), "lshw", networkadapter, "capabilities");

        // 添加网络适配器的其他属性
        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", networkadapter, m_articles, m_existArticles,"ManulTrack__NetworkAdpater","NetworkAdpater information");

        QString deviceName = name.value;
        if (name.isValid() == false && description.isValid()) {
            deviceName = description.value;
        }

        addDevice(deviceName, m_articles, networkadapterList.size());

        if (networkadapterList.size() > 1) {
            QStringList tab = {
                deviceName,
                vendor.value
            };

            tabList.push_back(tab);
        }


        if (overviewInfo_.value.isEmpty() == false) {
            overviewInfo_.value += " / ";
        }

        // 添加概况预览
        if (vendor.isValid() && deviceName.contains(vendor.value, Qt::CaseInsensitive) == false) {
            overviewInfo_.value += vendor.value;
            overviewInfo_.value += " ";
        }
        overviewInfo_.value += deviceName;
    }

    if (networkadapterList.size() > 1) {
        QStringList headers = { tr("Name"),  tr("Vendor") };
        addTable(headers, tabList);
    }
}

void NetworkadapterWidget::initWidget()
{
    QStringList networkadapterList = DeviceInfoParser::Instance().getLshwNetworkadapterList();

    if (networkadapterList.size() < 1) {
        setCentralInfo(tr("No network adapter found"));
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    foreach (const QString &networkadapter, networkadapterList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        name.queryData("lshw", networkadapter, "product");
        articles.push_back(name);
        existArticles.insert("product");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("lshw", networkadapter, "vendor");
        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct description(tr("Description"));
        description.queryData("lshw", networkadapter, "description");
        articles.push_back(description);
        existArticles.insert("description");

        ArticleStruct version(tr("Version"));
        version.queryData("lshw", networkadapter, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct busInfo(tr("Bus Info"));
        busInfo.queryData("lshw", networkadapter, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct logicalName(tr("Logical Name"));
        logicalName.queryData("lshw", networkadapter, "logical name");
        articles.push_back(logicalName);
        existArticles.insert("logical name");

        ArticleStruct mac(tr("MAC Address"));
        mac.queryData("lshw", networkadapter, "serial");
        articles.push_back(mac);
        existArticles.insert("serial");

        ArticleStruct speed(tr("Speed", "Network Adapter"));
        speed.queryData("lshw", networkadapter, "speed");
        articles.push_back(speed);
        existArticles.insert("speed");

        ArticleStruct width(tr("Width", "Network Adapter"));
        width.queryData("lshw", networkadapter, "width");
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct clock(tr("Clock", "Network Adapter"));
        clock.queryData("lshw", networkadapter, "clock");
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities(tr("Capabilities", "Network Adapter"));
        capabilities.queryData("lshw", networkadapter, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct type(tr("Type"));
        type.queryData("lshw", networkadapter, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct autonegotiation(tr("autonegotiation"));
        vendor.queryData("lshw", networkadapter, "autonegotiation");
        articles.push_back(autonegotiation);
        existArticles.insert("autonegotiation");

        ArticleStruct broadcast(tr("broadcast"));
        broadcast.queryData("lshw", networkadapter, "broadcast");
        articles.push_back(broadcast);
        existArticles.insert("broadcast");

        ArticleStruct capacity(tr("capacity"));
        capacity.queryData("lshw", networkadapter, "capacity");
        articles.push_back(capacity);
        existArticles.insert("capacity");

        ArticleStruct driver(tr("driver"));
        driver.queryData("lshw", networkadapter, "driver");
        articles.push_back(driver);
        existArticles.insert("driver");

        ArticleStruct driverversion(tr("driverversion"));
        driverversion.queryData("lshw", networkadapter, "driverversion");
        articles.push_back(driverversion);
        existArticles.insert("driverversion");

        ArticleStruct duplex(tr("duplex"));
        duplex.queryData("lshw", networkadapter, "duplex");
        articles.push_back(duplex);
        existArticles.insert("duplex");

        ArticleStruct firmware(tr("firmware"));
        firmware.queryData("lshw", networkadapter, "firmware");
        articles.push_back(firmware);
        existArticles.insert("firmware");

        ArticleStruct ioport(tr("ioport"));
        ioport.queryData("lshw", networkadapter, "ioport");
        articles.push_back(ioport);
        existArticles.insert("ioport");

        ArticleStruct ip(tr("ip"));
        ip.queryData("lshw", networkadapter, "ip");
        articles.push_back(ip);
        existArticles.insert("ip");

        ArticleStruct irq(tr("irq"));
        irq.queryData("lshw", networkadapter, "irq");
        articles.push_back(irq);
        existArticles.insert("irq");

        ArticleStruct latency(tr("latency"));
        latency.queryData("lshw", networkadapter, "latency");
        articles.push_back(latency);
        existArticles.insert("latency");

        ArticleStruct link(tr("link"));
        link.queryData("lshw", networkadapter, "link");
        articles.push_back(link);
        existArticles.insert("link");

        ArticleStruct memory(tr("memory"));
        memory.queryData("lshw", networkadapter, "memory");
        articles.push_back(memory);
        existArticles.insert("memory");

        ArticleStruct multicast(tr("multicast"));
        multicast.queryData("lshw", networkadapter, "multicast");
        articles.push_back(multicast);
        existArticles.insert("multicast");

        ArticleStruct physicalId(tr("physical id"));
        physicalId.queryData("lshw", networkadapter, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct port(tr("port"));
        port.queryData("lshw", networkadapter, "port");
        articles.push_back(port);
        existArticles.insert("port");

        ArticleStruct size(tr("size"));
        size.queryData("lshw", networkadapter, "size");
        articles.push_back(ioport);
        existArticles.insert("size");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", networkadapter, articles, existArticles,"ManulTrack__NetworkAdpater","NetworkAdpater information");

        QString deviceName = name.value;
        if (name.isValid() == false && description.isValid()) {
            deviceName = description.value;
        }

        addDevice(deviceName, articles, networkadapterList.size());
        if (networkadapterList.size() > 1) {
            QStringList tab = {
                deviceName,
                vendor.value
            };

            tabList.push_back(tab);
        }


        if (overviewInfo_.value.isEmpty() == false) {
            overviewInfo_.value += " / ";
        }

        // 添加概况预览
        if (vendor.isValid() && deviceName.contains(vendor.value, Qt::CaseInsensitive) == false) {
            overviewInfo_.value += vendor.value;
            overviewInfo_.value += " ";
        }
        overviewInfo_.value += deviceName;
    }

    if (networkadapterList.size() > 1) {
        QStringList headers = { tr("Name"),  tr("Vendor") };
        addTable(headers, tabList);
    }
}
