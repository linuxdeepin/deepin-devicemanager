// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PanguxGenerator.h"
#include "DDLog.h"

// 其它头文件
#include <QRegularExpression>
#include "../DeviceManager/DeviceAudio.h"
#include "../DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceManager.h"

using namespace DDLog;

PanguXGenerator::PanguXGenerator()
{
    qCDebug(appLog) << "PanguXGenerator constructor";
}

void PanguXGenerator::getAudioInfoFromCatAudio()
{
    qCDebug(appLog) << "PanguXGenerator::getAudioInfoFromCatAudio start";
    QMap<QString, QString> tempMap;
    tempMap["Name"] = "OnBoard Audio";
    tempMap["Vendor"]= "HUAWEI";
    DeviceAudio *device = new DeviceAudio();
    device->setCanEnale(false);
    device->setCanUninstall(false);
    device->setForcedDisplay(true);
    device->setInfoFromCatAudio(tempMap);
    DeviceManager::instance()->addAudioDevice(device);

    getAudioInfoFromHwinfo();
    qCDebug(appLog) << "PanguXGenerator::getAudioInfoFromCatAudio end";
}

void PanguXGenerator::generatorBluetoothDevice()
{
    qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice start";
    DeviceBluetooth *device0 = new DeviceBluetooth();
    QMap<QString, QString> tempMap;
    tempMap["Vendor"] = "HISILICON";
    tempMap["Name"] = "Hi1105C";
    tempMap["Model"] = "Hi1105C";
    device0->setCanEnale(false);
    device0->setForcedDisplay(true);
    device0->setInfoFromTomlOneByOne(tempMap);
    DeviceManager::instance()->addBluetoothDevice(device0);

    const QList<QMap<QString, QString> > lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        // qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice process hwinfo usb info";
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice usb info not enough";
            continue;
        }
        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            // qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice is hub/mouse/keyboard, skip";
            continue;
        }
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0"|| (*it)["Device"].contains("bluetooth", Qt::CaseInsensitive)) {
            // qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice is bluetooth device, add it";
            DeviceBluetooth *device = new DeviceBluetooth();
            device->setCanEnale(false);
            device->setForcedDisplay(true);
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addBluetoothDevice(device);

            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
    qCDebug(appLog) << "PanguXGenerator::generatorBluetoothDevice end";
}
void PanguXGenerator::generatorCpuDevice()
{
    qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice start";
    HWGenerator::generatorCpuDevice();

    QFile file("/proc/cpuinfo");
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(appLog) << "PanguXGenerator::generatorCpuDevice open cpuinfo file failed";
        return;
    }

    QMap<QString, QString> mapInfo;
    QString cpuInfo = file.readAll();
    QStringList infos = cpuInfo.split("\n\n");
    foreach (const QString &info, infos) {
        // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice process cpu info";
        if (info.isEmpty()) {
            // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice cpu info is empty, skip";
            continue;
        }

        QStringList lines = info.split("\n");
        foreach (const QString &line, lines) {
            // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice process line:" << line;
            if (line.isEmpty()) {
                // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice line is empty, skip";
                continue;
            }
            QStringList words = line.split(QRegularExpression("[\\s]*:[\\s]*"));
            if (words.size() != 2) {
                // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice words size is not 2, skip";
                continue;
            }
            if ("Hardware" == words[0]) {
                // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice get cpu name:" << words[1];
                mapInfo.insert("Name", words[1]);
            }
        }
    }
    file.close();
    if (mapInfo.find("Name") == mapInfo.end()) {
        qCWarning(appLog) << "PanguXGenerator::generatorCpuDevice cpu name not found";
        return;
    }

    QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Cpu);
    for (int i = 0; i < lst.size(); i++) {
        // qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice set cpu info";
        DeviceBaseInfo *device = lst[i];
        DeviceManager::instance()->tomlDeviceMapSet(DT_Cpu, device,mapInfo);
    }
    qCDebug(appLog) << "PanguXGenerator::generatorCpuDevice end";
}
