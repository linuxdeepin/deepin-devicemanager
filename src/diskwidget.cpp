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

#include "diskwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "Storage"))
{
    initWidget();
}

void DiskWidget::initWidget()
{
    QStringList diskList = DeviceInfoParserInstance.getLshwDisknameList();
    if(diskList.size() < 1)
    {
        setCentralInfo("No Disk found!");
        return;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int i = 0;
    foreach(const QString& disk, diskList)
    {
        QString logicalName = DeviceInfoParserInstance.queryData("lshw", disk, "logical name");

//        DeviceInfoParserInstance.loadSmartctlDatabase(logicalName);

        QString modelStr = DeviceInfoParserInstance.queryData("lshw", disk, "product");
        QString vendorStr = DeviceInfoParserInstance.queryData("lshw", disk, "vendor");
        QString mediaTypeStr = "Unknown";
        QString sizeStr = DeviceInfoParserInstance.queryData("lshw", disk, "size");
        sizeStr.replace("GiB","GB");

        articles.clear();
        existArticles.clear();

        ArticleStruct model("Model");
        model.value = modelStr;
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor("Vendor");
        vendor.value = vendorStr;
        if(vendor.isValid() == false)
        {
            if( model.value.startsWith("ST", Qt::CaseInsensitive))
            {
                vendor.value = "Seagate";
                vendor.valueTranslate = true;
            }
            else if(model.value.startsWith("SanDisk", Qt::CaseInsensitive))
            {
                vendor.value = "SanDisk";
                vendor.valueTranslate = true;
            }
        }

        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct mediaType("Media Type");
        if( DeviceInfoParserInstance.isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParserInstance.queryData("smartctl", logicalName, "Rotation Rate");
            if(rotationRate == DApplication::translate("Main", "Unknown") ||\
               rotationRate.contains("SSD", Qt::CaseInsensitive) ||\
               rotationRate.contains("Solid", Qt::CaseInsensitive) )
            {
                mediaTypeStr = "SDD";
            }
            else
            {
                mediaTypeStr = "HDD";
            }
        }

        mediaType.value = mediaTypeStr;
        articles.push_back(mediaType);

        ArticleStruct size(DApplication::translate("Disk", "Size"));
        size.value = sizeStr;
        articles.push_back(size);
        existArticles.insert("size");

        ArticleStruct interface("Interface");
        QStringList lst = disk.split("_");
        interface.value = DApplication::translate("Main", "Unknown");
        if( lst.size() > 2 )
        {
            interface.value = lst.at(lst.size() -2);
        }

//        if(interface.value.compare("usb", Qt::CaseInsensitive) == 0)
//        {
//            model.value += " USB disk";
//        }

        ArticleStruct serial("Serial");
        serial.queryData("lshw", disk, "serial");
        existArticles.insert("serial");
        existArticles.insert("Serial Number");

        if( DeviceInfoParserInstance.isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParserInstance.queryData("smartctl", logicalName, "Rotation Rate");
            if(rotationRate == DApplication::translate("Main", "Unknown") ||\
               rotationRate.contains("SSD", Qt::CaseInsensitive) ||\
               rotationRate.contains("Solid", Qt::CaseInsensitive))
            {
                mediaTypeStr = "SDD";
            }
            else
            {
                mediaTypeStr = "HDD";
            }

            QString sataVersion = DeviceInfoParserInstance.queryData("smartctl", logicalName, "SATA Version");
            QString version;
            QString speed;

            ArticleStruct mf("Model Family");
            mf.queryData("smartctl", logicalName, "Model Family");
            articles.push_back(mf);
            existArticles.insert("Model Family");

            ArticleStruct dm("Device Model");
            dm.queryData("smartctl", logicalName, "Device Model");
            articles.push_back(dm);
            existArticles.insert("Device Model");

            int index = sataVersion.indexOf(",");
            if(index>0)
            {
                ArticleStruct speed("Speed");
                speed.value = sataVersion.mid( index + 1 );
                speed.value = speed.value.trimmed();
                articles.push_back(speed);

                interface.value = sataVersion.mid(0, index);
                interface.value = interface.value.trimmed();
            }


            articles.push_back(interface);
            existArticles.insert("SATA Version");

            ArticleStruct rr("Rotation Rate");
            rr.value = rotationRate;
            articles.push_back(rr);
            existArticles.insert("Rotation Rate");

            articles.push_back(serial);

            ArticleStruct fromFactor("Form Factor");
            fromFactor.queryData("smartctl", logicalName, "Form Factor");
            fromFactor.value.replace("inches", DApplication::translate("Main", "inch"));
            articles.push_back(fromFactor);
            existArticles.insert("Form Factor");

            ArticleStruct powerOnHours("Power On Hours");
            powerOnHours.queryData("smartctl", logicalName, "Power_On_Hours");
            if(powerOnHours.isValid())
            {
                powerOnHours.value += DApplication::translate("Main", " Hours");
            }
            articles.push_back(powerOnHours);
            existArticles.insert("Power_On_Hours");

            ArticleStruct powerCycleCount("Power Cycle Count");
            powerCycleCount.queryData("smartctl", logicalName, "Power_Cycle_Count");
            if(powerCycleCount.isValid())
            {
                powerCycleCount.value += DApplication::translate("Main", " Times");
            }
            articles.push_back(powerCycleCount);
            existArticles.insert("Power_Cycle_Count");
        }

        ArticleStruct version("Version");
        version.queryData("lshw", disk, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capabilities("Capabilities");
        capabilities.queryData("lshw", disk, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct description("Description");
        description.queryData("lshw", disk, "description");
        articles.push_back(description);
        existArticles.insert("description");

        DeviceInfoParserInstance.queryRemainderDeviceInfo("lshw", disk, articles, existArticles);

        if( DeviceInfoParserInstance.isToolSuccess("smartctl") )
        {
            DeviceInfoParserInstance.queryRemainderDeviceInfo("smartctl", logicalName, articles, existArticles);
        }

        addDevice( model.value, articles, diskList.size() );

        QStringList tab =
        {
            modelStr,
            vendor.value,
            mediaTypeStr,
            sizeStr
        };

        tabList.push_back(tab);

        if( i == 0)
        {
            if(overviewInfo_.isValid())
            {
                overviewInfo_.value += " / ";
            }

            model.value.remove(vendor.value, Qt::CaseInsensitive);

            QList<ArticleStruct> asList;
            asList << vendor << model;

            overviewInfo_.value += joinArticle( asList );
            overviewInfo_.value += " (";
            QString diskSize = size.value;
            QRegExp reg("^[\\s\\S]*\\(([\\s\\S]+)\\)$");
            if(reg.exactMatch(diskSize))
            {
                overviewInfo_.value += reg.cap(1);
            }
            else
            {
                overviewInfo_.value += diskSize;
            }

            overviewInfo_.value += ")";
        }
    }

    if( diskList.size() > 1 )
    {
        //QStringList emptyList;
        //add(DApplication::translate("Main", "Disk")  + DApplication::translate("Main", " Info"), emptyList, emptyList);
        QStringList headers = { "Model",  "Vendor", "Media Type", DApplication::translate("Disk", "Size") };
        addTable(headers, tabList);
    }
}

