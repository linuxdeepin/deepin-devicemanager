// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguVGenerator.h"

// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include "EDIDParser.h"

#include <QProcess>

PanguVGenerator::PanguVGenerator()
{

}

void PanguVGenerator::generatorMonitorDevice()
{
    // 生成显示设备信息
//    const QList<QMap<QString, QString>> lstMapHDMI = DeviceManager::instance()->cmdInfo("EDID_HDMI");
//    QList<QMap<QString, QString> >::const_iterator itHDMI = lstMapHDMI.begin();
//    for (; itHDMI != lstMapHDMI.end(); ++itHDMI) {
//        if ((*itHDMI).size() < 1)
//            continue;

//        //HDMI interface EDID information
//        DeviceMonitor *device = new  DeviceMonitor();
//        device->setInfoFromEdid(*itHDMI);
//        DeviceManager::instance()->addMonitor(device);
//    }

//    const QList<QMap<QString, QString>> lstMapVGA = DeviceManager::instance()->cmdInfo("EDID_VGA");
//    QList<QMap<QString, QString> >::const_iterator it = lstMapVGA.begin();
//    for (; it != lstMapVGA.end(); ++it) {
//        if ((*it).size() < 1)
//            continue;

//        //VGA interface EDID information
//        DeviceMonitor *device = new DeviceMonitor();
//        device->setInfoFromEdid(*it);
//        DeviceManager::instance()->addMonitor(device);
//    }

    QStringList allEDIDS;
    allEDIDS.append("/sys/devices/platform/hisi-drm/drm/card0/card0-HDMI-A-1/edid");
    allEDIDS.append("/sys/devices/platform/hisi-drm/drm/card0/card0-VGA-1/edid");
    allEDIDS.append("/sys/devices/platform/hldrm/drm/card0/card0-HDMI-A-1/edid");
    allEDIDS.append("/sys/devices/platform/hldrm/drm/card0/card0-VGA-1/edid");
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
            mapInfo.insert("Date",edidParser.releaseDate());
            mapInfo.insert("Size",edidParser.screenSize());

            DeviceMonitor *device = new DeviceMonitor();
            device->setInfoFromEdid(mapInfo);
            DeviceManager::instance()->addMonitor(device);
        }
    }

}
