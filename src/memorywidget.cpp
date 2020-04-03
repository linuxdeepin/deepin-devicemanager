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

#include "memorywidget.h"
#include "deviceinfoparser.h"
#include "DTableWidget"
#include <QVBoxLayout>
#include <DApplication>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Memory"))
{
    initWidget();
}

void MemoryWidget::initWidget()
{
    auto resultFromDmi = DeviceInfoParser::Instance().getDmidecodeMemoryList();
    if (resultFromDmi.isEmpty() == false) {
        init_l_Designer_l_TableWdiget();
        if (update_l_Designer_l_WholeDownWidget()) {
            return;
        }
    }
    /**
     * on some loogson computer,dmidecode doesn't print any dmi device info,
     * lshw show memory summary info without memory bank.
     * we need collect memory bank info if it exist in future version.
     */
    auto memorys = DeviceInfoParser::Instance().getLshwMeoryList();
    QSet<QString> existArticles;
    if (memorys.isEmpty() == false) {
        int index = 1;
        for (auto ptr = memorys.begin(); ptr != memorys.end(); ptr++) {
            QList<ArticleStruct> articles;
            auto mem = ptr.value().keys().value(index);
            ArticleStruct type(tr("Type"));
            type.queryData("lshw", mem, "Type");
            articles.push_back(type);
            existArticles.insert("Type");

            ArticleStruct description(tr("Description"));
            description.queryData("lshw", mem, "description");
            articles.push_back(description);
            existArticles.insert("Description");

            ArticleStruct physicalId(tr("Physical ID"));
            physicalId.queryData("lshw", mem, "physical id");
            articles.push_back(physicalId);
            existArticles.insert("physical id");

            ArticleStruct size(tr("Size"));
            size.queryData("lshw", mem, "size");
            articles.push_back(size);
            existArticles.insert("size");

            ArticleStruct slot(tr("Slot"));
            slot.queryData("lshw", mem, "slot");
            articles.push_back(slot);
            existArticles.insert("slot");
            foreach (auto key, ptr.value().keys()) {
                ArticleStruct art(key);
                art.value = ptr.value().value(key);

//                ArticleStruct type(tr("Type"));
//                type.queryData("lshw", key, "Type");
//                articles.push_back(type);
//                existArticles.insert("Type");

                art.value.replace(QRegExp("TiB"), "TB");
                art.value.replace(QRegExp("GiB"), "GB");
                art.value.replace(QRegExp("MiB"), "MB");
                art.value.replace(QRegExp("KiB"), "KB");

                if (key == QString("size")) {
                    overviewInfo_.value += overviewInfo_.value.isEmpty() ? "" : " /";
                    overviewInfo_.value += art.value;
                }
//                articles.append(art);
            }
            QString title = tr("Memory");
            addSubInfo(memorys.count() <= 1 ?
                       QString("%1\n").arg(title) :
                       QString("%1 %2\n").arg(title).arg(index),
                       articles);
            index ++;
        }
        return;
    }
    return setCentralInfo(tr("Failed to get memory information"));
}

void MemoryWidget::init_l_Designer_l_TableWdiget()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();
    if (memList.isEmpty()) {
        return;
    }
    QStringList headers = { tr("Name"), tr("Vendor"), tr("Type"), tr("Speed", "memory info"),  tr("Size"), /* tr("Status")*/};

    QList<QStringList> tabList;
    QDBusInterface mermorySize("com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo");
    quint64 size  = mermorySize.property("MemoryCap").toLongLong();
    double size1 = size / 1024;
    double size2 = size1 / 1024;
    double size3 = size2 / 1024;
//    QString size4 = QString("%1").arg(size3).mid() + "GB";
    QString sizeMemory = QString::number(size3, 'f', 1) + "GB";

    foreach (const QString &mem, memList) {
        if (canUpgrade_ == -1) {
            canUpgrade_ = 0;
        }

        QString speed = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Speed");
//        QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size");//3311

        if (isSlotValid(sizeMemory, speed) == false) {
            canUpgrade_ = 1;
            continue;
        }

        QStringList tab = {
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Part Number"),
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Manufacturer"),
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Type"),
            speed,
            sizeMemory,
            //3311

            //tr("Good")
        };

        tabList.push_back(tab);
    }
    addTable(headers, tabList);
}

bool MemoryWidget::update_l_Designer_l_WholeDownWidget()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();

    int validMemoryNumber = 0;

    QList<ArticleStruct> articles;

    ArticleStruct slotCount(tr("Slot Count"));
    slotCount.queryData("dmidecode", "Physical Memory Array", "Number Of Devices");
    articles.push_back(slotCount);
//3311
    ArticleStruct size(tr("Size"));
    QDBusInterface mermorySize("com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo");
    quint64 size1  = mermorySize.property("MemoryCap").toLongLong();
    double size2 = size1 / 1024;
    double size3 = size2 / 1024;
    double size4 = size3 / 1024;
//    QString size4 = QString("%1").arg(size3).mid() + "GB";
    size.value = QString::number(size4, 'f', 1) + "GB";
//    size.value(sizeMermory)
//    size.queryData("lshw", "Computer_core_memory", sizeMermory);

//    if (size.isValid() == false) {
//        int total = 0;
//        QString unitStr;

//        foreach (const QString &mem, memList) {
//            ArticleStruct strMem(tr("Size"));
//            strMem.queryData("dmidecode", mem, "Size");
//            if (strMem.isValid() && strMem.value.contains(" ")) {
//                QStringList lst = strMem.value.split(" ");
//                int memInstelled = lst.first().toInt();
//                if (memInstelled > 0) {
//                    total += memInstelled;
//                    unitStr = lst.last();
//                }
//            }
//        }

//        size.value = QString::number(total) + " " + unitStr;
//    }

//    size.value.replace("GiB", " GB");
//    size.value.replace("MiB", " MB");
//    articles.push_back(size);

    ArticleStruct mc(tr("Maximum Capacity"));
    mc.queryData("dmidecode", "Physical Memory Array", "Maximum Capacity");
    articles.push_back(mc);

    if (mc.isValid() && size.isValid()) {
        if ((mc.value.contains("Mb") && size.value.contains("Mb")) || (mc.value.contains("GB") && size.value.contains("GB"))) {
            if (mc.value.left(mc.value.size() - 2).toInt() > size.value.left(mc.value.size() - 2).toInt()) {
                canUpgrade_ = 1;
            }
        }
    }

    if (canUpgrade_ != -1) {
        ArticleStruct ug(tr("Upgradeable"));
        ug.value = canUpgrade_ ? tr("Yes") : tr("No");
        articles.push_back(ug);
    }

    summaryInfo_ = articles;

    articles.clear();
    QSet<QString> existArticles;


    QStringList detailMem;
    foreach (const QString &mem, memList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct model(tr("Model", "memory's model"));
        model.queryData("dmidecode", mem, "Part Number");
        articles.push_back(model);
        existArticles.insert("Part Number");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("dmidecode", mem, "Manufacturer");
        articles.push_back(vendor);
        existArticles.insert("Manufacturer");

        ArticleStruct locator(tr("Locator"));
        locator.queryData("dmidecode", mem, "Locator");
        articles.push_back(locator);
        existArticles.insert("Locator");

        ArticleStruct size(tr("Size", "memory's size"));
//        size.queryData("dmidecode", mem, "Size");
        QDBusInterface mermorySize("com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo");
        quint64 size1  = mermorySize.property("MemoryCap").toLongLong();
        double size2 = size1 / 1024;
        double size3 = size2 / 1024;
        double size4 = size3 / 1024;
        //    QString size4 = QString("%1").arg(size3).mid() + "GB";
        size.value = QString::number(size4, 'f', 1) + "GB";
        articles.push_back(size);
        existArticles.insert("Size");
//        if (mem.contains("Size")) {
//            continue;
//        }

        ArticleStruct type(tr("Type"));
        type.queryData("dmidecode", mem, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct speed(tr("Speed", "memory's Speed"));
        speed.queryData("dmidecode", mem, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct serial(tr("Serial Number"));
        serial.queryData("dmidecode", mem, "Serial Number");
        articles.push_back(serial);
        existArticles.insert("Serial Number");

        ArticleStruct formFactor(tr("Form Factor"));
        formFactor.queryData("dmidecode", mem, "Form Factor");
        articles.push_back(formFactor);
        existArticles.insert("Form Factor");

        ArticleStruct rank(tr("Rank"));
        rank.queryData("dmidecode", mem, "Rank");
        articles.push_back(rank);
        existArticles.insert("Rank");

        ArticleStruct bankLocator(tr("Bank Locator"));
        bankLocator.queryData("dmidecode", mem, "Bank Locator");
        articles.push_back(bankLocator);
        existArticles.insert("Bank Locator");

        ArticleStruct configVoltage(tr("Configured Voltage", "memory info"));
        configVoltage.queryData("dmidecode", mem, "Configured Voltage");
        articles.push_back(configVoltage);
        existArticles.insert("Configured Voltage");

        ArticleStruct minVoltage(tr("Minimum Voltage", "memory's config"));
        minVoltage.queryData("dmidecode", mem, "Minimum Voltage");
        articles.push_back(minVoltage);
        existArticles.insert("Minimum Voltage");

        ArticleStruct maxVoltage(tr("Maximum Voltage", "memory's config"));
        maxVoltage.queryData("dmidecode", mem, "Maximum Voltage");
        articles.push_back(maxVoltage);
        existArticles.insert("Maximum Voltage");

        ArticleStruct configSpeed(tr("Configured Speed"));
        configSpeed.queryData("dmidecode", mem, "Configured Memory Speed");
        articles.push_back(configSpeed);
        existArticles.insert("Configured Memory Speed");

        ArticleStruct arrayHandle(tr("Array Handle"));
        arrayHandle.queryData("dmidecode", mem, "Array Handle");
        articles.push_back(arrayHandle);
        existArticles.insert("Array Handle");

        ArticleStruct assetTag(tr("Asset Tag"));
        assetTag.queryData("dmidecode", mem, "Asset Tag");
        articles.push_back(assetTag);
        existArticles.insert("Asset Tag");

        ArticleStruct dataWidth(tr("Data Width"));
        dataWidth.queryData("dmidecode", mem, "Data Width");
        articles.push_back(dataWidth);
        existArticles.insert("Data Width");

        ArticleStruct errorInformationHandle(tr("Error Information Handle"));
        errorInformationHandle.queryData("dmidecode", mem, "Error Information Handle");
        articles.push_back(errorInformationHandle);
        existArticles.insert("Error Information Handle");

        ArticleStruct set(tr("Set"));
        set.queryData("dmidecode", mem, "Set");
        articles.push_back(set);
        existArticles.insert("Set");

        ArticleStruct totalWidth(tr("Total Width"));
        totalWidth.queryData("dmidecode", mem, "Total Width");
        articles.push_back(totalWidth);
        existArticles.insert("Total Width");

        ArticleStruct typeDetail(tr("Type Detail"));
        typeDetail.queryData("dmidecode", mem, "Type Detail");
        articles.push_back(typeDetail);
        existArticles.insert("Type Detail");

//        if (mem.contains("size", Qt::CaseInsensitive)) {
//            DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);
//        }
        DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);

        QString deviceName = vendor.value + " " + model.value;
        if (vendor.value == tr("Unknown") && model.value == tr("Unknown")) {
            deviceName = tr("Unknown memory");
        }
        if (deviceName.trimmed().isEmpty() == true) {
            deviceName = locator.value;
            if (deviceName.trimmed().isEmpty() == true) {
                deviceName = bankLocator.value;
            }
        }

        if (false == isSlotValid(size.value, speed.value)) {
            continue;
        }

        ++validMemoryNumber;

        addSubInfo(deviceName, articles);

        QList<ArticleStruct> arLst;
        arLst << vendor << type << speed;
        QString overviewVendor = joinArticle(arLst);

        if (false == detailMem .contains(overviewVendor)) {
            detailMem.push_back(overviewVendor);
        }
    }

    if (validMemoryNumber < 1) {
        return false;
    }

    overviewInfo_.value = size.value;
    if (detailMem.size() > 0) {
        QString detail = detailMem.join(" / ");
        if (detail.isEmpty() == false) {
            overviewInfo_.value += " (";
            overviewInfo_.value += detail;
            overviewInfo_.value += ")";
        }
    }
    return true;
}

void MemoryWidget::initTableWdiget_Good()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();

    QStringList headers = { "Bank", "Vendor", "Type", /*"Speed",*/  "Size", /* "Status"*/};

    QList<QStringList> tabList;

    foreach (const QString &mem, memList) {
        if (canUpgrade_ == -1) {
            canUpgrade_ = 0;
        }

        QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size");
        if (size == tr("Unknown") || size == "No Module Installed") {
            canUpgrade_ = 1;
            QStringList tab = {
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Locator"),
                "--",
                "--",
                "--",
                //tr("Bad")
            };

            tabList.push_back(tab);
            continue;
        }

        QStringList tab = {
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Locator"),
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Manufacturer"),
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Type") + " " +
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Speed"),
            DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size"),
            //tr("Good")
        };

        tabList.push_back(tab);
    }
    addTable(headers, tabList);
}

void MemoryWidget::updateWholeDownWidget_Good()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();

    QList<ArticleStruct> articles;

    ArticleStruct slotCount(tr("Slot Count", "memory info"));
    slotCount.queryData("dmidecode", "Physical Memory Array", "Number Of Devices");
    articles.push_back(slotCount);

    ArticleStruct size(tr("Size", "memory info"));
    size.queryData("lshw", "Computer_core_memory", "size");

    if (size.isValid() == false) {
        int total = 0;
        QString unitStr;

        foreach (const QString &mem, memList) {
            ArticleStruct strMem(tr("Size", "memory info"));
            strMem.queryData("dmidecode", mem, "Size");
            if (strMem.isValid() && strMem.value.contains(" ")) {
                QStringList lst = strMem.value.split(" ");
                int memInstelled = lst.first().toInt();
                if (memInstelled > 0) {
                    total += memInstelled;
                    unitStr = lst.last();
                }
            }
        }

        size.value = QString::number(total) + " " + unitStr;
    }

    size.value.replace("GiB", " GB");
    size.value.replace("MiB", " MB");
    articles.push_back(size);

    ArticleStruct mc(tr("Maximum Capacity"));
    mc.queryData("dmidecode", "Physical Memory Array", "Maximum Capacity");
    articles.push_back(mc);

    if (mc.isValid() && size.isValid()) {
        if ((mc.value.contains("Mb") && size.value.contains("Mb")) || (mc.value.contains("GB") && size.value.contains("GB"))) {
            if (mc.value.left(mc.value.size() - 2).toInt() > size.value.left(mc.value.size() - 2).toInt()) {
                canUpgrade_ = true;
            }
        }
    }



    if (canUpgrade_ != -1) {
        ArticleStruct ug(tr("Upgradeable"));
        ug.value = canUpgrade_ ? tr("Yes") : tr("No");
        articles.push_back(ug);
    }

    addInfo("Memory Info", articles);

    articles.clear();
    QSet<QString> existArticles;


    QStringList detailMem;
    foreach (const QString &mem, memList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct locator(tr("Locator"));
        locator.queryData("dmidecode", mem, "Locator");
        articles.push_back(locator);
        existArticles.insert("Locator");

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("dmidecode", mem, "Manufacturer");
        articles.push_back(vendor);
        existArticles.insert("Manufacturer");

//        ArticleStruct size(tr("Size", "memory info"));
//        size.queryData("dmidecode", mem, "Size");
//        articles.push_back(size);
//        existArticles.insert("Size");

        ArticleStruct type(tr("Type", "memory info"));
        type.queryData("dmidecode", mem, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct speed(tr("Speed", "memory info"));
        speed.queryData("dmidecode", mem, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct serial(tr("Serial Number", "memory info"));
        serial.queryData("dmidecode", mem, "Serial Number");
        articles.push_back(serial);
        existArticles.insert("Serial Number");

        ArticleStruct model(tr("Model Part Number", "memory info"));
        model.queryData("dmidecode", mem, "Part Number");
        articles.push_back(model);
        existArticles.insert("Part Number");

        ArticleStruct formFactor(tr("Form Factor", "memory info"));
        formFactor.queryData("dmidecode", mem, "Form Factor");
        articles.push_back(formFactor);
        existArticles.insert("Form Factor");

        ArticleStruct rank(tr("Rank", "memory info"));
        rank.queryData("dmidecode", mem, "Rank");
        articles.push_back(rank);
        existArticles.insert("Rank");

        ArticleStruct bankLocator(tr("Bank Locator", "memory info"));
        bankLocator.queryData("dmidecode", mem, "Bank Locator");
        articles.push_back(bankLocator);
        existArticles.insert("Bank Locator");

        ArticleStruct configVoltage(tr("Configured Voltage", "memory info"));
        configVoltage.queryData("dmidecode", mem, "Configured Voltage");
        articles.push_back(configVoltage);
        existArticles.insert("Configured Voltage");

        ArticleStruct minVoltage(tr("Minimum Voltage"));
        minVoltage.queryData("dmidecode", mem, "Minimum Voltage");
        articles.push_back(minVoltage);
        existArticles.insert("Minimum Voltage");

        ArticleStruct maxVoltage(tr("Maximum Voltage", "memory info"));
        maxVoltage.queryData("dmidecode", mem, "Maximum Voltage");
        articles.push_back(maxVoltage);
        existArticles.insert("Maximum Voltage");

        ArticleStruct configSpeed(tr("Configured Speed", "memory info"));
        configSpeed.queryData("dmidecode", mem, "Configured Memory Speed");
        articles.push_back(configSpeed);
        existArticles.insert("Configured Memory Speed");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);

        QString deviceName;
        if (locator.isValid()) {
            deviceName = locator.value;
        } else if (bankLocator.isValid()) {
            deviceName = bankLocator.value;
        } else if (rank.isValid()) {
            deviceName = rank.value;
        }

        if (size.value == tr("Unknown") || size.value == "No Module Installed") {
            articles.clear();
            ArticleStruct status(tr("Status", "memory info"));
            status.value = tr("Bad", "memory info");
            articles.push_back(status);

            addSubInfo(deviceName, articles);
            continue;
        }
        addSubInfo(deviceName, articles);

        QString overviewVendor = vendor.value;
        overviewVendor += " ";
        overviewVendor += type.value;
        overviewVendor += " ";
        overviewVendor += speed.value;

        if (false == detailMem .contains(overviewVendor)) {
            detailMem.push_back(overviewVendor);
        }
    }

    overviewInfo_.value = size.value;
    if (detailMem.size() > 0) {
        QString detail  = detailMem.join("/");
        if (detail.isEmpty() == false) {
            overviewInfo_.value += " (";
            overviewInfo_.value += detail;
            overviewInfo_.value += ")";
        }
    }
}

bool MemoryWidget::isSlotValid(const QString &size, const QString &speed)
{
    if (size.contains("Unknown", Qt::CaseInsensitive) || size.contains("No Module Installed", Qt::CaseInsensitive)) {
        return false;
    }
    if (speed.contains("MT/s") == false) { //maybe weak
        return false;
    }

    return true;
}
