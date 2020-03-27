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

#include "cpuwidget.h"
#include "deviceinfoparser.h"
#include  <QObject>
#include <DApplication>
#include <QSet>
#include <QDBusInterface>

const int coresNumberArray[] = {
    1, 2, 4, 6, 8,
    10, 12, 14, 16, 18,
    20, 22, 24, 26, 28,
    30, 32, 34, 36, 38,
    40, 42, 44, 46, 48,
    50, 52, 54, 56, 58,
    60, 62, 64,
    128
};

DWIDGET_USE_NAMESPACE

CpuWidget::CpuWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("CPU"))
{
    initWidget();
}

void CpuWidget::initWidget()
{
    QStringList cpuList = DeviceInfoParser::Instance().getCatcpuCpuList();

    QString architecture =  DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Architecture");

//    overviewInfo_.value = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Model name");
//    overviewInfo_.value.remove(" CPU", Qt::CaseInsensitive);
    QDBusInterface cpuModelName("com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo");
    QString cpuModel  = cpuModelName.property("Processor").toString();
    overviewInfo_.value = cpuModel;
    overviewInfo_.value.remove(" CPU", Qt::CaseInsensitive);

    QString maxSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU max MHz");
    QString minSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU min MHz");

    double maxMHz = maxSpeed.remove(",").toDouble();

    double minMHz = minSpeed.remove(",").toDouble();

    QString speedUnit = "MHz";
    if (maxMHz > 1000) { //AMD CPU speed is MHZ, intel is GHZ
        maxMHz /= 1000.0;
        minMHz /= 1000.0;
        speedUnit = "GHz";
    }

    double modelSpeed = 0;
    int index = overviewInfo_.value.indexOf('@');
    if (index > 0) {
        QString t_modelSpeed = overviewInfo_.value;
        modelSpeed = t_modelSpeed.mid(index + 1).trimmed().remove("MHz", Qt::CaseInsensitive).remove("GHz", Qt::CaseInsensitive).toDouble();
    }
    if (overviewInfo_.value.contains("GHz", Qt::CaseInsensitive) && speedUnit == "MHz") {
        modelSpeed *= 1000.0;
    }

    speed_.clear();
    bool minMaxMHzSuccess = (minMHz > 0.0 && maxMHz > 0.0 && maxMHz >= minMHz);
    bool modelSpeedSuccess = modelSpeed > 0.0;


    QString modeSpeedFormatStr = QString::number(modelSpeed, '0', 2);
    QString minMHzFormatStr    = QString::number(minMHz, '0', 2);
    QString maxMHzFormatStr    = QString::number(maxMHz, '0', 2);

    if (modelSpeedSuccess) {
        if (minMaxMHzSuccess) {
            speed_ = QString("%1(%2-%3) %4").arg(modeSpeedFormatStr).arg(minMHzFormatStr).arg(maxMHzFormatStr).arg(speedUnit);
        } else {
            speed_ = QString("%1 %2").arg(modeSpeedFormatStr).arg(speedUnit);
        }
    }
    if (modelSpeedSuccess == false && minMaxMHzSuccess) {
        speed_ = QString("%1-%2 %3").arg(minMHzFormatStr).arg(maxMHzFormatStr).arg(speedUnit);
    }


    //获取cpu频率失败，尝试从dmidecode中获取

    bool t_getCpuSpeedFromDmiSuccess = false;
    if (minMaxMHzSuccess == false && modelSpeedSuccess == false) {
        QRegExp letter("[a-zA-Z]");
        QString maxSpeed_DMI = DeviceInfoParser::Instance().queryData("dmidecode", "Processor Information", "Max Speed");

        if (maxSpeed_DMI.contains("HZ", Qt::CaseInsensitive)) {
            maxSpeed_DMI = maxSpeed_DMI.remove(letter).trimmed();
        }
//        QString curSpeed_DMI = DeviceInfoParser::Instance().queryData("dmidecode","Processor Information","Current Speed");
//        if (curSpeed_DMI.contains("HZ",Qt::CaseInsensitive)) {
//            curSpeed_DMI = curSpeed_DMI.remove(englishLetter).trimmed();
//        }

//        double curSpeedMHz = curSpeed_DMI.toDouble();
        double maxSpeed = maxSpeed_DMI.toDouble();
        if (maxSpeed > 0.0) {
            QString unit = "MHz";
            maxSpeed_DMI = DeviceInfoParser::Instance().queryData("dmidecode", "Processor Information", "Max Speed");
            if (maxSpeed_DMI.contains("GHz", Qt::CaseInsensitive)) {
                unit = "GHz";
            }
            speed_ = QString("%1 %2").arg(maxSpeed).arg(unit);
            t_getCpuSpeedFromDmiSuccess = true;
        }
    }

    QStringList headers;
    if (speed_.isEmpty()) {
        headers << tr("Name") << tr("Vendor") << tr("Architecture");
    } else {
        if (t_getCpuSpeedFromDmiSuccess) {
            headers << tr("Name") << tr("Vendor") << tr("Max Speed") << tr("Architecture");
        } else {
            headers << tr("Name") << tr("Vendor") << tr("Speed") << tr("Architecture");
        }
    }

    QList<QStringList> tabList;

    foreach (const QString &cpu, cpuList) {
//        QString md = DeviceInfoParser::Instance().queryData("catcpu", cpu, "model name");
        QString mc = DeviceInfoParser::Instance().queryData("catcpu", cpu, "vendor_id");
        if (mc.isEmpty() || mc == tr("Unknown")) {
            mc = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Vendor ID");
        }

        if (mc == tr("Unknown")) {
            mc = cpuModel.split(" ").first();
        }

        QStringList tab = {};
        if (speed_.isEmpty()) {
            tab << cpuModel << mc << architecture;
        } else {
            tab << cpuModel << mc << speed_ << architecture;
        }

        tabList.push_back(tab);
    }

    addTable(headers, tabList);

    int sockets = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Socket(s)").toInt();
    int cores = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Core(s) per socket").toInt();
    int logicalCpus = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU(s)").toInt();

    int threadsPerCore = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Thread(s) per core").toInt();
    QString corePlus = " x " + QString::number(cores);

    overviewInfo_.value += " (";
    unsigned int i = 0;

    if (sockets > 1) {
        i = 0;
        for (; i < sizeof(coresNumberArray) / sizeof(int); ++i) {
            if (coresNumberArray[i] == sockets) {
                break;
            }
        }

        if (i < sizeof(coresNumberArray) / sizeof(int)) {
            overviewInfo_.value += getTrNumber(i);
        } else {
            overviewInfo_.value += QString::number(cores);
        }

        overviewInfo_.value += (" " + tr("Sockets"));

        overviewInfo_.value += " / ";
    }

    i = 0;
    cores *= sockets;
    for (; i < sizeof(coresNumberArray) / sizeof(int); ++i) {
        if (coresNumberArray[i] == cores) {
            break;
        }
    }

    if (i < sizeof(coresNumberArray) / sizeof(int)) {
        QString t = getTrNumber(i);
        overviewInfo_.value += t;
    } else {
        overviewInfo_.value += QString::number(cores);
    }

    overviewInfo_.value += (" " + tr("Core(s)"));

    if (logicalCpus > 1) {
        i = 0;
        overviewInfo_.value += " / ";
        for (; i < sizeof(coresNumberArray) / sizeof(int); ++i) {
            if (coresNumberArray[i] == logicalCpus) {
                break;
            }
        }

        if (i < sizeof(coresNumberArray) / sizeof(int)) {
            overviewInfo_.value += getTrNumber(i);
        } else {
            overviewInfo_.value += QString::number(logicalCpus);
        }

        overviewInfo_.value += (" " + tr("Processor(s)"));
    }

    overviewInfo_.value += ")";

    QList<ArticleStruct> articles;
    QSet<QString> existSet;

    ArticleStruct model("Model");
    model.value = overviewInfo_.value;
    articles.push_back(model);

    ArticleStruct ac("Architecture");
    ac.value = architecture;
    articles.push_back(ac);

    ArticleStruct vendor("Vendor");
    vendor.queryData("lscpu", "lscpu", "Vendor ID", existSet, articles);

    ArticleStruct speed(tr("Speed"));
    speed.value = speed_;
    articles.push_back(speed);

    ArticleStruct cpuCores("CPU cores");
    cpuCores.queryData("lscpu", "lscpu", "CPU(s)", existSet, articles);

    ArticleStruct tamount("Threads");
    tamount.value = QString::number(cores * threadsPerCore);
    articles.push_back(tamount);

    ArticleStruct l1dCache("L1d Cache");
    l1dCache.value = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "L1d cache") + corePlus;
    articles.push_back(l1dCache);

    ArticleStruct l1iCache("L1i Cache");
    l1iCache.value = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "L1i cache") + corePlus;
    articles.push_back(l1iCache);

    ArticleStruct l2Cache("L2 Cache");
    l2Cache.value = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "L2 cache") + corePlus;
    articles.push_back(l2Cache);

    ArticleStruct l3Cache("L3 Cache");
    l3Cache.queryData("lscpu", "lscpu", "L3 cache");
    articles.push_back(l3Cache);

//    ArticleStruct sockets("Socket(s)");
//    l3Cache.queryData("lscpu", "lscpu", "Socket(s)");
//    articles.push_back(l3Cache);

    //addInfo("Cpu Info", articles, false);

    foreach (auto precessor, cpuList) {
        addPrecessor(precessor);
    }

    //huawei requirement
    if (DeviceInfoParser::Instance().isHuaweiAndroidUos()) {
        overviewInfo_.value = QString("%1 (%2 %3)").\
                              arg(cpuModel).\
                              arg(getTrNumber(4)).\
                              arg(tr("Core(s)"));
    }
}

const QString getTrNumber(int i)
{
    static const  QStringList trNumbers = {
        QObject::tr("One"),
        QObject::tr("Two"),
        QObject::tr("Four"),
        QObject::tr("Six"),
        QObject::tr("Eight"),
        //                                         ,
        QObject::tr("Ten"),
        QObject::tr("Twelve"),
        QObject::tr("Fourteen"),
        QObject::tr("Sixteen"),
        QObject::tr("Eighteen"),
        //                                         ,
        QObject::tr("Twenty"),
        QObject::tr("Twenty-two"),
        QObject::tr("Twenty-four"),
        QObject::tr("Twenty-six"),
        QObject::tr("Twenty-eight"),
        QObject::tr("Thirty"),
        //                                         ,
        QObject::tr("Thirty-two"),
        QObject::tr("Thirty-four"),
        QObject::tr("Thirty-six"),
        QObject::tr("Thirty-eight"),
        QObject::tr("Forty"),
        //                                         ,
        QObject::tr("Forty-two"),
        QObject::tr("Forty-four"),
        QObject::tr("Forty-six"),
        QObject::tr("Forty-eight"),
        QObject::tr("Fifty"),
        //                                         ,
        QObject::tr("Fifty-two"),
        QObject::tr("Fifty-four"),
        QObject::tr("Fifty-six"),
        QObject::tr("Fifty-eight"),
        QObject::tr("Sixty"),
        //                                         ,
        QObject::tr("Sixty-two"),
        QObject::tr("Sixty-four"),
        QObject::tr("One hundred and Twenty-eight"),
    };
    if (i < trNumbers.size()) {
        return trNumbers.at(i);
    }
    return trNumbers.first();
}

void CpuWidget::addPrecessor(const QString &precessor)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    ArticleStruct name(tr("Name"));
    name.queryData("catcpu", precessor, "model name");
    articles.push_back(name);
    existArticles.insert("model name");

    ArticleStruct vendor(tr("Vendor"));
    vendor.queryData("catcpu", precessor, "vendor_id");
    vendor.queryData("lscpu", "lscpu", "Vendor ID");

    articles.push_back(vendor);
    existArticles.insert("vendor_id");

    ArticleStruct cpuid(tr("CPU ID"));
    cpuid.queryData("catcpu", precessor, "physical id");
    articles.push_back(cpuid);
    existArticles.insert("physical id");

    ArticleStruct coreid(tr("Core ID"));
    coreid.queryData("catcpu", precessor, "core id");
    articles.push_back(coreid);
    existArticles.insert("core id");

    ArticleStruct threadamount(tr("Threads"));
    threadamount.queryData("lscpu", "lscpu", "Thread(s) per core");
    articles.push_back(threadamount);

    ArticleStruct speed(tr("Current Speed"));
    speed.queryData("catcpu", precessor, "cpu MHz");
    articles.push_back(speed);
    existArticles.insert("cpu MHz");

    ArticleStruct BogoMIPS(tr("BogoMIPS"));
    BogoMIPS.queryData("catcpu", precessor, "bogomips");
    articles.push_back(BogoMIPS);
    existArticles.insert("bogomips");

    ArticleStruct architecture(tr("Architecture"));
    architecture.queryData("lscpu", "lscpu", "Architecture");
    articles.push_back(architecture);

    ArticleStruct cpufamily(tr("CPU Family"));
    cpufamily.queryData("catcpu", precessor, "cpu family");
    articles.push_back(cpufamily);
    existArticles.insert("cpu family");

    ArticleStruct model(tr("Model"));
    model.queryData("catcpu", precessor, "model");
    articles.push_back(model);
    existArticles.insert("model");

    ArticleStruct stepping(tr("Stepping"));
    stepping.queryData("catcpu", precessor, "stepping");
    articles.push_back(stepping);
    existArticles.insert("stepping");

    ArticleStruct l1d(tr("L1d Cache"));
    l1d.queryData("lscpu", "lscpu", "L1d cache");
    articles.push_back(l1d);

    ArticleStruct l1i(tr("L1i Cache"));
    l1i.queryData("lscpu", "lscpu", "L1i cache");
    articles.push_back(l1i);

    ArticleStruct l2(tr("L2 Cache"));
    l2.queryData("lscpu", "lscpu", "L2 cache");
    articles.push_back(l2);

    ArticleStruct l3(tr("L3 Cache"));
    l3.queryData("lscpu", "lscpu", "L3 cache");
    articles.push_back(l3);

    ArticleStruct flags(tr("Flags"));
    flags.queryData("catcpu", precessor, "flags");
    articles.push_back(flags);
    existArticles.insert("flags");

    ArticleStruct virtualization(tr("Virtualization"));
    virtualization.queryData("lscpu", "lscpu", "Virtualization");
    articles.push_back(virtualization);

    DeviceInfoParser::Instance().queryRemainderDeviceInfo("catcpu", precessor, articles, existArticles, "ManulTrack__CPU", "CPU Information");
    addSubInfo(tr("Processor") + " " +  precessor, articles);
    articles.clear();
}
