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

#include "printerwidget.h"
#include "deviceinfoparser.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PrinterWidget::PrinterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Printer"))
{
    initWidget();
}

//void PrinterWidget::loadWidget()
//{
//    QStringList hwinfoPrinterList = DeviceInfoParser::Instance().getHwinfoPrinterList();

//    if (hwinfoPrinterList.size() < 1) {
//        setCentralInfo(tr("No printer found"));
//        return;
//    }

//    QList<QStringList> tabList;
////    QList<ArticleStruct> articles;
////    QSet<QString> existArticles;

//    foreach (const QString &device, hwinfoPrinterList) {
//        m_articles.clear();
//        m_existArticles.clear();

//        // 添加printer相关信息
//        ArticleStruct name = addArticleStruct(tr("Name"), "printer", device, "Device");
//        ArticleStruct vendor = addArticleStruct(tr("Vendor"), "printer", device, "Vendor");
//        addArticleStruct(tr("Model"), "printer", device, "Model");
//        addArticleStruct(tr("Serial ID"), "printer", device, "Serial ID");
//        addArticleStruct(tr("Status"), "printer", device, "Config Status");
//        addArticleStruct(tr("Driver"), "printer", device, "Driver");
//        addArticleStruct(tr("Speed"), "printer", device, "Speed");
//        addArticleStruct(tr("BusID"), "printer", device, "SysFS BusID");
//        ArticleStruct description = addArticleStruct(tr("Description"), "printer", device, "description");

////        DeviceInfoParser::Instance().queryRemainderDeviceInfo("printer", device, articles, existArticles);

//        addDevice(device, m_articles, hwinfoPrinterList.size());

//        if (hwinfoPrinterList.size() > 1) {
//            QStringList tab = {
//                name.value,
//                vendor.value
//            };

//            tabList.push_back(tab);
//        }

//        if (overviewInfo_.value.isEmpty()) {
//            overviewInfo_.value = name.value;
//            if (false == overviewInfo_.value.contains(vendor.value, Qt::CaseInsensitive)) {
//                overviewInfo_.value = vendor.value + " " + name.value;
//            }
//        }
//    }

//    if (hwinfoPrinterList.size() > 1) {
//        QStringList headers = { tr("printer-info", "Printer Info,shown on first column of table's head"),
//                                tr("printer-make-and-model", "Printer Info,shown on second column of table's head ")
//                              };
//        addTable(headers, tabList);
//    }
//}

void PrinterWidget::initWidget()
{
    //setTitle(tr("Other Inputdevice")  + tr(" Info"));
    //获得所有打印信息
    QStringList printerList = DeviceInfoParser::Instance().getCupsPrinterList();

//    QStringList lshwPrinterList = DeviceInfoParser::Instance().getLshwPrinterList();

    if (printerList.size() < 1) {
//        QStringList hwinfoPrinterList = DeviceInfoParser::Instance().getHwinfoPrinterList();
        setCentralInfo(tr("No printer found"));
        return;
//        } else {
//            loadWidget();
//        }
    }

    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

//    foreach (const QString &device, lshwPrinterList) {
//        articles.clear();
//        existArticles.clear();

//        existArticles.insert("product");
//        ArticleStruct name(tr("Name"));
//        name.queryData("lshw", device, "product");
//        articles.push_back(name);
//        existArticles.insert("product");

//        ArticleStruct vendor(tr("Vendor"));
//        vendor.queryData("lshw", device, "vendor");
//        articles.push_back(vendor);
//        existArticles.insert("vendor");

//        ArticleStruct description(tr("Description"));
//        description.queryData("lshw", device, "description");
//        articles.push_back(description);
//        existArticles.insert("description");

//        ArticleStruct busInfo(tr("Bus Info"));
//        busInfo.queryData("lshw", device, "bus info");
//        articles.push_back(busInfo);
//        existArticles.insert("bus info");

//        ArticleStruct version(tr("Version"));
//        version.queryData("lshw", device, "version");
//        articles.push_back(version);
//        existArticles.insert("version");

//        ArticleStruct width(tr("Width"));
//        width.queryData("lshw", device, "width");
//        articles.push_back(width);
//        existArticles.insert("width");

//        ArticleStruct clock(tr("Clock"));
//        clock.queryData("lshw", device, "clock");
//        articles.push_back(clock);
//        existArticles.insert("clock");

//        ArticleStruct capabilities(tr("Capabilities"));
//        capabilities.queryData("lshw", device, "capabilities");
//        articles.push_back(capabilities);
//        existArticles.insert("capabilities");

//        DeviceInfoParser::Instance().queryRemainderDeviceInfo("lshw", device, articles, existArticles);

//        addDevice(device, articles, lshwPrinterList.size());

//        if (lshwPrinterList.size() > 1) {
//            QStringList tab = {
//                name.value,
//                vendor.value
//            };

//            tabList.push_back(tab);
//        }

//        if (overviewInfo_.value.isEmpty()) {
//            overviewInfo_.value = name.value;
//            if (false == overviewInfo_.value.contains(vendor.value, Qt::CaseInsensitive)) {
//                overviewInfo_.value = vendor.value + " " + name.value;
//            }
//        }
//    }
    foreach (const QString &device, printerList) {
        articles.clear();
        existArticles.clear();

        ArticleStruct copies(tr("Copies"));
        copies.queryData("Cups", device, "copies");
        articles.push_back(copies);
        existArticles.insert("copies");

        ArticleStruct deviceUri(tr("Device URI"));
        deviceUri.queryData("Cups", device, "device-uri");
        articles.push_back(deviceUri);
        existArticles.insert("device-uri");

        ArticleStruct finishings(tr("Finishings"));
        finishings.queryData("Cups", device, "finishings");
        articles.push_back(finishings);
        existArticles.insert("finishings");

        ArticleStruct jobCancelAfter(tr("Job Cancel After"));
        jobCancelAfter.queryData("Cups", device, "job-cancel-after");
        articles.push_back(jobCancelAfter);
        existArticles.insert("job-cancel-after");

        ArticleStruct jobHoldUntil(tr("Job Hold Until"));
        jobHoldUntil.queryData("Cups", device, "job-hold-until");
        articles.push_back(jobHoldUntil);
        existArticles.insert("job-hold-until");

        ArticleStruct jobPriority(tr("Job Priority"));
        jobPriority.queryData("Cups", device, "job-priority");
        articles.push_back(jobPriority);
        existArticles.insert("job-priority");

        ArticleStruct jobSheets(tr("Job Sheets"));
        jobSheets.queryData("Cups", device, "job-sheets");
        articles.push_back(jobSheets);
        existArticles.insert("job-sheets");

        ArticleStruct makerChangeTime(tr("Marker Change Time"));
        makerChangeTime.queryData("Cups", device, "marker-change-time");
        articles.push_back(makerChangeTime);
        existArticles.insert("marker-change-time");

        ArticleStruct media(tr("Media"));
        media.queryData("Cups", device, "media");
        articles.push_back(media);
        existArticles.insert("media");

        ArticleStruct numberUp(tr("Number Up"));
        numberUp.queryData("Cups", device, "number-up");
        articles.push_back(numberUp);
        existArticles.insert("number-up");

        ArticleStruct orientationRequested(tr("Orientation Requested"));
        orientationRequested.queryData("Cups", device, "orientation-requested");
        articles.push_back(orientationRequested);
        existArticles.insert("orientation-requestedr");

        ArticleStruct printColorMode(tr("Print Color Mode"));
        printColorMode.queryData("Cups", device, "print-color-mode");
        articles.push_back(printColorMode);
        existArticles.insert("print-color-mode");

        ArticleStruct printerInfo(tr("Printer Info"));
        printerInfo.queryData("Cups", device, "printer-info");
        articles.push_back(printerInfo);
        existArticles.insert("printer-info");

        ArticleStruct printerIsAcceptingJobs(tr("Printer Is Accepting Jobs"));
        printerIsAcceptingJobs.queryData("Cups", device, "printer-is-accepting-jobs");
        articles.push_back(printerIsAcceptingJobs);
        existArticles.insert("printer-is-accepting-jobs");

        ArticleStruct printerIsShared(tr("Printer Is Shared"));
        printerIsShared.queryData("Cups", device, "printer-is-shared");
        articles.push_back(printerIsShared);
        existArticles.insert("printer-is-shared");

        ArticleStruct printerIsTemporary(tr("Printer Is Temporary"));
        printerIsTemporary.queryData("Cups", device, "printer-is-temporar");
        articles.push_back(printerIsTemporary);
        existArticles.insert("printer-is-temporar");

        ArticleStruct printerMakeAndModel(tr("Printer Make And Model"));
        printerMakeAndModel.queryData("Cups", device, "printer-make-and-model");
        articles.push_back(printerMakeAndModel);
        existArticles.insert("printer-make-and-model");

        ArticleStruct printerState(tr("Printer State"));
        printerState.queryData("Cups", device, "printer-state");
        articles.push_back(printerState);
        existArticles.insert("printer-state");

        ArticleStruct printerStateChangeTime(tr("Printer State Change Time"));
        printerStateChangeTime.queryData("Cups", device, "printer-state-change-time");
        articles.push_back(printerStateChangeTime);
        existArticles.insert("printer-state-change-time");

        ArticleStruct printerStateReasons(tr("Printer State Reasons"));
        printerStateReasons.queryData("Cups", device, "printer-state-reasons");
        articles.push_back(printerStateReasons);
        existArticles.insert("printer-state-reasons");

        ArticleStruct printerType(tr("Printer Type"));
        printerType.queryData("Cups", device, "printer-type");
        articles.push_back(printerType);
        existArticles.insert("printer-type");

        ArticleStruct printerUriSupported(tr("Printer URI Supported"));
        printerUriSupported.queryData("Cups", device, "printer-uri-supported");
        articles.push_back(printerUriSupported);
        existArticles.insert("printer-uri-supported");

        ArticleStruct sides(tr("Sides"));
        sides.queryData("Cups", device, "sides");
        articles.push_back(sides);
        existArticles.insert("sides");

//        ArticleStruct numberUp(tr("number-up"));
//        busInfo.queryData("Cups", device, "number-up");
//        articles.push_back(numberUp);
//        existArticles.insert("number-up");

        DeviceInfoParser::Instance().queryRemainderDeviceInfo("Cups", device, articles, existArticles);

        addDevice(device, articles, printerList.size());

        //此处为添加表头，由于可以重复添加同一个打印机，printerInfo可能相同，导致在表头没区别
        // 因此可以添加device的toolname为表头
        QString printerName = device;

        QStringList tab = {
            printerName,
            printerMakeAndModel.value
        };

        tabList.push_back(tab);
        if (overviewInfo_.value.isEmpty()) {
            overviewInfo_.value = printerName;
        } else {
            overviewInfo_.value += " / " + printerName;
        }
    }

    if (printerList.size() > 1) {
        QStringList headers = { tr("Printer Info", "Printer Info,shown on first column of table's head"),
                                tr("Printer Make And Model", "Printer Info,shown on second column of table's head ")
                              };
        addTable(headers, tabList);
    }
}

