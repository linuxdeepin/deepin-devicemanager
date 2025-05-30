// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguGenerator.h"
#include "DDLog.h"
#include <QRegularExpression>
// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceComputer.h"

using namespace DDLog;

PanguGenerator::PanguGenerator()
{
    qCDebug(appLog) << "PanguGenerator constructor";
}

void PanguGenerator::generatorComputerDevice()
{
    qCDebug(appLog) << "PanguGenerator::generatorComputerDevice start";
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");

    DeviceComputer *device = new  DeviceComputer();

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &dmidecode1List = DeviceManager::instance()->cmdInfo("dmidecode1");
    const QList<QMap<QString, QString> >  &dmidecode2List = DeviceManager::instance()->cmdInfo("dmidecode2");
    const QList<QMap<QString, QString> >  &dmidecode3List = DeviceManager::instance()->cmdInfo("dmidecode3");

    if (dmidecode1List.size() > 1) {
        device->setVendor(dmidecode1List[1]["Manufacturer"], dmidecode2List[0]["Manufacturer"]);
        device->setName(dmidecode1List[1]["Product Name"], dmidecode2List[0]["Product Name"], dmidecode1List[1]["Family"], dmidecode1List[1]["Version"]);
    }

    if (dmidecode3List.size() > 1) {
        device->setType(dmidecode3List[1]["Type"]);
    }

    // setOsDescription
    QString os = "UOS";
    DSysInfo::DeepinType type = DSysInfo::deepinType();
    if (DSysInfo::DeepinProfessional == type)
        os =  "UOS 20";
    else if (DSysInfo::DeepinPersonal == type)
        os =  "UOS 20 Home";
    else if (DSysInfo::DeepinDesktop == type)
        os =  "Deepin 20 Beta";

    device->setOsDescription(os);

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        
        // 使用 QRegularExpression 替换 QRegExp
        QRegularExpression reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)");
        QRegularExpressionMatch match = reg.match(info);
        if (match.hasMatch()) {
            QString tmp = match.captured(0);
            info.remove(tmp);
            info.insert(match.capturedStart(), match.captured(1));
        }

        // 第二个正则表达式匹配
        reg.setPattern("(\\(root.*\\)) (\\(.*\\))");
        match = reg.match(info);
        if (match.hasMatch()) {
            QString tmp = match.captured(1);
            info.remove(tmp);
        }

        info.remove("version");
        device->setOS(info);
    }

    DeviceManager::instance()->addComputerDevice(device);
    qCDebug(appLog) << "PanguGenerator::generatorComputerDevice end, added computer device";
}
