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

#include "motherboardwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

MotherboardWidget::MotherboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Motherboard"))
{
    initWidget();
}

void MotherboardWidget::initWidget()
{
    //setTitle(DApplication::translate("Main", "Motherboard")  + DApplication::translate("Main", " Info"));
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    QStringList names = {   DApplication::translate("Main", "Model"),
                            DApplication::translate("Main", "Vendor"),
                            DApplication::translate("Main", "Chipset Family"),
                            DApplication::translate("Main", "Bios")  + DApplication::translate("Main", "Version"),
                            DApplication::translate("Main", "Features")
                        };

    ArticleStruct vendor("Vendor");
    vendor.queryData("dmidecode", "Base Board Information", "Manufacturer");
    if(vendor.isValid() == false)
    {
        vendor.queryData("catbaseboard", "Base Board Information", "Manufacturer");
    }
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct model("Model");
    //QString manufactor = DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Manufacturer");
    model.value = /*manufactor + " "+*/ DeviceInfoParserInstance.queryData("dmidecode", "Base Board Information", "Product Name");
    if(model.isValid() == false)
    {
        model.queryData("catbaseboard", "Base Board Information", "Board name");
        existArticles.insert("Board name");
    }
    articles.push_back(model);
    existArticles.insert("Product Name");

    ArticleStruct version("Version");
    version.queryData("dmidecode", "Base Board Information", "Version");
    articles.push_back(version);
    existArticles.insert("Version");

    ArticleStruct chipsetFamily("Chipset Family");
    chipsetFamily.value = DeviceInfoParserInstance.fuzzyQueryData("lspci", "ISA bridge", "Subsystem");
    QRegExp rx("^[\\s\\S]*\\(([\\S]*)\\)$");
    if( rx.exactMatch(chipsetFamily.value) )
    {
        chipsetFamily.value =  rx.cap(1)+" " + DApplication::translate("Main", "Chipset Family");
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

    if( chipsetFamily.isValid() )
    {
        overviewInfo_.value += " (";
        overviewInfo_.value += chipsetFamily.value;
        overviewInfo_.value += ")";
    }

    ArticleStruct serial("Serial Number");
    serial.queryData("dmidecode", "Base Board Information", "Serial Number");
    articles.push_back(serial);
    existArticles.insert("Serial Number");

    existArticles.insert("Features");

    bool res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "Base Board Information", articles, existArticles);
    if(res == false)
    {
        res = DeviceInfoParserInstance.queryRemainderDeviceInfo("catbaseboard", "Base Board Information", articles, existArticles);
    }

    ArticleStruct SMBIOSVersion("SMBIOS Version");
    SMBIOSVersion.queryData("dmidecode", "SMBIOS", "version");
    articles.push_back(SMBIOSVersion);

    ArticleStruct features("Features");
    features.queryData("dmidecode", "Base Board Information", "Features");
    articles.push_back(features);

    if(res)
    {
        addInfo( "", articles );
    }

    addMemoryInfo();

    articles.clear();
    existArticles.clear();

    vendor.value.clear();
    vendor.queryData("dmidecode", "System Information", "Manufacturer");
    articles.push_back(vendor);
    existArticles.insert("Manufacturer");

    ArticleStruct productName("Product Name");
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

    ArticleStruct uuid("UUID");
    uuid.queryData("dmidecode", "System Information", "UUID");
    articles.push_back(uuid);
    existArticles.insert("UUID");

    ArticleStruct wakeupType("Wake-up Type");
    wakeupType.queryData("dmidecode", "System Information", "Wake-up Type");
    articles.push_back(wakeupType);
    existArticles.insert("Wake-up Type");

    ArticleStruct sku("SKU Number");
    sku.queryData("dmidecode", "System Information", "SKU Number");
    articles.push_back(sku);
    existArticles.insert("SKU Number");

    existArticles.insert("Features");

    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "System Information", articles, existArticles);

    //features.queryData("dmidecode", "System Information", "Features");
    //articles.push_back(features);

    if(res)
    {
        addSubInfo( "System Information", articles, DeviceWidgetContentMarginLeft_ );
    }

    articles.clear();
    existArticles.clear();

    ArticleStruct biosVersion("Bios Version");
    biosVersion.queryData("dmidecode", "BIOS Information", "Version");
    articles.push_back(biosVersion);
    existArticles.insert("Version");
    if(biosVersion.isValid() == false)
    {
        biosVersion.queryData("catbaseboard", "BIOS Information", "Version");
    }


    ArticleStruct releaseDate("Release Date");
    releaseDate.queryData("dmidecode", "BIOS Information", "Release Date");
    if(releaseDate.isValid() == false)
    {
        releaseDate.queryData("catbaseboard", "BIOS Information", "Release Date");
    }
    articles.push_back(releaseDate);
    existArticles.insert("Release Date");


    ArticleStruct biosVendor("Bios Vendor");
    biosVendor.queryData("dmidecode", "BIOS Information", "Vendor");
    if(biosVendor.isValid() == false)
    {
        biosVendor.queryData("catbaseboard", "BIOS Information", "Vendor");
    }
    articles.push_back(biosVendor);
    existArticles.insert("Vendor");

    existArticles.insert("Characteristics");
    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "BIOS Information", articles, existArticles);
    if(res == false)
    {
        res = DeviceInfoParserInstance.queryRemainderDeviceInfo("catbaseboard", "BIOS Information", articles, existArticles);
    }

    DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "BIOS Language Information", articles);

    ArticleStruct characteristics("Characteristics");
    characteristics.queryData("dmidecode", "BIOS Information", "Characteristics");
    articles.push_back(characteristics);

    if(res)
    {
        addSubInfo( "Bios", articles, DeviceWidgetContentMarginLeft_ );
    }

    articles.clear();
    existArticles.clear();

    articles.clear();
    existArticles.clear();

    ArticleStruct chassisVendor("Vendor");
    chassisVendor.queryData("dmidecode", "Chassis Information", "Manufacturer");
    articles.push_back(chassisVendor);
    existArticles.insert("Manufacturer");

    ArticleStruct chassisType("Type");
    chassisType.queryData("dmidecode", "Chassis Information", "Type");
    articles.push_back(chassisType);
    existArticles.insert("Type");

    ArticleStruct chassisSN("Serial Number");
    chassisSN.queryData("dmidecode", "Chassis Information", "Serial Number");
    articles.push_back(chassisSN);
    existArticles.insert("Serial Number");

    ArticleStruct chassisVersion("Version");
    chassisVersion.queryData("dmidecode", "Chassis Information", "Version");
    articles.push_back(chassisVersion);
    existArticles.insert("Version");

    res = DeviceInfoParserInstance.queryRemainderDeviceInfo("dmidecode", "Chassis Information", articles, existArticles);

    if(res)
    {
        addSubInfo( "Chassis Information", articles, DeviceWidgetContentMarginLeft_);
    }

}

void MotherboardWidget::addMemoryInfo()
{
    int canUpgrade = -1;
    QStringList memList = DeviceInfoParserInstance.getDimdecodeMemoryList();
    foreach(const QString& mem, memList)
    {
        if(canUpgrade == -1)
        {
            canUpgrade = 0;
        }

        QString size = DeviceInfoParserInstance.queryData("dmidecode", mem, "Size");
        if( size == DApplication::translate("Main", "Unknown") || size == "No Module Installed" )
        {
            canUpgrade = 1;
            continue;
        }
    }

    QList<ArticleStruct> articles;

    ArticleStruct slotCount("Slot Count");
    slotCount.queryData("dmidecode", "Physical Memory Array", "Number Of Devices");
    articles.push_back(slotCount);

    ArticleStruct size("Size");
    size.queryData("lshw", "Computer_core_memory", "size");

    if(size.isValid() == false)
    {
        int total = 0;
        QString unitStr;

        foreach(const QString& mem, memList)
        {
            ArticleStruct strMem("Size");
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

    ArticleStruct mc("Maximum Capacity");
    QStringList pmList = DeviceInfoParserInstance.getDimdecodePhysicMemory();
    foreach(auto pm , pmList)
    {
        if(mc.value.isEmpty() == false)
        {
            mc.value += " + ";
        }
        mc.value += DeviceInfoParserInstance.queryData("dmidecode", pm, "Maximum Capacity");
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
        ArticleStruct ug("Upgradeable");
        ug.value = canUpgrade ? DApplication::translate("Main", "Yes") : DApplication::translate("Main", "No");
        articles.push_back(ug);
    }

    addSubInfo("Memory Bank Info", articles, DeviceWidgetContentMarginLeft_);
}
