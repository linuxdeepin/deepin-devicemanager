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

#include "audiodevicewidget.h"
#include "deviceinfoparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>

DWIDGET_USE_NAMESPACE

AudiodeviceWidget::AudiodeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Sound Adapter"))
{
    //initWidget();

    // 先从lshw命令和cat /proc/bus/input/devices 中获取音频列表
    QStringList multimediaList, inputdeviceList;
    int totalSize = getAudioList(multimediaList, inputdeviceList);

    // 如果都没有则直接返回
    if (totalSize < 1) {
        setCentralInfo(tr("No sound adapter found"));
        return;
    }

    // ***************************************************************************************
    QList<QStringList> tabList;
    // 从lshw中获取显示信息
    getInfoFromLshw(tabList, multimediaList);

    // 从cat /proc/bus/input/devices中获取信息
    getInfoFromCatInput(tabList, inputdeviceList);

    // 添加表格内容
    QStringList headers = { tr("Name"),  tr("Vendor")};
    addTable(headers, tabList);

}

int AudiodeviceWidget::getAudioList(QStringList &lshwList, QStringList &catList)
{
    lshwList = DeviceInfoParser::Instance().getLshwMultimediaList();
    catList  = DeviceInfoParser::Instance().getCatinputAudioDeviceList();
    return lshwList.size() + catList.size();
}

void AudiodeviceWidget::getInfoFromLshw(QList<QStringList> &tabList, const QStringList &lshwList)
{
    foreach (auto lshwMedia, lshwList) {
        QStringList tabItem;
        addLshwAudioInfo(lshwMedia, tabItem);
        tabList.append(tabItem);
    }
}

void AudiodeviceWidget::addLshwAudioInfo(const QString &item, QStringList &tabItem)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    // 获取总线信息 和 名称
    ArticleStruct busInfo(tr("Bus Info", "Audio Device Info"));
    ArticleStruct name(tr("Name", "Audio Device Info"));
    getBusInfoAndName(item, busInfo, name, existArticles);

    // 获取设备商信息
    ArticleStruct vendor(tr("Vendor", "Audio Device Info"));
    vendor.queryData("lshw", item, "vendor", existArticles);

    // 获取产品描述信息
    ArticleStruct description(tr("Description"));
    description.queryData("lshw", item, "description", existArticles);

    // 这一句没有琢磨是什么意思
    if (name.value == vendor.value) {
        name.value = description.value;
    }

    // *******************************************************
    articles.push_back(name);
    articles.push_back(vendor);
    articles.push_back(description);
    articles.push_back(busInfo);

    // 添加设备的其它信息
    DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", item, articles, existArticles,
                                                          "ManulTrack__AudioDevices", "Audio device infomation from lshw");
    //addDevice( name.value , articles,1);
    addSubInfo(name.value, articles);

    // 获取表格内容
    tabItem.append(name.value.remove(vendor.value));
    tabItem.append(vendor.value);

    // 获取概况
    if (overviewInfo_.value.isEmpty() == false) {
        overviewInfo_.value += " / ";
    }
    if (vendor.isValid() && name.value.contains(vendor.value, Qt::CaseInsensitive) == false) {
        overviewInfo_.value += vendor.value;
        overviewInfo_.value += " ";
    }
    overviewInfo_.value += name.value;
}

void AudiodeviceWidget::addCatAudioInfo(const QString &item, QStringList &tabItem)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    // 获取名称
    ArticleStruct name(tr("Name"));
    name.queryData("catinput", item, "Name");
    name.value.remove("\"");
    articles.push_back(name);
    existArticles.insert("Name");

    // 获取设备商
    ArticleStruct vendor(tr("Vendor"));
    vendor.queryData("catinput", item, "Vendor", existArticles, articles);

    // 获取版本信息
    ArticleStruct vesion(tr("Version"));
    vesion.queryData("catinput", item, "Version", existArticles, articles);

    // 获取Sysfs信息
    ArticleStruct sysfs(tr("Sysfs", "Audio Device Info"));
    sysfs.queryData("catinput", item, "Sysfs", existArticles, articles);

    // 添加其他的设备信息
    DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", item, articles, existArticles,
                                                          "ManulTrack__AudioDevices", "Audio device infomation from catinput");
    addSubInfo(name.value, articles);

    // 获取表格内容
    tabItem.append(name.value);
    tabItem.append(vendor.value);
}

void AudiodeviceWidget::getBusInfoAndName(const QString &item, ArticleStruct &busInfo, ArticleStruct &name, QSet<QString> &existArticles)
{
    busInfo.queryData("lshw", item, "bus info", existArticles);
    QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");
    QString pci_bus;
    if (reg.exactMatch(busInfo.value)) {
        pci_bus = reg.cap(1);
    }

    QString lspciDeviceName;
    DeviceInfoParser::Instance().fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

    name.queryData("lspci", lspciDeviceName, "Name");
    name.queryData("lshw", item, "product", existArticles);
    name.value.remove(" Corporation", Qt::CaseInsensitive);
    int index = name.value.indexOf('(');
    if (index > 0) {
        name.value = name.value.mid(0, index);
    }
}

void AudiodeviceWidget::getInfoFromCatInput(QList<QStringList> &tabList, const QStringList &catList)
{
    foreach (auto catMedia, catList) {
        QStringList tabItem;
        addCatAudioInfo(catMedia, tabItem);
        tabList.append(tabItem);
    }
}

void AudiodeviceWidget::initWidget()
{
    QStringList multimediaList = DeviceInfoParser::Instance().getLshwMultimediaList();
    QStringList inputdeviceList = DeviceInfoParser::Instance().getCatinputAudioDeviceList();

    if (multimediaList.size() + inputdeviceList.size() < 1) {
        setCentralInfo(tr("No sound adapter found"));
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach (auto multimedia, multimediaList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct busInfo(tr("Bus Info", "Audio Device Info"));
        busInfo.queryData("lshw", multimedia, "bus info", existArticles);
        QRegExp reg("^pci@[0-9]*:([\\s\\S]*)$");

        QString pci_bus;
        if (reg.exactMatch(busInfo.value)) {
            pci_bus = reg.cap(1);
        }

        QString lspciDeviceName;
        DeviceInfoParser::Instance().fuzzeyQueryKey("lspci", pci_bus, lspciDeviceName);

        ArticleStruct name(tr("Name", "Audio Device Info"));
        name.queryData("lspci", lspciDeviceName, "Name");
        name.queryData("lshw", multimedia, "product", existArticles);

        name.value.remove(" Corporation", Qt::CaseInsensitive);
        int index = name.value.indexOf('(');
        if (index > 0) {
            name.value = name.value.mid(0, index);
        }

        ArticleStruct vendor(tr("Vendor", "Audio Device Info"));
        vendor.queryData("lshw", multimedia, "vendor", existArticles);

        ArticleStruct description(tr("Description"));
        description.queryData("lshw", multimedia, "description");

        existArticles.insert("description");

        if (name.value == vendor.value) {
            name.value = description.value;
        }

        articles.push_back(name);
        articles.push_back(vendor);
        articles.push_back(description);
        articles.push_back(busInfo);

        ArticleStruct version(tr("Version", "Audio Device Info"));
        version.queryData("lshw", multimedia, "version", existArticles, articles);
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct width(tr("Width", "Audio Device Info"));
        width.queryData("lshw", multimedia, "width", existArticles, articles);
        articles.push_back(width);
        existArticles.insert("width");

        ArticleStruct multimediaNum(tr("multimedia", "Audio Device Info"));
        multimediaNum.queryData("lshw", multimedia, "multimedia", existArticles, articles);
        articles.push_back(multimediaNum);
        existArticles.insert("multimedia");

        ArticleStruct physicalId(tr("physical id", "Audio Device Info"));
        physicalId.queryData("lshw", multimedia, "physical id", existArticles, articles);
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct clock(tr("Clock", "Audio Device Info"));
        clock.queryData("lshw", multimedia, "clock", existArticles, articles);
        articles.push_back(clock);
        existArticles.insert("clock");

        ArticleStruct capabilities(tr("Capabilities", "Audio Device Info"));
        capabilities.queryData("lshw", multimedia, "capabilities", existArticles, articles);
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct driver(tr("driver", "Audio Device Info"));
        driver.queryData("lshw", multimedia, "driver", existArticles, articles);
        articles.push_back(driver);
        existArticles.insert("driver");

        ArticleStruct irq(tr("irq", "Audio Device Info"));
        irq.queryData("lshw", multimedia, "irq", existArticles, articles);
        articles.push_back(irq);
        existArticles.insert("irq");

        ArticleStruct latency(tr("latency", "Audio Device Info"));
        latency.queryData("lshw", multimedia, "latency", existArticles, articles);
        articles.push_back(latency);
        existArticles.insert("latency");

        ArticleStruct memory(tr("memory", "Audio Device Info"));
        memory.queryData("lshw", multimedia, "memory", existArticles, articles);
        articles.push_back(memory);
        existArticles.insert("memory");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", multimedia, articles, existArticles,
                                                              "ManulTrack__AudioDevices", "Audio device infomation from lshw");
        addDevice(name.value, articles, multimediaList.size() + inputdeviceList.size());

        if (multimediaList.size() + inputdeviceList.size() > 1) {
            QStringList tab = {
                name.value.remove(vendor.value),
                vendor.value
            };

            tabList.push_back(tab);
        }


        if (overviewInfo_.value.isEmpty() == false) {
            overviewInfo_.value += " / ";
        }

        if (vendor.isValid() && name.value.contains(vendor.value, Qt::CaseInsensitive) == false) {
            overviewInfo_.value += vendor.value;
            overviewInfo_.value += " ";
        }
        overviewInfo_.value += name.value;
    }

    foreach (const QString &device, inputdeviceList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        name.queryData("catinput", device, "Name");
        name.value.remove("\"");
        articles.push_back(name);
        existArticles.insert("Name");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("catinput", device, "Vendor", existArticles, articles);

        ArticleStruct vesion(tr("Version"));
        vesion.queryData("catinput", device, "Version", existArticles, articles);

        ArticleStruct sysfs(tr("Sysfs", "Audio Device Info"));
        sysfs.queryData("catinput", device, "Sysfs", existArticles, articles);

        ArticleStruct bus(tr("Bus"));
        vendor.queryData("catinput", device, "Bus", existArticles, articles);

        ArticleStruct product(tr("Product"));
        vesion.queryData("catinput", device, "Product", existArticles, articles);

//        ArticleStruct sysfs(tr("Sysfs", "Audio Device Info"));
//        sysfs.queryData("catinput", device, "Sysfs", existArticles, articles);

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("catinput", device, articles, existArticles,
                                                              "ManulTrack__AudioDevices", "Audio device infomation from catinput");
        addSubInfo(name.value, articles);

        if (multimediaList.size() + inputdeviceList.size() > 1) {
            QStringList tab = {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }
    }

    if (multimediaList.size() + inputdeviceList.size() > 1) {
        QStringList headers = { tr("Name"),  tr("Vendor")};
        addTable(headers, tabList);
    }
}
