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

#include "monitorwidget.h"
#include "deviceinfoparser.h"
#include "edidparser.h"
#include "math.h"
#include <QDate>
#include <DApplication>
#include "Logger.h"
#include <assert.h>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int gcd(int a, int b)
{
    if (a < b)
        std::swap(a, b);
    if (a % b == 0)
        return b;
    else
        return gcd(b, a % b);
}

bool findAspectRatio(int width, int height, int &ar_w, int &ar_h)
{
    float r1 = float(width) / float(height);

    for (ar_w = 21; ar_w > 0; --ar_w) {
        for (ar_h = 21; ar_h > 0; --ar_h) {
            if ( std::abs(r1 - float(ar_w) / float(ar_h)) / r1 < 0.01) {
                int r = gcd(ar_w, ar_h);
                ar_w /= r;
                ar_h /= r;
                return true;
            }
        }
    }

    return false;
}

MonitorWidget::MonitorWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Monitor"))
{
    initWidget();
}

void MonitorWidget::initWidget()
{
    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    if(DeviceInfoParser::Instance().isHuaweiAndroidUos()) {
        return initHwMonitor();
    }
    QStringList hwinfMonitorList = DeviceInfoParser::Instance().getHwinfoMonitorList();
    QStringList xrandrMonitorList = DeviceInfoParser::Instance().getXrandrMonitorList();

    int maxSize = std::max(hwinfMonitorList.size(), xrandrMonitorList.size());

    for (int i = 0; i < maxSize; ++i) {
        articles.clear();
        existArticles.clear();

        ArticleStruct name(tr("Name"));
        ArticleStruct vendor(tr("Vendor"));
        ArticleStruct currentResolution(tr("Resolution"));
        ArticleStruct resolutionList(tr("Support Resolution"));
        ArticleStruct displayRatio(tr("Display Ratio"));

        ArticleStruct monitorSize(tr("Size"));

        double inch = 0.0;

        if ( i < hwinfMonitorList.size() ) {
            const QString &monitor = hwinfMonitorList.at(i);

            name.queryData("hwinfo", monitor, "Model");
            name.value = name.value.remove("\"");

            existArticles.insert("Model");

            vendor.queryData("hwinfo", monitor, "Vendor");
            QString abb;
            QRegExp rx("(^[\\s\\S]*)\"([\\s\\S]+)\"$");
            if ( rx.exactMatch(vendor.value) ) {
                abb = rx.cap(1).trimmed();
                vendor.value = rx.cap(2).trimmed();
            }
            existArticles.insert("Vendor");

            name.value.remove(abb);

            if (name.value.contains(vendor.value, Qt::CaseInsensitive) == false) {
                name.value = vendor.value + " " + name.value;
            }

            articles.push_back(name);
            articles.push_back(vendor);

            ArticleStruct serial(tr("Serial Number"));
            serial.queryData("hwinfo", monitor, "Serial ID");
            serial.value.remove("\"");
            articles.push_back(serial);
            existArticles.insert("Serial ID");

            QString sizeDescrition = DeviceInfoParser::Instance().queryData("hwinfo", monitor, "Size");
            QSize size(0, 0);
            QString inchValue = parseMonitorSize(sizeDescrition, inch, size);

            ArticleStruct mDate(tr("Manufacture Date"));
            mDate.queryData("hwinfo", monitor, "Year of Manufacture");
            if ( mDate.isValid() && mDate.value.toInt() != 0) {
                mDate.value = mDate.value + tr("Year");

                QString mw = DeviceInfoParser::Instance().queryData("hwinfo", monitor, "Week of Manufacture");
                if ( mw.isEmpty() == false && mw != tr("Unknown") && mw != "0") {
                    mDate.value += " ";
                    mDate.value += mw;
                    mDate.value += tr("Week");
                }
                articles.push_back(mDate);

                existArticles.insert("Year of Manufacture");
                existArticles.insert("Week of Manufacture");
            }


            ArticleStruct tmy(tr("The Model Year(Not Manufacture Date)"));
            tmy.queryData("hwinfo", monitor, "The Model Year", existArticles );
            if ( tmy.isValid() ) {
                tmy.value = tmy.value + tr("Year");
                articles.push_back(tmy);
            }

            resolutionList.queryData("hwinfo", monitor, "Support Resolution");
        }

        ArticleStruct primaryMonitor(tr("Primary Monitor"));
        primaryMonitor.value = "No";
        if ( i < xrandrMonitorList.size()) {
            if (name.isValid() == false) {
                int connectedIndex = xrandrMonitorList.at(i).indexOf("connected", Qt::CaseInsensitive);
                if (connectedIndex > 0) {
                    name.value = xrandrMonitorList.at(i).left(connectedIndex);
                }
            }

            if ( true == xrandrMonitorList.at(i).contains("primary", Qt::CaseInsensitive) ) {
                primaryMonitor.value = "Yes";
            }


            currentResolution.queryData("xrandr", xrandrMonitorList.at(i), "Resolution");
            //guarantee resulotion end with "@","\\d+","hz|HZ|Hz"
            parseCurResolution(currentResolution, resolutionList);
            articles.push_back(currentResolution);

            displayRatio.value = parseDisplayRatio(currentResolution.value);
            articles.push_back(displayRatio);

            if (monitorSize.isValid() == false) {
                monitorSize.queryData("xrandr", xrandrMonitorList.at(i), "Size");
                QSize size;
                QString inchValue = parseMonitorSize(monitorSize.value, inch, size);
                monitorSize.value = inchValue;

            }

            //always get monitor size from EDID
//            QString sizeValue = "";
//            sizeValue = getMonitorSizeFromEDID();
//            if (!sizeValue.isEmpty()) {
//                monitorSize.value = sizeValue;
//            }
            if (monitorSize.isValid()) {
                articles.push_back(monitorSize);
            }
        }
        articles.push_back(primaryMonitor);

        ArticleStruct connectType(tr("Connect Type"));
        connectType.value = tr("Unknown");
        if ( i < xrandrMonitorList.size()) {
            connectType.value = xrandrMonitorList.at(i);
            int index = connectType.value.indexOf(' ');
            if ( index > 0 ) {
                connectType.value = connectType.value.mid(0, index);
            }
            index = connectType.value.indexOf('-');
            if ( index > 0 ) {
                connectType.value = connectType.value.mid(0, index);
            }
        }
        articles.push_back(connectType);

        articles.push_back(resolutionList);

        addDevice( name.value, articles, maxSize );

        if ( maxSize > 1 ) {
            QStringList tab = {
                name.value,
                vendor.value
            };

            tabList.push_back(tab);
        }

        if (overviewInfo_.value.isEmpty() == false) {
            overviewInfo_.value += " / ";
        }

        overviewInfo_.value += name.value;
        if (inch != 0.0) {
            if (overviewInfo_.isValid() == false) {
                overviewInfo_.value = QString::number(inch, 10, 1) + " " + tr("inch");
            } else {
                overviewInfo_.value += " (";
                overviewInfo_.value += QString::number(inch, 10, 1) + " " + tr("inch");
                overviewInfo_.value += ")";
            }
        }
    }

    if ( maxSize > 1 ) {
        QStringList headers = { tr("Name"),  tr("Vendor") };
        addTable( headers, tabList);
    }
    if (maxSize == 0 ) {
        setCentralInfo(tr("Failed to find monitor information"));
    }
}

QString MonitorWidget::parseMonitorSize(const QString &sizeDescription, double &inch, QSize &retSize)
{
    inch = 0.0;

    QString res = sizeDescription;
    QRegExp re("^([\\d]*)x([\\d]*) mm$");
    if ( re.exactMatch(sizeDescription) ) {
        double width = re.cap(1).toDouble();
        double height = re.cap(2).toDouble();
        retSize = QSize(width, height);
        width /= 2.54;
        height /= 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    re.setPattern("([0-9]\\d*)mm x ([0-9]\\d*)mm");
    if ( re.exactMatch(sizeDescription) ) {
        double width = re.cap(1).toDouble();
        double height = re.cap(2).toDouble();
        retSize = QSize(width, height);
        width /= 2.54;
        height /= 2.54;
        inch = std::sqrt(width * width + height * height) / 10.0;
        res = QString::number(inch, 10, 1) + " " + tr("inch") + " (";
        res += sizeDescription;
        res += ")";
    }

    return res;
}

QString MonitorWidget::parseDisplayRatio(const QString &resulotion)
{
    QRegExp re("^([\\d]*)x([\\d]*)(.*)$");
    QString res;
    if ( re.exactMatch(resulotion) ) {
        int width = re.cap(1).toInt();
        int height = re.cap(2).toInt();
        int gys = gcd(width, height);
        int w = width / gys;
        int h = height / gys;

        if (w > 21) {
            findAspectRatio(w, h, w, h);
        }

        res = QString::number(w) + " : " + QString::number(h);
    }

    return res;
}

QString MonitorWidget::getMonitorSizeFromEDID()
{
    QString edid = DeviceInfoParser::Instance().getEDID();
    if (edid.isEmpty()) {
        return "";
    }
    QStringList list = edid.split('\n');
    QString secondItem = list.at(1);
    QString width_field = secondItem.mid(10, 2);
    QString height_field = secondItem.mid(12, 2);

    int width = 0;
    bool trWidthOk = false;
    int height = 0;
    bool trHeightOk = false;
    width = width_field.toInt(&trWidthOk, 16);
    height = height_field.toInt(&trHeightOk, 16);
    if (trWidthOk == false || trHeightOk == false) {
        return "";
    }
    if (height <= 0) return  "";
    if (width <= 0)return  "";
    double inch = std::sqrt(height * height + width * width) / 2.54;
    QString inchStr_tr = tr("inch");
    QString ret = QString("%1 %2(%3x%4 %5)").arg(QString::number(inch, '0', 1)).arg(inchStr_tr).arg(width).arg(height).arg(tr("cm", "size unit"));
    return ret;
}

void MonitorWidget::parseCurResolution(ArticleStruct &artOfCurResolution, ArticleStruct &artOfResolutionList)
{
    QStringList resolutionList = artOfResolutionList.value.split(",", QString::SkipEmptyParts);
    foreach (auto supportResolution, resolutionList) {
        supportResolution.remove(QRegExp("\\s"));
        if (supportResolution.startsWith(artOfCurResolution.value, Qt::CaseSensitive) &&
                QRegExp("^.*@\\d+(Hz|hz|HZ)$").exactMatch(supportResolution)
           ) {
            artOfCurResolution.value = supportResolution;
            break;
        }
    }
};

//content in table view widget is missed
void MonitorWidget::initHwMonitor()
{
    QList<ArticleStruct> articles;
    EDIDParser edidParser;
    edidParser.setSource("/sys/devices/platform/hisi-drm/drm/card0/card0-dp-1/edid");
    int monitorCount = 1;
    if(edidParser.isVaild()) monitorCount = 2;

    ArticleStruct vendor(tr("Vendor"));
    ArticleStruct currentResolution(tr("Resolution"));
    ArticleStruct resolutionList(tr("Support Resolution"));
    ArticleStruct date(tr("Manufacture Date"));
    ArticleStruct monitorSize(tr("Size"));

    vendor.value = "HUAWEI";
    currentResolution.value = "2160x1440@60Hz";
    resolutionList.value = "2160x1440@60Hz";
    date.value = QString("%1%2%3%4").arg(2019).arg(tr("Year")).arg(31).arg(tr("Week"));
    monitorSize.value = "296x197 mm";
    articles = {vendor, currentResolution, resolutionList, date, monitorSize};
    addDevice("Notebook monitor", articles, monitorCount );
    overviewInfo_.value = tr("Notebook monitor");
    if(edidParser.isVaild()){
        //external monitor
        vendor.value = edidParser.getManufatureName();
        resolutionList.value = edidParser.getSupportResolutionList().join(" ");
        int t_week = 1;int t_year = 2019;
        edidParser.getDate(t_week,t_year);
        date.value = QString("%1%2%3%4").arg(t_year).arg(tr("Year")).arg(t_week).arg(tr("Week"));
        monitorSize.value = edidParser.getMaxImageSize();
        articles = {vendor,resolutionList, date, monitorSize};
        addDevice("External monitor", articles, monitorCount );
        overviewInfo_.value += QString(" / ").arg(tr("External monitor"));
    }

    return;
}
