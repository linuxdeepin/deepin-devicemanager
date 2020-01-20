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
#include <DApplication>
#include <QSet>

static int coresNumberArray[] =
{
    1,2,4,6,8,
    10,12,14,16,18,
    20,22,24,26,28,
    30,32,34,36,38,
    40,42,44,46,48,
    50,52,54,56,58,
    60,62,64,
    128
};

static QString NumberStrinArray[] =
{
    "One",
    "Two",
    "Four",
    "Six",
    "Eight",

    "Ten",
    "Twelve",
    "Fourteen",
    "Sixteen",
    "Eighteen",

    "Twenty",
    "Twenty-two",
    "Twenty-four",
    "Twenty-six",
    "Twenty-eight",
    "Thirty",

    "Thirty-two",
    "Thirty-four",
    "Thirty-six",
    "Thirty-eight",
    "Fourty",

    "Fourty-two",
    "Fourty-four",
    "Fourty-six",
    "Fourty-eight",
    "Fifty",

    "Fifty-two",
    "Fifty-four",
    "Fifty-six",
    "Fifty-eight",
    "Sixty",

    "Sixty-two",
    "Sixty-four",
    "One hundred and Twenty-eight",
};

DWIDGET_USE_NAMESPACE

CpuWidget::CpuWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, DApplication::translate("Main", "CPU"))
{
    initWidget();
}

void CpuWidget::initWidget()
{ 
    QStringList cpuList = DeviceInfoParser::Instance().getCatcpuCpuList();

    QString architecture =  DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Architecture");

    overviewInfo_.value = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Model name");
    overviewInfo_.value.remove(" CPU", Qt::CaseInsensitive);

    QString maxSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU max MHz");
    QString minSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU min MHz");

    double maxMHz = maxSpeed.remove(",").toDouble()/1000.0;

    double minMHz = minSpeed.remove(",").toDouble()/1000.0;
    if(maxMHz > 10000)  //AMD CPU speed is MHZ, intel is GHZ
    {
        maxMHz/=10000.0;
        minMHz/=10000.0;
    }

    speed_.clear();
    if(minMHz > 0.0 && maxMHz > 0.0 && maxMHz >= minMHz)
    {
        int index = overviewInfo_.value.indexOf('@');
        if(index > 0)
        {
            speed_ = overviewInfo_.value.mid(index+1).trimmed();
        }
        else
        {
            speed_ = QString::number(maxMHz) + "GHz";
        }

        speed_ += " (";
        speed_ += QString::number(minMHz);
        speed_ += " - ";
        speed_ += QString::number(maxMHz);
        speed_ += "GHz)";
    }
    //获取cpu频率失败，尝试从dmidecode中获取

    bool t_getCpuSpeedFromDmiSuccess = false;
    if (speed_.isEmpty()) {
        QString maxSpeed_DMI = DeviceInfoParser::Instance().queryData("dmidecode","Processor Information","Max Speed");
        QString curSpeed_DMI = DeviceInfoParser::Instance().queryData("dmidecode","Processor Information","Current Speed");

        QRegExp englishLetter("[a-zA-Z]");
        if (maxSpeed_DMI.contains("MHZ",Qt::CaseInsensitive)) {
            maxSpeed_DMI = maxSpeed_DMI.remove(englishLetter).trimmed();
        }
        if (curSpeed_DMI.contains("MHZ",Qt::CaseInsensitive)) {
            curSpeed_DMI = curSpeed_DMI.remove(englishLetter).trimmed();
        }

        double maxMHz = maxSpeed_DMI.toDouble();
        double curSpeedMHz = curSpeed_DMI.toDouble();
        if (maxMHz > 0.0 && curSpeedMHz >0.0 && maxMHz >= curSpeedMHz) {
            speed_ = QString::number(curSpeedMHz) + QString("(Max:") + QString::number(maxMHz) + QString(") MHz");
            t_getCpuSpeedFromDmiSuccess = true;
        }
    }

    QStringList headers;
    if (speed_.isEmpty()) {
       headers << "Name" << "Vendor" << "Architecture";
    } else {
        headers << "Name" << "Vendor" << DApplication::translate("CPU", "Speed") << "Architecture";
    }

    QList<QStringList> tabList;

    foreach(const QString& cpu, cpuList)
    {
        QString md = DeviceInfoParser::Instance().queryData("catcpu", cpu, "model name");

        QString mc = DeviceInfoParser::Instance().queryData("catcpu", cpu, "vendor_id");
        if(mc.isEmpty() || mc == DApplication::translate("Main", "Unknown"))
        {
            mc = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Vendor ID");
        }

        if(mc == DApplication::translate("Main", "Unknown"))
        {
            mc = md.split(" ").first();
        }

        QStringList tab = {};
        if (speed_.isEmpty()) {
            tab << md << mc <<architecture;
        } else {
            tab << md << mc << speed_ << architecture;
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

    if(sockets > 1)
    {
        i = 0;
        for( ; i < sizeof(coresNumberArray)/sizeof(int); ++i)
        {
            if(coresNumberArray[i] == sockets)
            {
                break;
            }
        }

        if( i < sizeof(coresNumberArray)/sizeof(int) )
        {
            overviewInfo_.value += DApplication::translate("CPU", NumberStrinArray[i].toStdString().data() );
        }
        else
        {
            overviewInfo_.value += QString::number(cores);
        }

        overviewInfo_.value += DApplication::translate("CPU", " Sockets");

        overviewInfo_.value += " / ";
    }

    i = 0;
    cores*=sockets;
    for( ; i < sizeof(coresNumberArray)/sizeof(int); ++i)
    {
        if(coresNumberArray[i] == cores)
        {
            break;
        }
    }

    if( i < sizeof(coresNumberArray)/sizeof(int) )
    {
        overviewInfo_.value += DApplication::translate("CPU", NumberStrinArray[i].toStdString().data() );
    }
    else
    {
        overviewInfo_.value += QString::number(cores);
    }

    overviewInfo_.value += DApplication::translate("CPU", " Core(s)");

    if(logicalCpus > 1)
    {
        i = 0;
        overviewInfo_.value += " / ";
        for( ; i < sizeof(coresNumberArray)/sizeof(int); ++i)
        {
            if(coresNumberArray[i] == logicalCpus)
            {
                break;
            }
        }

        if( i < sizeof(coresNumberArray)/sizeof(int) )
        {
            overviewInfo_.value += DApplication::translate("CPU", NumberStrinArray[i].toStdString().data() );
        }
        else
        {
            overviewInfo_.value += QString::number(logicalCpus);
        }

        overviewInfo_.value += DApplication::translate("CPU", " Precessor(s)");
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

    ArticleStruct speed(DApplication::translate("CPU", "Speed"));
    speed.value = speed_;
    articles.push_back(speed);

    ArticleStruct cpuCores("CPU cores");
    cpuCores.queryData("lscpu", "lscpu", "CPU(s)", existSet, articles);

    ArticleStruct tamount("Threads amount");
    tamount.value = QString::number(cores*threadsPerCore);
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

    foreach(auto precessor, cpuList)
    {
        addPrecessor(precessor);
    }
}

void CpuWidget::addPrecessor(const QString& precessor)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    ArticleStruct name("Name");
    name.queryData("catcpu", precessor, "model name" );
    articles.push_back(name);
    existArticles.insert("model name");

    ArticleStruct vendor("Vendor");
    vendor.queryData("catcpu", precessor, "vendor_id" );
    vendor.queryData("lscpu", "lscpu", "Vendor ID" );

    articles.push_back(vendor);
    existArticles.insert("vendor_id");

    ArticleStruct cpuid("Cpu id");
    cpuid.queryData("catcpu", precessor, "physical id" );
    articles.push_back(cpuid);
    existArticles.insert("physical id");

    ArticleStruct coreid("Core id");
    coreid.queryData("catcpu", precessor, "core id" );
    articles.push_back(coreid);
    existArticles.insert("core id");

    ArticleStruct threadamount("Threads amount");
    threadamount.queryData("lscpu", "lscpu", "Thread(s) per core" );
    articles.push_back(threadamount);

    ArticleStruct speed("Current Speed");
    speed.queryData("catcpu", precessor, "cpu MHz" );
    articles.push_back(speed);
    existArticles.insert("cpu MHz");

    ArticleStruct BogoMIPS("BogoMIPS");
    BogoMIPS.queryData( "catcpu", precessor, "bogomips" );
    articles.push_back(BogoMIPS);
    existArticles.insert("bogomips");

    ArticleStruct architecture("Architecture");
    architecture.queryData( "lscpu", "lscpu", "Architecture" );
    articles.push_back(architecture);

    ArticleStruct cpufamily("Cpu Family");
    cpufamily.queryData("catcpu", precessor, "cpu family" );
    articles.push_back(cpufamily);
    existArticles.insert("cpu family");

    ArticleStruct model("Model");
    model.queryData("catcpu", precessor, "model" );
    articles.push_back(model);
    existArticles.insert("model");

    ArticleStruct stepping("Stepping");
    stepping.queryData("catcpu", precessor, "stepping" );
    articles.push_back(stepping);
    existArticles.insert("stepping");

    ArticleStruct l1d( "L1d Cache");
    l1d.queryData( "lscpu", "lscpu", "L1d cache" );
    articles.push_back(l1d);

    ArticleStruct l1i( "L1i Cache");
    l1i.queryData( "lscpu", "lscpu", "L1i cache" );
    articles.push_back(l1i);

    ArticleStruct l2( "L2 Cache");
    l2.queryData( "lscpu", "lscpu", "L2 cache" );
    articles.push_back(l2);

    ArticleStruct l3( "L3 Cache");
    l3.queryData( "lscpu", "lscpu", "L3 cache" );
    articles.push_back(l3);

    ArticleStruct flags("Flags");
    flags.queryData( "catcpu", precessor, "flags" );
    articles.push_back(flags);
    existArticles.insert("flags");

    ArticleStruct virtualization("Virtualization");
    virtualization.queryData( "lscpu", "lscpu", "Virtualization" );
    articles.push_back(virtualization);

    DeviceInfoParser::Instance().queryRemainderDeviceInfo("catcpu", precessor, articles, existArticles);

    addSubInfo( DApplication::translate("CPU", "Processor") + " " +  precessor, articles);
    articles.clear();
}
