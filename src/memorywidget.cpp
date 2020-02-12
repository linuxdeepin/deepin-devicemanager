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

DWIDGET_USE_NAMESPACE

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Memory"))
{
    initWidget();
}

void MemoryWidget::initWidget()
{
    init_l_Designer_l_TableWdiget();
    update_l_Designer_l_WholeDownWidget();
}

void MemoryWidget::init_l_Designer_l_TableWdiget()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();
    bool getDmidecodeMemoryListSuccess = true;
    if (memList.isEmpty()) {
        getDmidecodeMemoryListSuccess = false;
        memList = DeviceInfoParser::Instance().getDmidecodeMemoryArrayMappedAddress();
    }
    if (getDmidecodeMemoryListSuccess == true) {

        QStringList headers = { "Name", "Vendor", "Type", "Speed",  "Size", /* "Status"*/};

        QList<QStringList> tabList;

        foreach(const QString& mem, memList)
        {
            if(canUpgrade_ == -1)
            {
                canUpgrade_ = 0;
            }

            QString speed = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Speed");
            QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size");
            if( isSlotValid(size, speed) == false )
            {
                canUpgrade_ = 1;
                continue;
            }

            QStringList tab = {
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Part Number"),
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Manufacturer"),
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Type"),
                speed,
                size,

                //DApplication::translate("Main", "Good")
            };

            tabList.push_back(tab);
        }
        addTable(headers, tabList);
    }

    if (getDmidecodeMemoryListSuccess == false) {
        QStringList headers = { "Name", "Vendor", "Type", "Speed",  "Size",};

        QList<QStringList> tabList;

        foreach(const QString& mem, memList)
        {
            if(canUpgrade_ == -1)
            {
                canUpgrade_ = 0;
            }

            QString speed = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Speed");
            QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Range Size");

            QStringList tab = {
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Part Number"),
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Manufacturer"),
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Type"),
                speed,
                size,

                //DApplication::translate("Main", "Good")
            };

            tabList.push_back(tab);
        }
        addTable(headers, tabList);
    }

}

void MemoryWidget::update_l_Designer_l_WholeDownWidget()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();
    bool getDmidecodeMemoryListSuccess = true;
    if (memList.isEmpty()) {
        getDmidecodeMemoryListSuccess = false;
        memList = DeviceInfoParser::Instance().getDmidecodeMemoryArrayMappedAddress();
    }
    int validMemoryNumber = 0;

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
    mc.queryData("dmidecode", "Physical Memory Array", "Maximum Capacity");
    articles.push_back(mc);

    if( mc.isValid() && size.isValid())
    {
        if( (mc.value.contains("Mb") && size.value.contains("Mb")) || (mc.value.contains("GB") && size.value.contains("GB")) )
        {
            if(mc.value.left(mc.value.size() - 2 ).toInt() > size.value.left(mc.value.size() -2 ).toInt())
            {
                canUpgrade_ = 1;
            }
        }
    }

    if(canUpgrade_ != -1)
    {
        ArticleStruct ug("Upgradeable");
        ug.value = canUpgrade_ ? DApplication::translate("Main", "Yes") : DApplication::translate("Main", "No");
        articles.push_back(ug);
    }

    summaryInfo_ = articles;

    articles.clear();
    QSet<QString> existArticles;


    QStringList detailMem;
    foreach(const QString& mem, memList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct model("Model");
        model.queryData("dmidecode", mem, "Part Number");
        articles.push_back(model);
        existArticles.insert("Part Number");

        ArticleStruct vendor("Vendor");
        vendor.queryData("dmidecode", mem, "Manufacturer");
        articles.push_back(vendor);
        existArticles.insert("Manufacturer");

        ArticleStruct locator("Locator");
        locator.queryData("dmidecode", mem, "Locator");
        articles.push_back(locator);
        existArticles.insert("Locator");

        ArticleStruct size("Size");
        size.queryData("dmidecode", mem, "Size");
        articles.push_back(size);
        existArticles.insert("Size");

        ArticleStruct type("Type");
        type.queryData("dmidecode", mem, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct speed("Speed");
        speed.queryData("dmidecode", mem, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct serial("Serial Number");
        serial.queryData("dmidecode", mem, "Serial Number");
        articles.push_back(serial);
        existArticles.insert("Serial Number");

        ArticleStruct formFactor("Form Factor");
        formFactor.queryData("dmidecode", mem, "Form Factor");
        articles.push_back(formFactor);
        existArticles.insert("Form Factor");

        ArticleStruct rank("Rank");
        rank.queryData("dmidecode", mem, "Rank");
        articles.push_back(rank);
        existArticles.insert("Rank");

        ArticleStruct bankLocator("Bank Locator");
        bankLocator.queryData("dmidecode", mem, "Bank Locator");
        articles.push_back(bankLocator);
        existArticles.insert("Bank Locator");

        ArticleStruct configVoltage("Configured Voltage");
        configVoltage.queryData("dmidecode", mem, "Configured Voltage");
        articles.push_back(configVoltage);
        existArticles.insert("Configured Voltage");

        ArticleStruct minVoltage("Minimum Voltage");
        minVoltage.queryData("dmidecode", mem, "Minimum Voltage");
        articles.push_back(minVoltage);
        existArticles.insert("Minimum Voltage");

        ArticleStruct maxVoltage("Maximum Voltage");
        maxVoltage.queryData("dmidecode", mem, "Maximum Voltage");
        articles.push_back(maxVoltage);
        existArticles.insert("Maximum Voltage");

        ArticleStruct configSpeed("Configured Speed");
        configSpeed.queryData("dmidecode", mem, "Configured Memory Speed");
        articles.push_back(configSpeed);
        existArticles.insert("Configured Memory Speed");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);

        QString deviceName = vendor.value + " " + model.value;
        if(vendor.value == DApplication::translate("Main", "Unknown") && model.value == DApplication::translate("Main", "Unknown")){
            deviceName = tr("Unknow memory");
        }
        if(deviceName.trimmed().isEmpty() == true)
        {
            deviceName = locator.value;
            if(deviceName.trimmed().isEmpty() == true)
            {
                deviceName = bankLocator.value;
            }
        }

        if( false == isSlotValid(size.value, speed.value) && getDmidecodeMemoryListSuccess)
        {
            continue;
        }

        ++validMemoryNumber;

        addSubInfo( deviceName, articles );

        QList<ArticleStruct> arLst;
        arLst << vendor << type << speed;
        QString overviewVendor = joinArticle(arLst);

        if( false == detailMem .contains(overviewVendor) )
        {
            detailMem.push_back(overviewVendor);
        }
    }

    if(validMemoryNumber < 1 && getDmidecodeMemoryListSuccess == true)
    {
        setCentralInfo(tr("Failed to get memory information"));
        return;
    }

    overviewInfo_.value = size.value;
    if( detailMem.size() > 0 )
    {
        QString detail = detailMem.join(" / ");
        if(detail.isEmpty() == false) {
            overviewInfo_.value += " (";
            overviewInfo_.value += detail;
            overviewInfo_.value += ")";
        }
    }
}

void MemoryWidget::initTableWdiget_Good()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();

    QStringList headers = { "Bank", "Vendor", "Type", /*"Speed",*/  "Size", /* "Status"*/};

    QList<QStringList> tabList;

    foreach(const QString& mem, memList)
    {
        if(canUpgrade_ == -1)
        {
            canUpgrade_ = 0;
        }

        QString size = DeviceInfoParser::Instance().queryData("dmidecode", mem, "Size");
        if( size == DApplication::translate("Main", "Unknown") || size == "No Module Installed" )
        {
            canUpgrade_ = 1;
            QStringList tab = {
                DeviceInfoParser::Instance().queryData("dmidecode", mem, "Locator"),
                "--",
                "--",
                "--",
                //DApplication::translate("Main", "Bad")
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

            //DApplication::translate("Main", "Good")
        };

        tabList.push_back(tab);
    }
    addTable(headers, tabList);
}

void MemoryWidget::updateWholeDownWidget_Good()
{
    QStringList memList = DeviceInfoParser::Instance().getDmidecodeMemoryList();

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
    mc.queryData("dmidecode", "Physical Memory Array", "Maximum Capacity");
    articles.push_back(mc);

    if( mc.isValid() && size.isValid())
    {
        if( (mc.value.contains("Mb") && size.value.contains("Mb")) || (mc.value.contains("GB") && size.value.contains("GB")) )
        {
            if(mc.value.left(mc.value.size() - 2 ).toInt() > size.value.left(mc.value.size() -2 ).toInt())
            {
                canUpgrade_ = true;
            }
        }
    }

    if(canUpgrade_ != -1)
    {
        ArticleStruct ug("Upgradeable");
        ug.value = canUpgrade_ ? DApplication::translate("Main", "Yes") : DApplication::translate("Main", "No");
        articles.push_back(ug);
    }

    addInfo("Memory Info", articles, false);

    articles.clear();
    QSet<QString> existArticles;


    QStringList detailMem;
    foreach(const QString& mem, memList)
    {
        articles.clear();
        existArticles.clear();

        ArticleStruct locator("Locator");
        locator.queryData("dmidecode", mem, "Locator");
        articles.push_back(locator);
        existArticles.insert("Locator");

        ArticleStruct vendor("Vendor");
        vendor.queryData("dmidecode", mem, "Manufacturer");
        articles.push_back(vendor);
        existArticles.insert("Manufacturer");

        ArticleStruct size("Size");
        size.queryData("dmidecode", mem, "Size");
        articles.push_back(size);
        existArticles.insert("Size");

        ArticleStruct type("Type");
        type.queryData("dmidecode", mem, "Type");
        articles.push_back(type);
        existArticles.insert("Type");

        ArticleStruct speed("Speed");
        speed.queryData("dmidecode", mem, "Speed");
        articles.push_back(speed);
        existArticles.insert("Speed");

        ArticleStruct serial("Serial Number");
        serial.queryData("dmidecode", mem, "Serial Number");
        articles.push_back(serial);
        existArticles.insert("Serial Number");

        ArticleStruct model("Model");
        model.queryData("dmidecode", mem, "Part Number");
        articles.push_back(model);
        existArticles.insert("Part Number");

        ArticleStruct formFactor("Form Factor");
        formFactor.queryData("dmidecode", mem, "Form Factor");
        articles.push_back(formFactor);
        existArticles.insert("Form Factor");

        ArticleStruct rank("Rank");
        rank.queryData("dmidecode", mem, "Rank");
        articles.push_back(rank);
        existArticles.insert("Rank");

        ArticleStruct bankLocator("Bank Locator");
        bankLocator.queryData("dmidecode", mem, "Bank Locator");
        articles.push_back(bankLocator);
        existArticles.insert("Bank Locator");

        ArticleStruct configVoltage("Configured Voltage");
        configVoltage.queryData("dmidecode", mem, "Configured Voltage");
        articles.push_back(configVoltage);
        existArticles.insert("Configured Voltage");

        ArticleStruct minVoltage("Minimum Voltage");
        minVoltage.queryData("dmidecode", mem, "Minimum Voltage");
        articles.push_back(minVoltage);
        existArticles.insert("Minimum Voltage");

        ArticleStruct maxVoltage("Maximum Voltage");
        maxVoltage.queryData("dmidecode", mem, "Maximum Voltage");
        articles.push_back(maxVoltage);
        existArticles.insert("Maximum Voltage");

        ArticleStruct configSpeed("Configured Speed");
        configSpeed.queryData("dmidecode", mem, "Configured Memory Speed");
        articles.push_back(configSpeed);
        existArticles.insert("Configured Memory Speed");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("dmidecode", mem, articles, existArticles);

        QString deviceName;
        if(locator.isValid())
        {
            deviceName = locator.value;
        }
        else if(bankLocator.isValid())
        {
            deviceName = bankLocator.value;
        }
        else if(rank.isValid())
        {
            deviceName = rank.value;
        }

        if(size.value == DApplication::translate("Main", "Unknown") || size.value == "No Module Installed" )
        {
            articles.clear();

            ArticleStruct status("Status");
            status.value = DApplication::translate("Main", "Bad");
            articles.push_back(status);

            addSubInfo( deviceName, articles );
            continue;
        }


        addSubInfo( deviceName, articles );

        QString overviewVendor = vendor.value;
        overviewVendor += " ";
        overviewVendor += type.value;
        overviewVendor += " ";
        overviewVendor += speed.value;

        if( false == detailMem .contains(overviewVendor) )
        {
            detailMem.push_back(overviewVendor);
        }
    }

    overviewInfo_.value = size.value;
    if( detailMem.size() > 0 )
    {
        QString detail  = detailMem.join("/");
        if (detail.isEmpty() == false) {
            overviewInfo_.value += " (";
            overviewInfo_.value += detail;
            overviewInfo_.value += ")";
        }
    }
}

bool MemoryWidget::isSlotValid(const QString& size, const QString& speed )
{
    if(size.contains("Unknown",Qt::CaseInsensitive)|| size.contains("No Module Installed",Qt::CaseInsensitive))
    {
        return false;
    }
    if(speed.contains("MT/s") == false)//maybe weak
    {
        return false;
    }

    return true;
}
