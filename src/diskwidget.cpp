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

#include "diskwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Storage"))
{
    initWidget();
}

void DiskWidget::initWidget()
{
    if (getDiskInfo()) {
        return;
    }
    return NoDiskFound();
}


void DiskWidget::NoDiskFound()
{
    setCentralInfo(tr("No disk found"));
    return;
}

bool DiskWidget::getDiskInfo()
{
    QStringList disksFromLshw = DeviceInfoParser::Instance().getLshwDiskNameList();
    QList<QString> blockFileList = getAllDiskFromLsblk();
    int diskCount = blockFileList.count();

    if(diskCount < 1)
    {
        return false;
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    foreach(const QString& disk, disksFromLshw)
    {
        const QString logicalName = DeviceInfoParser::Instance().queryData("lshw", disk, "logical name");
        QString logicalName_ = logicalName;
        blockFileList.removeAll(logicalName_.remove("/dev/"));

        QString modelStr = DeviceInfoParser::Instance().queryData("lshw", disk, "product");
        QString vendorStr = DeviceInfoParser::Instance().queryData("lshw", disk, "vendor");
        QString mediaTypeStr = "Unknown";
        QString sizeStr = DeviceInfoParser::Instance().queryData("lshw", disk, "size");
        sizeStr.replace("GiB","GB");

        articles.clear();
        existArticles.clear();

        ArticleStruct model(tr("Model","stroage info"));
        model.value = modelStr;
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor(tr("Vendor","stroage info"));
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

        ArticleStruct mediaType(tr("Media Type","stroage info"));
        if( DeviceInfoParser::Instance().isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Rotation Rate");
            QString modelFamilyStr = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Model Family");
            QString deviceModelStr = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Device Model");
            QString modelStr = DeviceInfoParser::Instance().queryData("lshw", disk, "product");

            if( getDiskType(rotationRate, mediaTypeStr) == false )
            {
                if( getDiskType(modelFamilyStr, mediaTypeStr) == false )
                {
                    if(getDiskType(deviceModelStr, mediaTypeStr) == false){
                        getDiskType(modelStr, mediaTypeStr);
                    }
                }
            }
        }

        ArticleStruct interface(tr("Interface","stroage info"));
        QStringList lst = disk.split("_");
        interface.value = tr("Unknown");
        if( lst.size() > 2 )
        {
            interface.value = lst.at(lst.size() -2);
        }

        if(mediaTypeStr == "Unknown" && interface.value.compare("usb", Qt::CaseInsensitive) == 0)
        {
            mediaTypeStr = " USB disk";
        }
        mediaType.value = mediaTypeStr;
        articles.push_back(mediaType);

        ArticleStruct size(tr("Size","stroage info"));
        size.value = sizeStr;
        articles.push_back(size);
        existArticles.insert("size");

        ArticleStruct serial(tr("Serial Number","stroage info"));
        serial.queryData("lshw", disk, "serial");
        existArticles.insert("serial");
        existArticles.insert("Serial Number");

        if( DeviceInfoParser::Instance().isToolSuccess("smartctl") )
        {
            QString rotationRate = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Rotation Rate");

            QString sataVersion = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "SATA Version");
            QString version;
            QString speed;

            ArticleStruct mf(tr("Model Family","stroage info"));
            mf.queryData("smartctl", logicalName, "Model Family");
            articles.push_back(mf);
            existArticles.insert("Model Family");

            ArticleStruct dm(tr("Device Model","stroage info"));
            dm.queryData("smartctl", logicalName, "Device Model");
            articles.push_back(dm);
            existArticles.insert("Device Model");

            ArticleStruct description(tr("Description","stroage info"));
            description.queryData("lshw", disk, "description");
            articles.push_back(description);
            existArticles.insert("description");

            int index = sataVersion.indexOf(",");
            if(index>0)
            {
                ArticleStruct speed(tr("Speed","stroage info"));
                speed.value = sataVersion.mid( index + 1 );
                speed.value = speed.value.trimmed();
                articles.push_back(speed);

                interface.value = sataVersion.mid(0, index);
                interface.value = interface.value.trimmed();
            }

            articles.push_back(interface);
            existArticles.insert("SATA Version");

            ArticleStruct rr(tr("Rotation Rate","stroage info"));
            rr.value = rotationRate;
            articles.push_back(rr);
            existArticles.insert("Rotation Rate");

            articles.push_back(serial);

            ArticleStruct fromFactor(tr("Form Factor","stroage info"));
            fromFactor.queryData("smartctl", logicalName, "Form Factor");
            fromFactor.value.replace("inches", tr("inch"));
            articles.push_back(fromFactor);
            existArticles.insert("Form Factor");

            ArticleStruct powerOnHours(tr("Power On Hours","stroage info"));
            powerOnHours.queryData("smartctl", logicalName, "Power_On_Hours", existArticles);
            if(powerOnHours.isValid())
            {
                powerOnHours.value += (" "+ tr("Hours"));
            }
            articles.push_back(powerOnHours);

            ArticleStruct powerOnMinutes(tr("Power On Minutes","stroage info"));
            powerOnMinutes.queryData("smartctl", logicalName, "Power_On_Minutes", existArticles);
            if(powerOnMinutes.isValid())
            {
                powerOnMinutes.value += (" " + tr("Minutes","Power_On_Minutes"));
            }
            articles.push_back(powerOnMinutes);


            ArticleStruct powerOnHalfMinutes(tr("Power On Half Minutes","stroage info"));
            powerOnHalfMinutes.queryData("smartctl", logicalName, "Power_On_Half_Minutes", existArticles);
            if(powerOnHalfMinutes.isValid())
            {
                powerOnHalfMinutes.value += " ";
                powerOnHalfMinutes.value += tr("Half Minutes");
            }
            articles.push_back(powerOnHalfMinutes);


            ArticleStruct powerOnSeconds(tr("Power On Seconds","stroage info"));
            powerOnSeconds.queryData("smartctl", logicalName, "Power_On_Seconds", existArticles);
            if(powerOnSeconds.isValid())
            {
                powerOnSeconds.value += " ";
                powerOnSeconds.value += tr("Seconds");
            }
            articles.push_back(powerOnSeconds);

            ArticleStruct powerCycleCount(tr("Power Cycle Count","stroage info"));
            powerCycleCount.queryData("smartctl", logicalName, "Power_Cycle_Count");
            if(powerCycleCount.isValid())
            {
                powerCycleCount.value += " ";
                powerCycleCount.value += tr("Times","Power Cycle Count");
            }
            articles.push_back(powerCycleCount);
            existArticles.insert("Power_Cycle_Count");
        }

        ArticleStruct version(tr("Version","stroage info"));
        version.queryData("lshw", disk, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capabilities(tr("Capabilities","stroage info"));
        capabilities.queryData("lshw", disk, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", disk, articles, existArticles);

        if( DeviceInfoParser::Instance().isToolSuccess("smartctl") )
        {
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("smartctl", logicalName, articles, existArticles);
        }

        addDevice( model.value, articles, diskCount );

        QStringList tab =
        {
            logicalName_.remove("/dev/"),
            modelStr,
            mediaTypeStr,
            sizeStr
        };

        tabList.push_back(tab);

        //overiew info
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

    }//for circle end

    if (blockFileList.isEmpty() == false && DeviceInfoParser::Instance().toolDatabase_.contains("smartctl")) {
        foreach (auto blockFileName, blockFileList) {
            QString fullPathDeviceName = QString("/dev/%1").arg(blockFileName);
            QList<ArticleStruct> articles;
            articles.clear();
            auto smartctlDb = DeviceInfoParser::Instance().toolDatabase_.value("smartctl");
            if (smartctlDb.contains(fullPathDeviceName) == false) {
                continue;
            }

            QString deviceName = blockFileName;
            QString model;
            QString mediaType;
            QString size = "Unknown";
            if (DeviceInfoParser::Instance().toolDatabase_.contains("lsblk")) {
                size = DeviceInfoParser::Instance().toolDatabase_.value("lsblk").value(deviceName).value("size");
            }
            foreach (auto artHead, smartctlDb.value(fullPathDeviceName).keys()) {

                static const QStringList showItems = {"Model Family",
                                                      "Device Model",
                                                      "SATA Version",
                                                      "Device Model",
                                                      "Form Factor",
                                                      "Firmware Version",
                                                      "Rotation Rate",
                                                      "Power_On_Minutes",
                                                      "Power_On_Half_Minutes",
                                                      "Power_On_Seconds"
                                                      "Power_Cycle_Count"
                                                     };
             static const QStringList tr_showItems = {tr("Model Family"),
                                                      tr("Device Model"),
                                                      tr("SATA Version"),
                                                      tr("Device Model"),
                                                      tr("Form Factor"),
                                                      tr("Firmware Version"),
                                                      tr("Rotation Rate"),
                                                      tr("Power On Minutes"),
                                                      tr("Power On Half_Minutes"),
                                                      tr("Power On Seconds"),
                                                      tr("Power Cycle Count"),
                                                     };
                if (showItems.contains(artHead)) {
                    QString artName = artHead.replace("_"," ",Qt::CaseInsensitive);
                    ArticleStruct art(tr(artName.toStdString().data()));
                    art.value = smartctlDb.value(fullPathDeviceName).value(artHead);
                    if (art.isValid())
                    articles.append(art);
                }

                if (artHead == QLatin1String("Device Model")) {
                    model = smartctlDb.value(fullPathDeviceName).value(artHead);
                }
                if (artHead == QLatin1String("Rotation Rate")) {

                    getDiskType(smartctlDb.value(fullPathDeviceName).value(artHead),mediaType);

                    ArticleStruct type(tr("Media Type","stroage info"));
                    type.value = mediaType;
                    if (type.isValid())
                    articles.append(type);
                }
            }
            if (overviewInfo_.value.isEmpty() == false) {
                overviewInfo_.value += " / ";
            }
            overviewInfo_.value +=  QString("%1(%2)").arg(model).arg(size);
            tabList.append(QStringList({deviceName,model,mediaType,size}));
            addDevice(blockFileName,articles,diskCount);
        }
    }

    if (diskCount > 1 )
    {
        QStringList headers = {tr("Device Name") ,tr("Model"), tr("Media Type"), tr("Size") };
        addTable(headers, tabList);
    }

    return true;
}
//return disk file name without prefix("/dev/")
QList<QString> DiskWidget::getAllDiskFromLsblk()
{
    QList<QString> retList;
    retList.clear();
    auto toolDb = DeviceInfoParser::Instance().toolDatabase_;
    if (toolDb.contains("lsblk") == false) {
        return retList;
    }
    return  toolDb.value("lsblk").keys();
}
bool DiskWidget::getDiskType(const QString& diskProperty, QString& type)
{
    if( diskProperty.contains("SSD", Qt::CaseInsensitive) || diskProperty.contains("Solid", Qt::CaseInsensitive) )
    {
        type = "SSD";
        return true;
    }

    if(diskProperty.contains("HDD", Qt::CaseInsensitive) || diskProperty.contains("7200", Qt::CaseInsensitive) \
            || diskProperty.contains("5400", Qt::CaseInsensitive) || diskProperty.contains("rpm", Qt::CaseInsensitive))
    {
        type = "HDD";
        return true;
    }

    if( diskProperty.contains("Western Digital", Qt::CaseInsensitive) )
    {
        if( diskProperty.contains("Black", Qt::CaseInsensitive) ||
            diskProperty.contains("Green", Qt::CaseInsensitive) ||
            diskProperty.contains("Blue", Qt::CaseInsensitive) ||
            diskProperty.contains("Red", Qt::CaseInsensitive) ||
            diskProperty.contains("Scorpio", Qt::CaseInsensitive) )
        type = "HDD";
        return true;
    }

    return false;
}

