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
//#include <QDebug>

DWIDGET_USE_NAMESPACE

DiskWidget::DiskWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Storage"))
{
//    initWidget();
    getHwinfoDiskList();
}

QString DiskWidget::setMediaType(QString &disk, QStringList &diskLogicName)
{
    QString mediaType;

    QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
    foreach (auto LogicName, diskLogicName) {
        if (devicefileStr.contains(LogicName, Qt::CaseInsensitive)) {
            mediaType = DeviceInfoParser::Instance().queryData("MediaType", LogicName, "type");
        }
    }

    if (mediaType == "0") {
        mediaType = "SSD";
    } else if (mediaType == "1") {
        mediaType = "HDD";
    } else {
        mediaType = tr("Unknown");
    }

    ArticleStruct media(tr("Media Type"));
    media.value = mediaType;
    m_articles.push_back(media);
    m_existArticles.insert("Media Type");

    return mediaType;
}

ArticleStruct DiskWidget::setDiskSizeInfo(QString &disk)
{
    QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
    int index = devicefileStr.indexOf("(");
    devicefileStr = devicefileStr.mid(0, index).trimmed();

//    QString driverStr = DeviceInfoParser::Instance().queryData("Storage",disk, "Driver");

    ArticleStruct size(tr("Size"));
    if (DeviceInfoParser::Instance().isSmartctlSupport(devicefileStr) == true)  {
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(size.value, devicefileStr, "capacity");

        QRegExp reg("\\[[\\s\\S]+\\]");

        // 获取 [] 中的实际大小
        if ( reg.indexIn(size.value) > -1) {
            size.value = reg.cap(0);

            reg.setPattern("[0-9]+[\\s\\S]*[^\\]*]");
            if ( reg.indexIn(size.value) > -1) {
                size.value = reg.cap(0);
            }
        }

        if (size.isValid() == false) {
            size.queryData("Storage", disk, "Capacity");
            int index = size.value.indexOf("(");
            size.value = size.value.mid(0, index).trimmed();
        }

    } else {
        size.queryData("Storage", disk, "Capacity");
        int index = size.value.indexOf("(");
        size.value = size.value.mid(0, index).trimmed();
    }

    m_articles.push_back(size);
    m_existArticles.insert("Capacity");

    return size;
}

ArticleStruct DiskWidget::setDiskModelInfo(QString &disk)
{
    ArticleStruct model(tr("Model"));

    QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
//    QString driverStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Driver");
    int index = devicefileStr.indexOf("(");
    devicefileStr = devicefileStr.mid(0, index).trimmed();

    if (DeviceInfoParser::Instance().isSmartctlSupport(devicefileStr) == true) {
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(model.value, devicefileStr, "model");

        // smartctl 中无法获取U盘model信息时
        if (model.isValid() == false) {
            model.queryData("Storage", disk, "Model");
            model.value = model.value.replace("\"", "").trimmed();
        }
    } else {
        model.queryData("Storage", disk, "Model");
        model.value = model.value.replace("\"", "").trimmed();
    }

    m_articles.push_back(model);
    m_existArticles.insert("Model");

    return model;
}

void DiskWidget::addOtherSmartctlInfo(QString &disk)
{
    QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
//    QString driverStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Driver");
    int index = devicefileStr.indexOf("(");
    devicefileStr = devicefileStr.mid(0, index).trimmed();

    if (DeviceInfoParser::Instance().isSmartctlSupport(devicefileStr) == true) {

        // 固件版本
        ArticleStruct firmware(tr("Firmware Version"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(firmware.value, devicefileStr, "Firmware Version");
        m_articles.push_back(firmware);
        m_existArticles.insert("Firmware Version");


        QString sataVersion;
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(sataVersion, devicefileStr, "SATA Version");

        int index = sataVersion.indexOf(",");
        if (index > 0) {
            ArticleStruct speed(tr("Speed", "stroage info"));
            speed.value = sataVersion.mid( index + 1 );
            speed.value = speed.value.trimmed();
            m_articles.push_back(speed);
        }

        ArticleStruct rr(tr("Rotation Rate", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(rr.value, devicefileStr, "Rotation Rate");
        m_articles.push_back(rr);
        m_existArticles.insert("Rotation Rate");

        ArticleStruct powerOnHours(tr("Power On Hours", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerOnHours.value, devicefileStr, "Power_On_Hours");
        if (powerOnHours.isValid()) {
            powerOnHours.value += (" " + tr("Hours"));
        } else {
            DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerOnHours.value, devicefileStr, "Power On Hours");
            if (powerOnHours.isValid()) {
                powerOnHours.value += (" " + tr("Hours"));
            }
        }

        m_articles.push_back(powerOnHours);
        m_existArticles.insert("Power_On_Hours");

        ArticleStruct powerOnMinutes(tr("Power On Minutes", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerOnMinutes.value, devicefileStr, "Power_On_Minutes");
        if (powerOnMinutes.isValid()) {
            powerOnMinutes.value += (" " + tr("Minutes", "Power_On_Minutes"));
        }
        m_articles.push_back(powerOnMinutes);
        m_existArticles.insert("Power_On_Minutes");


        ArticleStruct powerOnHalfMinutes(tr("Power On Half Minutes", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerOnHalfMinutes.value, devicefileStr, "Power_On_Half_Minutes");
        if (powerOnHalfMinutes.isValid()) {
            powerOnHalfMinutes.value += " ";
            powerOnHalfMinutes.value += tr("Half Minutes");
        }
        m_articles.push_back(powerOnHalfMinutes);
        m_existArticles.insert("Power_On_Half_Minutes");

        ArticleStruct powerOnSeconds(tr("Power On Seconds", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerOnSeconds.value, devicefileStr, "Power_On_Seconds");
        if (powerOnSeconds.isValid()) {
            powerOnSeconds.value += " ";
            powerOnSeconds.value += tr("Seconds");
        } else {

        }
        m_articles.push_back(powerOnSeconds);
        m_existArticles.insert("Power_On_Seconds");

        ArticleStruct powerCycleCount(tr("Power Cycle Count", "stroage info"));
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerCycleCount.value, devicefileStr, "Power_Cycle_Count");
        if (powerCycleCount.isValid()) {
            powerCycleCount.value += " ";
            powerCycleCount.value += tr("Times", "Power Cycle Count");
        } else {
            DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(powerCycleCount.value, devicefileStr, "Power Cycles");
            if (powerCycleCount.isValid()) {
                powerCycleCount.value += " ";
                powerCycleCount.value += tr("Times", "Power Cycle Count");
            }

        }
        m_articles.push_back(powerCycleCount);
        m_existArticles.insert("Power_Cycle_Count");
    }
}

ArticleStruct DiskWidget::setDiskInterfaceInfo(QString &disk)
{
    ArticleStruct interface(tr("Interface"));
    interface.queryData("Storage", disk, "Driver");

    if (interface.value.contains("usb", Qt::CaseInsensitive) == true) {
        interface.value = "USB";
    } else if (interface.value.contains("nvme", Qt::CaseInsensitive) == true) {
        QStringList strList = interface.value.split(",");
        if (strList.size() >= 1)
            interface.value = strList[0].toUpper().trimmed();
        interface.value.replace("\"", "");
    } else {
        QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
        DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(interface.value, devicefileStr, "SATA Version");
        int index = interface.value.indexOf(",");
        interface.value = interface.value.mid(0, index);
        interface.value.replace("\"", "");
    }

    m_articles.push_back(interface);
    m_existArticles.insert("Driver");

    return interface;
}

ArticleStruct DiskWidget::setDiskSerialInfo(QString &disk)
{
    // 序列号
    ArticleStruct serial(tr("Serial Number"));

//    QString devicefileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device File");
//    int index = devicefileStr.indexOf("(");
//    devicefileStr = devicefileStr.mid(0, index).trimmed();

//    DeviceInfoParser::Instance().getDiskModelFromSmartctlInfo(serial.value, devicefileStr, "serial");

//    if (serial.isValid() == false) {
//        serial.value = DeviceInfoParser::Instance().queryData("Storage", disk, "Serial ID");
//        serial.value.replace("\"", "");
//    }

    QString deviceFileStr = DeviceInfoParser::Instance().queryData("Storage", disk, "Device Files");
    QStringList itemList = deviceFileStr.split(",");

    foreach (auto item, itemList) {
        if (item.contains("by-id", Qt::CaseInsensitive)) {
            int index;/* = item.indexOf(QRegExp("_[\\S\\d]*[\\s\\S]*$"));
            item = item.mid(index);*/
            index = item.lastIndexOf("_");
            item = item.mid(index + 1);
            item.replace(QRegExp("-[\\s\\S]*$"), "");
            serial.value = item;
            break;
        }
    }

    m_articles.push_back(serial);
    m_existArticles.insert("Serial Number");

    return serial;
}

void DiskWidget::getHwinfoDiskList()
{
    QStringList diskList = DeviceInfoParser::Instance().getHwinfoDiskList();
    QStringList diskType = DeviceInfoParser::Instance().getLsblkDiskTypeList();

    if (diskList.size() < 1) {
        setCentralInfo(tr("No disk found"));
        return;
    }

    foreach (auto disk, diskList) {
        m_articles.clear();
        m_existArticles.clear();


        ArticleStruct model = setDiskModelInfo(disk);

        ArticleStruct vendor(tr("Vendor"));
        vendor.queryData("Storage", disk, "Vendor");

        // vendor 信息只显示 "" 里面的内容
        int startIndex = vendor.value.indexOf("\"");
        vendor.value = vendor.value.mid(startIndex);
        vendor.value = vendor.value.replace("\"", "");
        m_articles.push_back(vendor);
        m_existArticles.insert("Vendor");

        // 设置介质类型
        QString mediaType = setMediaType(disk, diskType);

        // 设置硬盘大小
        ArticleStruct size = setDiskSizeInfo(disk);

        // 设置硬盘接口
        setDiskInterfaceInfo(disk);

        // 设置硬盘序列号
        setDiskSerialInfo(disk);

        // 设置硬盘序smartctl其他信息
        addOtherSmartctlInfo(disk);

        // 描述
        ArticleStruct description = addArticleStruct(tr("Description"), "Storage", disk, "Device File");

        QString title = model.isValid() ? model.value : description.value;
        //m_articlesmap.insert(title, m_articles);
        addDevice( model.value, m_articles, diskList.size());
        QStringList tab = {
            title,
            vendor.value,
            mediaType,
            size.value
        };

        m_tabList.push_back(tab);

        if (overviewInfo_.isValid()) {
            overviewInfo_.value += " / ";
        }

        QList<ArticleStruct> overArticle;
        overArticle << vendor << model;
        if (model.isValid() == false) {
            overArticle << description;
        }
        overviewInfo_.value += joinArticle(overArticle);

    }

    if ( diskList.size() > 1 ) {
        QStringList headers = { tr("Model"),  tr("Vendor"), tr("Media Type"), tr("Size") };
        addTable(headers, m_tabList);
    }
}

void DiskWidget::initWidget()
{
    QStringList diskList = DeviceInfoParser::Instance().getLshwDiskNameList();

    int expect_nvme_index = 1;
    QStringList lsblkList = DeviceInfoParser::Instance().getLsblkDiskNameList();
    if (diskList.size() < 1) {
        setCentralInfo(tr("No disk found"));
        return;
//        getHwinfoDiskList();
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    int i = 0;
    foreach (const QString &disk, diskList) {
        bool isNvmeDisk = false;
        QString logicalName;
        if (disk.contains("storage", Qt::CaseInsensitive) == false) {
            logicalName = DeviceInfoParser::Instance().queryData("lshw", disk, "logical name");
        } else {
            int searched_nvme_index = 0;
            foreach (QString diskName, lsblkList) {
                isNvmeDisk = diskName.contains("nvme", Qt::CaseInsensitive);
                if (isNvmeDisk) {
                    searched_nvme_index ++;
                    if (searched_nvme_index == expect_nvme_index) {
                        //if ture, the diskName is corresponding lsblk key ,also smartctl db key
                        expect_nvme_index ++;
                        logicalName = QString("/dev/%1").arg(diskName);
                        break;
                    }
                }
            }
        }

        QString modelStr = DeviceInfoParser::Instance().queryData("lshw", disk, "product");
        QString vendorStr = DeviceInfoParser::Instance().queryData("lshw", disk, "vendor");
        QString mediaTypeStr = "Unknown";
        QString sizeStr = DeviceInfoParser::Instance().queryData("lshw", disk, "size");

        int brackets = sizeStr.indexOf('(');
        QString sizeStr1 = sizeStr.mid(0, brackets);
        if (sizeStr1.contains("GiB")) {
            sizeStr1.replace("GiB", "GB");
        }
        if (sizeStr1.contains("MiB")) {
            sizeStr1.replace( "MiB", " MB" );
        }

        articles.clear();
        existArticles.clear();

        ArticleStruct model(tr("Model", "stroage info"));
        model.value = modelStr;
        articles.push_back(model);
        existArticles.insert("product");

        ArticleStruct vendor(tr("Vendor", "stroage info"));
        vendor.value = vendorStr;
        if (vendor.isValid() == false) {
            if ( model.value.startsWith("ST", Qt::CaseInsensitive)) {
                vendor.value = "Seagate";
                vendor.valueTranslate = true;
            } else if (model.value.startsWith("SanDisk", Qt::CaseInsensitive)) {
                vendor.value = "SanDisk";
                vendor.valueTranslate = true;
            }
        }

        articles.push_back(vendor);
        existArticles.insert("vendor");

        ArticleStruct mediaType(tr("Media Type", "stroage info"));
        //firstly,try to get media type from smartctl
        if ( DeviceInfoParser::Instance().isToolSuccess("smartctl") ) {
            QString rotationRate = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Rotation Rate");
            QString modelFamilyStr = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Model Family");
            QString deviceModelStr = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Device Model");
            QString modelStr = DeviceInfoParser::Instance().queryData("lshw", disk, "product");

            if ( getDiskType(rotationRate, mediaTypeStr) == false ) {
                if ( getDiskType(modelFamilyStr, mediaTypeStr) == false ) {
                    if (getDiskType(deviceModelStr, mediaTypeStr) == false) {
                        getDiskType(modelStr, mediaTypeStr);
                    }
                }
            }
        }
        //secondly, try to get media type from lshw
        if (mediaTypeStr == "Unknown") {
            if (disk.contains("usb", Qt::CaseInsensitive)) {
                mediaTypeStr = " USB disk";
            } else {
                QString curKey = disk;
                QString parentKey = curKey.mid(0, curKey.lastIndexOf("_"));
                auto &parent_dev = DeviceInfoParser::Instance().toolDatabase_.value("lshw").value(parentKey);
                if (parentKey.endsWith("core") == false) {
                    if (parent_dev.value("configuration").contains("usb")) {
                        mediaTypeStr = " USB disk";
                    }
                    if (parent_dev.value("bus info").contains("usb")) {
                        mediaTypeStr = " USB disk";
                    }
                }
            }
        }
        //thirdly, try to get media type if media type is unknown, and try to get nvme disk size
        if (isNvmeDisk) {
            auto  &db = DeviceInfoParser::Instance().toolDatabase_;
            QString logicalName_ = logicalName;
            if (mediaTypeStr == "Unknown") {
                if (db.value("smartctl").value(logicalName).value("Rotation Rate").contains("rpm", Qt::CaseInsensitive)) {
                    mediaTypeStr = "HDD";
                }
                if (db.value("smartctl").value(logicalName).value("Model Family").contains("HHD", Qt::CaseInsensitive)) {
                    mediaTypeStr = "HHD";
                }
                if (db.value("smartctl").value(logicalName).value("Model Family").contains("HDD", Qt::CaseInsensitive)) {
                    mediaTypeStr = "HDD";
                }
            }
            //get nvme disk used size
            sizeStr1 = db.value("lsblk").value(logicalName_.remove("/dev/")).value("size");
            sizeStr1.replace(QRegExp("(G|g)(B|b){0,1}"), "GB");
            sizeStr1.replace(QRegExp("(M|m)(B|b){0,1}"), "MB");

            if (sizeStr == tr("Unknown")) {
                sizeStr = sizeStr1;
            }
            //try to get nvme disk total size
            QString totalNVMSize = db.value("smartctl").value(logicalName).value("Total NVM Capacity");
            if (totalNVMSize.isEmpty() == false) {
//                QRegExp rx("^(.*)\\[(.*GB)\\]$");
                QRegExp rx("^(.*)\\[(.*GB)\\]");
                QStringList capTexts ;
                if (rx.exactMatch(totalNVMSize)) {
                    capTexts = rx.capturedTexts();
                    if (capTexts.isEmpty() == false) {
                        sizeStr1 += QString(" (%1)").arg(capTexts.last());
                    }
                }
            }
        }
        mediaType.value = mediaTypeStr;
        articles.push_back(mediaType);


        ArticleStruct interface(tr("Interface", "stroage info"));
        QStringList lst = disk.split("_");
        interface.value = tr("Unknown");
        if ( lst.size() > 2 ) {
            interface.value = lst.at(lst.size() - 2);
        }

        ArticleStruct size(tr("Size", "stroage info"));
        size.value = sizeStr1;
        articles.push_back(size);
        existArticles.insert("size");

        ArticleStruct serial(tr("Serial Number", "stroage info"));
        serial.queryData("lshw", disk, "serial");
        existArticles.insert("serial");
        existArticles.insert("Serial Number");

        if ( DeviceInfoParser::Instance().isToolSuccess("smartctl") ) {
            QString rotationRate = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "Rotation Rate");

            QString sataVersion = DeviceInfoParser::Instance().queryData("smartctl", logicalName, "SATA Version");
            QString version;
            QString speed;

            ArticleStruct mf(tr("Model Family", "stroage info"));
            mf.queryData("smartctl", logicalName, "Model Family");
            articles.push_back(mf);
            existArticles.insert("Model Family");

            ArticleStruct dm(tr("Device Model", "stroage info"));
            dm.queryData("smartctl", logicalName, "Device Model");
            articles.push_back(dm);
            existArticles.insert("Device Model");

            ArticleStruct description(tr("Description", "stroage info"));
            description.queryData("lshw", disk, "description");
            articles.push_back(description);
            existArticles.insert("description");

            int index = sataVersion.indexOf(",");
            if (index > 0) {
                ArticleStruct speed(tr("Speed", "stroage info"));
                speed.value = sataVersion.mid( index + 1 );
                speed.value = speed.value.trimmed();
                articles.push_back(speed);

                interface.value = sataVersion.mid(0, index);
                interface.value = interface.value.trimmed();
            }
            articles.push_back(interface);
            existArticles.insert("SATA Version");

            ArticleStruct rr(tr("Rotation Rate", "stroage info"));
            rr.value = rotationRate;
            articles.push_back(rr);
            existArticles.insert("Rotation Rate");

            articles.push_back(serial);

            ArticleStruct fromFactor(tr("Form Factor", "stroage info"));
            fromFactor.queryData("smartctl", logicalName, "Form Factor");
            fromFactor.value.replace("inches", tr("inch"));
            articles.push_back(fromFactor);
            existArticles.insert("Form Factor");

            ArticleStruct powerOnHours(tr("Power On Hours", "stroage info"));
            powerOnHours.queryData("smartctl", logicalName, "Power_On_Hours", existArticles);
            if (powerOnHours.isValid()) {
                powerOnHours.value += (" " + tr("Hours"));
            }
            articles.push_back(powerOnHours);

            ArticleStruct powerOnMinutes(tr("Power On Minutes", "stroage info"));
            powerOnMinutes.queryData("smartctl", logicalName, "Power_On_Minutes", existArticles);
            if (powerOnMinutes.isValid()) {
                powerOnMinutes.value += (" " + tr("Minutes", "Power_On_Minutes"));
            }
            articles.push_back(powerOnMinutes);


            ArticleStruct powerOnHalfMinutes(tr("Power On Half Minutes", "stroage info"));
            powerOnHalfMinutes.queryData("smartctl", logicalName, "Power_On_Half_Minutes", existArticles);
            if (powerOnHalfMinutes.isValid()) {
                powerOnHalfMinutes.value += " ";
                powerOnHalfMinutes.value += tr("Half Minutes");
            }
            articles.push_back(powerOnHalfMinutes);


            ArticleStruct powerOnSeconds(tr("Power On Seconds", "stroage info"));
            powerOnSeconds.queryData("smartctl", logicalName, "Power_On_Seconds", existArticles);
            if (powerOnSeconds.isValid()) {
                powerOnSeconds.value += " ";
                powerOnSeconds.value += tr("Seconds");
            }
            articles.push_back(powerOnSeconds);

            ArticleStruct powerCycleCount(tr("Power Cycle Count", "stroage info"));
            powerCycleCount.queryData("smartctl", logicalName, "Power_Cycle_Count");
            if (powerCycleCount.isValid()) {
                powerCycleCount.value += " ";
                powerCycleCount.value += tr("Times", "Power Cycle Count");
            }
            articles.push_back(powerCycleCount);
            existArticles.insert("Power_Cycle_Count");
        }

        ArticleStruct version(tr("Version", "stroage info"));
        version.queryData("lshw", disk, "version");
        articles.push_back(version);
        existArticles.insert("version");

        ArticleStruct capabilities(tr("Capabilities", "stroage info"));
        capabilities.queryData("lshw", disk, "capabilities");
        articles.push_back(capabilities);
        existArticles.insert("capabilities");

        ArticleStruct type(tr("Type", "stroage info"));
        type.queryData("lshw", disk, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct ansiversion(tr("ANSI Version", "stroage info"));
        ansiversion.queryData("lshw", disk, "ansiversion");
        articles.push_back(ansiversion);
        existArticles.insert("ansiversion");

        ArticleStruct guid(tr("GUID", "stroage info"));
        guid.queryData("lshw", disk, "guid");
        articles.push_back(guid);
        existArticles.insert("guid");

        ArticleStruct busInfo(tr("Bus Info", "stroage info"));
        busInfo.queryData("lshw", disk, "bus info");
        articles.push_back(busInfo);
        existArticles.insert("bus info");

        ArticleStruct logicalName1(tr("Logical Name", "stroage info"));
        logicalName1.queryData("lshw", disk, "logical name");
        articles.push_back(logicalName1);
        existArticles.insert("logical name");

        ArticleStruct logicalsectorsize(tr("Logical Sector Size", "stroage info"));
        logicalsectorsize.queryData("lshw", disk, "logicalsectorsize");
        articles.push_back(logicalsectorsize);
        existArticles.insert("logicalsectorsize");

        ArticleStruct physicalId(tr("Capabilities", "physical id"));
        physicalId.queryData("lshw", disk, "physical id");
        articles.push_back(physicalId);
        existArticles.insert("physical id");

        ArticleStruct sectorsize(tr("Sector Size", "stroage info"));
        sectorsize.queryData("lshw", disk, "sectorsize");
        articles.push_back(sectorsize);
        existArticles.insert("sectorsize");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", disk, articles, existArticles, "ManulTrack__Disk", "Disk Information");

        if ( DeviceInfoParser::Instance().isToolSuccess("smartctl") ) {

            ArticleStruct ataVersion(tr("ATA Version", "stroage info"));
            ataVersion.queryData("smartctl", logicalName, "ATA Version");
            articles.push_back(ataVersion);
            existArticles.insert("ATA Version");

            ArticleStruct autoOfflineDataCollection(tr("Auto Offline Data Collection", "stroage info"));
            autoOfflineDataCollection.queryData("smartctl", logicalName, "Auto Offline Data Collection");
            articles.push_back(autoOfflineDataCollection);
            existArticles.insert("Auto Offline Data Collection");

            ArticleStruct currentPendingSector(tr("Current Pending Sector", "stroage info"));
            currentPendingSector.queryData("smartctl", logicalName, "Current_Pending_Sector");
            articles.push_back(currentPendingSector);
            existArticles.insert("Current_Pending_Sector");

            ArticleStruct device(tr("Device", "stroage info"));
            device.queryData("smartctl", logicalName, "Device");
            articles.push_back(device);
            existArticles.insert("Device");

            ArticleStruct endToEndError(tr("End-to-End Error", "stroage info"));
            endToEndError.queryData("smartctl", logicalName, "End-to-End_Error");
            articles.push_back(endToEndError);
            existArticles.insert("End-to-End_Error");

            ArticleStruct firmwareVersion(tr("Firmware Version", "stroage info"));
            firmwareVersion.queryData("smartctl", logicalName, "Firmware Version");
            articles.push_back(firmwareVersion);
            existArticles.insert("Firmware Version");

            ArticleStruct smartCapabilities(tr("SMART Capabilities", "stroage info"));
            smartCapabilities.queryData("smartctl", logicalName, "SMART capabilities");
            articles.push_back(smartCapabilities);
            existArticles.insert("SMART capabilities");

//            ArticleStruct (tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");

//            ArticleStruct sectorsize(tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");
//            ArticleStruct sectorsize(tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");
//            ArticleStruct sectorsize(tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");
//            ArticleStruct sectorsize(tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");
//            ArticleStruct sectorsize(tr("sectorsize", "stroage info"));
//            sectorsize.queryData("lshw", logicalName, "sectorsize");
//            articles.push_back(sectorsize);
//            existArticles.insert("sectorsize");
            DeviceInfoParser::Instance().queryRemainderDeviceInfo("smartctl", logicalName, articles, existArticles, "ManulTrack__Disk", "Disk Information");
        }

        addDevice( model.value, articles, diskList.size() );

        tabList.push_back({ modelStr, vendor.value, mediaType.value, sizeStr });

        if ( i == 0) {
            if (overviewInfo_.isValid()) {
                overviewInfo_.value += " / ";
            }

            model.value.remove(vendor.value, Qt::CaseInsensitive);

            QList<ArticleStruct> asList;
            asList << vendor << model;

            overviewInfo_.value += joinArticle( asList );
            overviewInfo_.value += " (";
            QString diskSize = size.value;
            QRegExp reg("^[\\s\\S]*\\(([\\s\\S]+)\\)$");
            if (reg.exactMatch(diskSize)) {
                overviewInfo_.value += reg.cap(1);
            } else {
                overviewInfo_.value += diskSize;
            }

            overviewInfo_.value += ")";
        }
    }

    if ( diskList.size() > 1 ) {
        QStringList headers = { tr("Model"),  tr("Vendor"), tr("Media Type"), tr("Size") };
        addTable(headers, tabList);
    }
}

bool DiskWidget::getDiskType(const QString &diskProperty, QString &type)
{
    if ( diskProperty.contains("SSD", Qt::CaseInsensitive) || diskProperty.contains("Solid", Qt::CaseInsensitive) ) {
        type = "SSD";
        return true;
    }

    if (diskProperty.contains("HDD", Qt::CaseInsensitive) || diskProperty.contains("7200", Qt::CaseInsensitive) \
            || diskProperty.contains("5400", Qt::CaseInsensitive) || diskProperty.contains("rpm", Qt::CaseInsensitive)) {
        type = "HDD";
        return true;
    }

    if ( diskProperty.contains("Western Digital", Qt::CaseInsensitive) ) {
        if ( diskProperty.contains("Black", Qt::CaseInsensitive) ||
                diskProperty.contains("Green", Qt::CaseInsensitive) ||
                diskProperty.contains("Blue", Qt::CaseInsensitive) ||
                diskProperty.contains("Red", Qt::CaseInsensitive) ||
                diskProperty.contains("Scorpio", Qt::CaseInsensitive) )
            type = "HDD";
        return true;
    }

    return false;
}
