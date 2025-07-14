// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguVGenerator.h"
#include "DDLog.h"

// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "EDIDParser.h"
#include "DeviceManager/DeviceNetwork.h"
#include <QProcess>

using namespace DDLog;

PanguVGenerator::PanguVGenerator()
{
    qCDebug(appLog) << "PanguVGenerator constructor";

}

void parseEDID(QStringList allEDIDS,QString input)
{
    qCDebug(appLog) << "parseEDID start, input:" << input;
    for (auto edid:allEDIDS) {
        // qCDebug(appLog) << "parseEDID process edid:" << edid;
        QProcess process;
        process.start(QString("hexdump %1").arg(edid));
        process.waitForFinished(-1);

        QString deviceInfo = process.readAllStandardOutput();
        if(deviceInfo.isEmpty()) {
            qCWarning(appLog) << "parseEDID device info is empty, skip";
            continue;
        }

        QString edidStr;
        QStringList lines = deviceInfo.split("\n");
        for (auto line:lines) {
            // qCDebug(appLog) << "parseEDID process line:" << line;
            QStringList words = line.trimmed().split(" ");
            if(words.size() != 9) {
                // qCWarning(appLog) << "parseEDID words size is not 9, skip";
                continue;
            }

            words.removeAt(0);
            QString l = words.join("");
            l.append("\n");
            edidStr.append(l);
        }

        lines = edidStr.split("\n");
        if(lines.size() > 3){
            // qCDebug(appLog) << "parseEDID lines size > 3, parse edid";
            EDIDParser edidParser;
            QString errorMsg;
            edidParser.setEdid(edidStr,errorMsg,"\n", false);

            QMap<QString, QString> mapInfo;
            mapInfo.insert("Vendor",edidParser.vendor());
            mapInfo.insert("Date",edidParser.releaseDate());
            mapInfo.insert("Size",edidParser.screenSize());
            mapInfo.insert("Display Input",input);

            DeviceMonitor *device = new DeviceMonitor();
            device->setInfoFromEdid(mapInfo);
            DeviceManager::instance()->addMonitor(device);
            // qCDebug(appLog) << "parseEDID add monitor device";
        }
    }
    qCDebug(appLog) << "parseEDID end";
}

void PanguVGenerator::generatorMonitorDevice()
{
    qCDebug(appLog) << "PanguVGenerator::generatorMonitorDevice start";
    QStringList allEDIDS1;
    allEDIDS1.append("/sys/devices/platform/hisi-drm/drm/card0/card0-HDMI-A-1/edid");
    allEDIDS1.append("/sys/devices/platform/hldrm/drm/card0/card0-HDMI-A-1/edid");
    parseEDID(allEDIDS1,"HDMI-A-1");

    QStringList allEDIDS2;
    allEDIDS2.append("/sys/devices/platform/hisi-drm/drm/card0/card0-VGA-1/edid");
    allEDIDS2.append("/sys/devices/platform/hldrm/drm/card0/card0-VGA-1/edid");
    parseEDID(allEDIDS2,"VGA-1");
    qCDebug(appLog) << "PanguVGenerator::generatorMonitorDevice end";
}

void PanguVGenerator::generatorNetworkDevice()
{
    qCDebug(appLog) << "PanguVGenerator::generatorNetworkDevice start";
    QStringList ifconfigCardName =  getNetworkInfoFromifconfig();
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("lshw_network");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        // qCDebug(appLog) << "PanguVGenerator::generatorNetworkDevice process lshw network info";
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "PanguVGenerator::generatorNetworkDevice network info not enough";
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
        if(! ifconfigCardName.contains(logicalName)) {
            // qCDebug(appLog) << "PanguVGenerator::generatorNetworkDevice logical name not in ifconfig list, skip";
            continue;
        }

        DeviceNetwork *device = new DeviceNetwork();
        device->setInfoFromLshw(*it);
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        DeviceManager::instance()->addNetworkDevice(device);
    }

    qCDebug(appLog) << "PanguVGenerator::generatorNetworkDevice end";
}

QStringList PanguVGenerator::getNetworkInfoFromifconfig()
{
    qCDebug(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig start";
    //通过ifconfig 判断网络是否valiate
    QStringList ret;
    QProcess process;
    QString cmd = "ifconfig -s";
    process.start(cmd);
    QString ifconfigInfo;
    bool re = process.waitForFinished(-1);
    if (!re) {
        qCWarning(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig ifconfig command failed";
        return  ret;
    }
    ifconfigInfo = process.readAllStandardOutput();
    //截取查询到的各个网卡连接信息
    QStringList list = ifconfigInfo.split("\n");
    for (int i = 1; i < list.size(); i++) {  //skip Iface
        // qCDebug(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig process ifconfig line:" << list.at(i);
        //filter "lo" 网卡
        if (list.at(i).contains("lo")) {
            // qCDebug(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig is lo interface, skip";
            continue;
        }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QStringList line = list.at(i).split(" ", QString::SkipEmptyParts);
#else
        QStringList line = list.at(i).split(" ", Qt::SkipEmptyParts);
#endif
        {
            if(line.size() < 2) {
                // qCWarning(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig line size < 2, skip";
                continue;
            }
            if(line.at(0).isEmpty()) {
                // qCWarning(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig line is empty, skip";
                continue;
            }
            else
                ret.append(line.at(0));
        }
    }
    qCDebug(appLog) << "PanguVGenerator::getNetworkInfoFromifconfig end, found interfaces:" << ret;
    return ret;
}
