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
    // 获取cpu列表，即有多少个CPU
    QStringList cpuList = DeviceInfoParser::Instance().getDmidecodeCpuList();
    if(cpuList.size() < 1){return;}

    // 获取cpu核心列表，即有多少个CPU核
    QStringList coreList = DeviceInfoParser::Instance().getCatcpuCpuList();
    if(coreList.size() < 1){return;}

    // 获取CPU模型
    QDBusInterface cpuModelName("com.deepin.daemon.SystemInfo", "/com/deepin/daemon/SystemInfo", "com.deepin.daemon.SystemInfo");
    QString cpuModel  = cpuModelName.property("Processor").toString();
    overviewInfo_.value = cpuModel;
    overviewInfo_.value.remove(" CPU", Qt::CaseInsensitive);

    // 从 lscpu 获取 CPU 最大频率和最小频率
    QString maxSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU max MHz");
    QString minSpeed = DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "CPU min MHz");
    double maxMHz = maxSpeed.remove(",").toDouble();
    double minMHz = minSpeed.remove(",").toDouble();

    // 由于获取cpu的频率在不同的环境下的命令不同和方式不同，因此先判断是否可以从lscpu获取
    // 如果不能，则从dmidecode中获取
    speed_.clear();
    bool canGetSpeedFromLscpu = isCanGetSpeedFromLsCpu(minMHz,maxMHz);
    if(canGetSpeedFromLscpu){
        //可以从lscpu获取频率
        getSpeedFromLscpu(minMHz,maxMHz);
    }else{
        //不可以从lscpu获取频率,从dmidecode中获取
        getSpeedFromDmidecode();
    }

    // 获取显示界面上面的表头
    QStringList headers;
    getTableHeader(canGetSpeedFromLscpu,headers);

    // 获取显示界面里面的内容
    QList<QStringList> tabList;
    getTableContentFromLscpu(cpuModel,coreList,tabList);

    // 将获取的数据写到表格
    addTable(headers, tabList);

    // 添加cpu详细信息
    foreach (auto precessor, coreList) {
        if(canGetSpeedFromLscpu){
            addDetailFromLscpuAndCatcpu(precessor);
        }else {
            addDetailFromLscpuCatcpuDmidecode(cpuList[0],precessor);
        }
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

void CpuWidget::addDetailFromLscpuAndCatcpu(const QString& precessor)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

    addArticleStruct(tr("Name"),"catcpu",precessor,"model name",articles,existArticles);
    addArticleStruct(tr("CPU ID"),"catcpu",precessor,"physical id",articles,existArticles);
    addArticleStruct(tr("Core ID"),"catcpu",precessor,"core id",articles,existArticles);
    addArticleStruct(tr("Threads"),"lscpu","lscpu","Thread(s) per core",articles,existArticles);
    addArticleStruct(tr("Current Speed"),"catcpu",precessor,"cpu MHz",articles,existArticles);
    addArticleStruct(tr("BogoMIPS"),"catcpu",precessor,"bogomips",articles,existArticles);
    addArticleStruct(tr("Architecture"),"lscpu","lscpu","Architecture",articles,existArticles);
    addArticleStruct(tr("CPU Family"),"catcpu",precessor,"cpu family",articles,existArticles);
    addArticleStruct(tr("Model"),"catcpu",precessor,"model",articles,existArticles);
    addArticleStruct(tr("Stepping"),"catcpu",precessor,"stepping",articles,existArticles);
    addArticleStruct(tr("L1d Cache"),"lscpu","lscpu","L1d cache",articles,existArticles);
    addArticleStruct(tr("L1i Cache"),"lscpu","lscpu","L1i cache",articles,existArticles);
    addArticleStruct(tr("L2 Cache"),"lscpu","lscpu","L2 cache",articles,existArticles);
    addArticleStruct(tr("L3 Cache"),"lscpu","lscpu","L3 cache",articles,existArticles);
    addArticleStruct(tr("Flags"),"catcpu",precessor,"flags",articles,existArticles);
    addArticleStruct(tr("Virtualization"),"lscpu","lscpu","Virtualization",articles,existArticles);
    DeviceInfoParser::Instance().queryRemainderDeviceInfo("catcpu", precessor, articles, existArticles, "ManulTrack__CPU", "CPU Information");
    addSubInfo(tr("Processor") + " " +  precessor, articles);
    articles.clear();

}

void CpuWidget::addDetailFromLscpuCatcpuDmidecode(const QString& dmiProcessor,const QString& catcpuProcessor)
{
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;

//    addArticleStruct(tr("Name"),"lscpu","lscpu","Model name",articles,existArticles);
//    //addArticleStruct(tr("CPU ID"),"dmidecode",precessor,"physical id",articles,existArticles);
//    //addArticleStruct(tr("Core ID"),"dmidecode",precessor,"core id",articles,existArticles);
//    addArticleStruct(tr("Threads"),"lscpu","lscpu","Thread(s) per core",articles,existArticles);
//    addArticleStruct(tr("Current Speed"),"dmidecode",dmiProcessor,"Current Speed",articles,existArticles);
//    //addArticleStruct(tr("BogoMIPS"),"dmidecode",precessor,"bogomips",articles,existArticles);
//    addArticleStruct(tr("Architecture"),"lscpu","lscpu","Architecture",articles,existArticles);
//    addArticleStruct(tr("CPU Family"),"dmidecode",precessor,"Family",articles,existArticles);
//    addArticleStruct(tr("Model"),"lscpu","lscpu","Model",articles,existArticles);
//    addArticleStruct(tr("Stepping"),"lscpu","lscpu","Stepping",articles,existArticles);
//    addArticleStruct(tr("L1d Cache"),"dmidecode",precessor,"L1 Cache Handle",articles,existArticles);
//    addArticleStruct(tr("L1i Cache"),"dmidecode",precessor,"L1 Cache Handle",articles,existArticles);
//    addArticleStruct(tr("L2 Cache"),"dmidecode",precessor,"L2 Cache Handle",articles,existArticles);
//    addArticleStruct(tr("L3 Cache"),"dmidecode",precessor,"L3 Cache Handle",articles,existArticles);
//    addArticleStruct(tr("Flags"),"lscpu","lscpu","Flags",articles,existArticles);
    //addArticleStruct(tr("Virtualization"),"dmidecode",precessor,"Virtualization",articles,existArticles);

    DeviceInfoParser::Instance().queryRemainderDeviceInfo("catcpu", dmiProcessor, articles, existArticles, "ManulTrack__CPU", "CPU Information");
    addSubInfo(tr("Processor") + " " +  dmiProcessor, articles);
    articles.clear();
}


void CpuWidget::addArticleStruct(const QString& name,const QString& command,const QString& item,const QString& attri,QList<ArticleStruct>& articles,QSet<QString>& existArticles)
{
    ArticleStruct Name(name);
    Name.queryData(command, item,attri);
    articles.push_back(Name);
    existArticles.insert(attri);
}

bool CpuWidget::isCanGetSpeedFromLsCpu(double& minMHz,double& maxMHz)
{
    if(maxMHz <= 0 || minMHz <= 0 || maxMHz < minMHz){
        return false;
    }else{
        return true;
    }
}

void CpuWidget::getSpeedFromLscpu(double& minMHz,double& maxMHz)
{
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
    bool modelSpeedSuccess = modelSpeed > 0.0;


    QString modeSpeedFormatStr = QString::number(modelSpeed, '0', 2);
    QString minMHzFormatStr    = QString::number(minMHz, '0', 2);
    QString maxMHzFormatStr    = QString::number(maxMHz, '0', 2);
    if (modelSpeedSuccess) {
        speed_ = QString("%1(%2-%3) %4").arg(modeSpeedFormatStr).arg(minMHzFormatStr).arg(maxMHzFormatStr).arg(speedUnit);
    }else {
        speed_ = QString("%1-%2 %3").arg(minMHzFormatStr).arg(maxMHzFormatStr).arg(speedUnit);
    }
}

void CpuWidget::getSpeedFromDmidecode()
{
    QString maxMHz = DeviceInfoParser::Instance().queryData("dmidecode", "Processor Information", "Max Speed");
    QString curMHz = DeviceInfoParser::Instance().queryData("dmidecode","Processor Information","Current Speed");
    speed_ = curMHz;
}
void CpuWidget::getTableHeader(bool getSpeedFromLscpu,QStringList& headers)
{
    if (speed_.isEmpty()) {
        headers << tr("Name") << tr("Vendor") << tr("Architecture");
    } else {
        if (getSpeedFromLscpu) {
            headers << tr("Name") << tr("Vendor") << tr("Max Speed") << tr("Architecture");
        } else {
            headers << tr("Name") << tr("Vendor") << tr("Speed") << tr("Architecture");
        }
    }
}
void CpuWidget::getTableContentFromLscpu(const QString& cpuModel,const QStringList& cpuList,QList<QStringList>& tabList)
{
    // 获取CPU架构
    QString architecture =  DeviceInfoParser::Instance().queryData("lscpu", "lscpu", "Architecture");

    foreach (const QString &cpu, cpuList) {
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
}
void CpuWidget::getTableContentFromDmidecode(const QString& cpuModel,const QStringList& cpuList,QList<QStringList>& tabList)
{

}
