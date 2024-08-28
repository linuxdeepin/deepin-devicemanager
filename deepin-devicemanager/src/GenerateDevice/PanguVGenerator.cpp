// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguVGenerator.h"

// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "EDIDParser.h"
#include "DeviceManager/DeviceNetwork.h"
#include <QProcess>
#include <QDir>
#include <QFileInfoList>

PanguVGenerator::PanguVGenerator()
{

}

void parseEDID(QStringList allEDIDS,QString input)
{
    for (auto edid:allEDIDS) {
        QProcess process;
        process.start(QString("hexdump %1").arg(edid));
        process.waitForFinished(-1);

        QString deviceInfo = process.readAllStandardOutput();
        if(deviceInfo.isEmpty())
            continue;

        QString edidStr;
        QStringList lines = deviceInfo.split("\n");
        for (auto line:lines) {
            QStringList words = line.trimmed().split(" ");
            if(words.size() != 9)
                continue;

            words.removeAt(0);
            QString l = words.join("");
            l.append("\n");
            edidStr.append(l);
        }

        lines = edidStr.split("\n");
        if(lines.size() > 3){
            EDIDParser edidParser;
            QString errorMsg;
            edidParser.setEdid(edidStr,errorMsg,"\n", false);

            QMap<QString, QString> mapInfo;
            mapInfo.insert("Vendor",edidParser.vendor());
            mapInfo.insert("Model",edidParser.model());
            mapInfo.insert("Date",edidParser.releaseDate());
            mapInfo.insert("Size",edidParser.screenSize());
            mapInfo.insert("Display Input",input);

            DeviceMonitor *device = new DeviceMonitor();
            device->setInfoFromEdid(mapInfo);
            DeviceManager::instance()->addMonitor(device);
        }
    }
}

void PanguVGenerator::generatorMonitorDevice()
{
    QString toDir = "/sys/class/drm";
    QDir toDir_(toDir);

    if (!toDir_.exists())
        return;

    QFileInfoList fileInfoList = toDir_.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == ".." || !fileInfo.fileName().startsWith("card"))
            continue;

        if(QFile::exists(fileInfo.filePath() + "/" + "edid")) {
            QStringList allEDIDS_all;
            allEDIDS_all.append(fileInfo.filePath() + "/" + "edid");
            QString interface = fileInfo.fileName().remove("card0-").remove("card1-").remove("card2-");
            parseEDID(allEDIDS_all,interface);
         }
    }
}

void PanguVGenerator::generatorNetworkDevice()
{
    QStringList ifconfigCardName =  getNetworkInfoFromifconfig();
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("lshw_network");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        QMap<QString, QString> tempMap = *it;
        /*
        capabilities: ethernet physical wireless
        configuration: broadcast=yes ip=10.4.6.115 multicast=yes wireless=IEEE 802.11
        */
//        if ((tempMap["configuration"].indexOf("wireless=IEEE 802.11") > -1) ||
//                (tempMap["capabilities"].indexOf("wireless") > -1)) {
//            continue;
//        }

        QString logicalName = tempMap["logical name"].trimmed();
        if(! ifconfigCardName.contains(logicalName))
            continue;

        DeviceNetwork *device = new DeviceNetwork();
        device->setInfoFromLshw(*it);
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        DeviceManager::instance()->addNetworkDevice(device);
    }


}

QStringList PanguVGenerator::getNetworkInfoFromifconfig()
{
    //通过ifconfig 判断网络是否valiate
    QStringList ret;
    QProcess process;
    QString cmd = "ifconfig -s";
    process.start(cmd);
    QString ifconfigInfo;
    bool re = process.waitForFinished(-1);
    if (!re)
        return  ret;
    ifconfigInfo = process.readAllStandardOutput();
    //截取查询到的各个网卡连接信息
    QStringList list = ifconfigInfo.split("\n");
    for (int i = 1; i < list.size(); i++) {  //skip Iface
        //filter "lo" 网卡
        if (list.at(i).contains("lo"))
            continue;

        QStringList line = list.at(i).split(" ", QString::SkipEmptyParts);
        {
            if(line.size() < 2)
                continue;
            if(line.at(0).isEmpty())
                continue;
            else
                ret.append(line.at(0));
        }
    }
    return ret;
}
