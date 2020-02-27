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

#include "motherboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

MotherboardWidget::MotherboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Motherboard"))
{
    initWidget();
}

void MotherboardWidget::initWidget()
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList names = {   tr("Model"),
                            tr("Vendor"),
                            tr("Chipset Family"),
                            tr("BIOS")  + tr("Version"),
                            tr("Features")
                        };

    ArticleStruct vendor(tr("Vendor"));
    vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    if(vendor.isValid() == false)
    {
        vendor.queryData("catbaseboard", "Base Board Information", "Manufacturer");
    }
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct model(tr("Model"));
    //QString manufactor = DeviceInfoParser::Instance().queryData("dmidecode", "Base Board Information", "Manufacturer");
    model.value = /*manufactor + " "+*/ DeviceInfoParser::Instance().queryData("dmidecode", "Base Board Information", "Product Name");
    if(model.isValid() == false)
    {
        model.queryData("catbaseboard", "Base Board Information", "Board name");
        existArticles.insert("Board name");
    }
    articles.push_back(model);
    existArticles.insert("Product Name");

    ArticleStruct version(tr("Version"));
    version.queryData("dmidecode", "Base Board Information", "Version");
    articles.push_back(version);
    existArticles.insert("Version");

    ArticleStruct chipsetFamily(tr("Chipset Family"));
    chipsetFamily.value = DeviceInfoParser::Instance().fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily.value) )
    {
        chipsetFamily.value =  rx.cap(1)+" " + tr("Chipset Family");
    }
    if(chipsetFamily.isValid() == false)
    {
        chipsetFamily.queryData("catbaseboard", "Base Board Information", "Family");
        existArticles.insert("Family");
    }
    articles.push_back(chipsetFamily);

    model.value.remove(vendor.value);
    chipsetFamily.value.remove(vendor.value);
    model.value.remove(" Company");
    model.value.remove(" Company");

    QList<ArticleStruct> aList;
    aList << vendor << model;
    overviewInfo_.value = joinArticle(aList);

    if( chipsetFamily.isValid())
    {
        overviewInfo_.value += " (";
        overviewInfo_.value += chipsetFamily.value;
        overviewInfo_.value += ")";
    }

    ArticleStruct serial(tr("Serial Number"));
    serial.queryData("dmidecode", "Base Board Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    existArticles.insert("Features");

    bool res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", "Base Board Information", articles, existArticles,
                                                                     "ManulTrack__MotherBoard_dmidecode","mother board info from dmidecode");
    if(res == false)
    {
        res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("catbaseboard", "Base Board Information", articles, existArticles,
                                                                    "ManulTrack__MotherBoard_catbaseboard","mother board info from catbaseboard");
    }

    ArticleStruct SMBIOSVersion(tr("SMBIOS Version"));
    SMBIOSVersion.queryData("dmidecode", "SMBIOS", "version");
    articles.push_back(SMBIOSVersion);

    ArticleStruct features(tr("Features"));
    features.queryData("dmidecode", "Base Board Information", "Features");
    articles.push_back(features);

    addInfo("", articles);

    articles.clear();
    existArticles.clear();

    vendor.value.clear();
    vendor.queryData("dmidecode", "System Information", "Manufacturer");
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct productName(tr("Product Name"));
    productName.queryData("dmidecode", "System Information", "Product Name");
    articles.push_back(productName);
    existArticles.insert("Product Name");

    serial.value.clear();
    serial.queryData("dmidecode", "System Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    version.value.clear();
    version.queryData("dmidecode", "System Information", "Version");
    articles.push_back(version);
    existArticles.insert("Version");

    ArticleStruct uuid(tr("UUID"));
    uuid.queryData("dmidecode", "System Information", "UUID");
    articles.push_back(uuid);
    existArticles.insert("UUID");

    ArticleStruct wakeupType(tr("Wake-up Type"));
    wakeupType.queryData("dmidecode", "System Information", "Wake-up Type");
    articles.push_back(wakeupType);
    existArticles.insert("Wake-up Type");

    ArticleStruct sku(tr("SKU Number"));
    sku.queryData("dmidecode", "System Information", "SKU Number");
    articles.push_back(sku);
    existArticles.insert("SKU Number");

    existArticles.insert("Features");

    res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", "System Information", articles, existArticles,
                                                                "ManulTrack__MotherBoard_SystemInfo","System info of mother board from dmidecode");

    //features.queryData("dmidecode", "System Information", "Features");
    //articles.push_back(features);

    if (hasValidInfo(articles)) {
        addSubInfo(tr("System Information"), articles);
    }

    addMemoryInfo();

    articles.clear();
    existArticles.clear();

    ArticleStruct biosVersion(tr("BIOS Version"));
    biosVersion.queryData("dmidecode", "BIOS Information", "Version");
    articles.push_back(biosVersion);
    existArticles.insert("Version");
    if(biosVersion.isValid() == false)
    {
        biosVersion.queryData("catbaseboard", "BIOS Information", "Version");
    }


    ArticleStruct releaseDate(tr("Release Date"));
    releaseDate.queryData("dmidecode", "BIOS Information", "Release Date");
    if(releaseDate.isValid() == false)
    {
        releaseDate.queryData("catbaseboard", "BIOS Information", "Release Date");
    }
    articles.push_back(releaseDate);
    existArticles.insert("Release Date");


    ArticleStruct biosVendor(tr("BIOS Vendor"));
    biosVendor.queryData("dmidecode", "BIOS Information", "Vendor");
    if(biosVendor.isValid() == false)
    {
        biosVendor.queryData("catbaseboard", "BIOS Information", "Vendor");
    }
    articles.push_back(biosVendor);
    existArticles.insert("Vendor");

    existArticles.insert("Characteristics");
    res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", "BIOS Information", articles, existArticles,
                                                                "ManulTrack__BiosInfo_dmi","bios info of motherboard from dmidecode");
    if(res == false)
    {
        res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("catbaseboard", "BIOS Information", articles, existArticles,
                                                                    "ManulTrack__BiosInfo_catbaseboard","bios info of motherboard from catbaseboard");
    }

    DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", "BIOS Language Information", articles,QSet<QString>(),
                                                          "ManulTrack__Bios_lang","BIOS Language Information");
    ArticleStruct characteristics(tr("Characteristics"));
    characteristics.queryData("dmidecode", "BIOS Information", "Characteristics");
    articles.push_back(characteristics);

    if (hasValidInfo(articles)) {
        addSubInfo(tr("BIOS"), articles);
    }

    articles.clear();
    existArticles.clear();

    articles.clear();
    existArticles.clear();

    ArticleStruct chassisVendor(tr("Vendor"));
    chassisVendor.queryData("dmidecode", "Chassis Information", "Manufacturer");
    articles.push_back(chassisVendor);
    existArticles.insert("Manufacturer");

    ArticleStruct chassisType(tr("Type","Chassis Information"));
    chassisType.queryData("dmidecode", "Chassis Information", "Type");
    articles.push_back(chassisType);
    existArticles.insert("Type");

    ArticleStruct chassisSN(tr("Serial Number"));
    chassisSN.queryData("dmidecode", "Chassis Information", "Serial Number");
    articles.push_back(chassisSN);
    existArticles.insert("Serial Number");

    ArticleStruct chassisVersion(tr("Version","Chassis Information"));
    chassisVersion.queryData("dmidecode", "Chassis Information", "Version");
    articles.push_back(chassisVersion);
    existArticles.insert("Version");

    res = DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", "Chassis Information", articles, existArticles,
                                                                "ManulTrack__Chassis information","Chassis");
    if (hasValidInfo(articles)) {
         addSubInfo(tr("Chassis Information"), articles);
    }
}

void MotherboardWidget::addMemoryInfo()
{
    int canUpgrade = -1;
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();
    foreach(const QString& mem, memList)
    {
        if(canUpgrade == -1)
        {
            canUpgrade = 0;
        }

        QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size");
        if( size == tr("Unknown") || size == "No Module Installed" )
        {
            canUpgrade = 1;
            continue;
        }
    }

    QList<ArticleStruct> articles;

    ArticleStruct slotCount(tr("Slot Count"));
    slotCount.queryData("dmidecode", "Physical Memory Array", "Number Of Devices");
    articles.push_back(slotCount);

    ArticleStruct size(tr("Size","Computer_core_memory"));
    size.queryData("lshw", "Computer_core_memory", "size");

    if(size.isValid() == false)
    {
        int total = 0;
        QString unitStr;

        foreach(const QString& mem, memList)
        {
            ArticleStruct strMem(tr("Size","memory size"));
            strMem.queryData("dmidecode", mem, "Size");
            if(strMem.isValid() && strMem.value.contains(" "))
            {
                QStringList lst = strMem.value.split(" ");
                int memInstelled = lst.first().toInt();
                if(memInstelled > 0)
                {
                    total += memInstelled;
                    unitStr = lst.last();
                }
            }
        }

        size.value = QString::number(total) + " " + unitStr;
    }

    size.value.replace( "GiB", " GB" );
    size.value.replace( "MiB", " MB" );
    articles.push_back(size);

    ArticleStruct mc(tr("Maximum Capacity","PhysicMemory"));
    QStringList pmList = DeviceInfoParser::Instance().getDmidecodePhysicMemory();
    foreach(auto pm , pmList)
    {
        if(mc.value.isEmpty() == false)
        {
            mc.value += " + ";
        }
        mc.value += DeviceInfoParser::Instance().queryData("dmidecode", pm, "Maximum Capacity");
    }

    articles.push_back(mc);

    if( mc.isValid() && size.isValid())
    {
        if( (mc.value.contains("Mb") && size.value.contains("Mb")) || (mc.value.contains("GB") && size.value.contains("GB")) )
        {
            if(mc.value.left(mc.value.size() - 2 ).toInt() > size.value.left(mc.value.size() -2 ).toInt())
            {
                canUpgrade = 1;
            }
        }
    }

    if(canUpgrade != -1)
    {
        ArticleStruct ug(tr("Upgradeable"));
        ug.value = canUpgrade ?tr("Yes") : tr("No");
        articles.push_back(ug);
    }

    addSubInfo(tr("Memory Info"), articles);
}

bool MotherboardWidget::hasValidInfo(QList<ArticleStruct> &arts)
{
    foreach (auto art,arts) {
        if (art.isValid()) {
            return true;
        }
    }
    return false;
}
