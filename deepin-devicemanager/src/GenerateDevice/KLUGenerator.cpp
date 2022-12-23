// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "KLUGenerator.h"

// Qt库文件
#include <QDebug>
#include <QProcess>

// 其它头文件
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceInput.h"

KLUGenerator::KLUGenerator()
{

}

void KLUGenerator::getDiskInfoFromLshw()
{
    QString bootdevicePath("/proc/bootdevice/product_name");
    QString modelStr = "";
    QFile file(bootdevicePath);
    if (file.open(QIODevice::ReadOnly)) {
        modelStr = file.readLine().simplified();
        file.close();
    }

    const QList<QMap<QString, QString>> lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2)
            continue;

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*dIt).keys()) {
            tempMap.insert(key, (*dIt)[key]);
        }

//        qInfo() << tempMap["product"] << " ***** " << modelStr << " " << (tempMap["product"] == modelStr);
        // HW写死
        if (tempMap["product"] == modelStr) {
            // 应HW的要求，将描述固定为   Universal Flash Storage
            tempMap["description"] = "Universal Flash Storage";
            // 应HW的要求，添加interface   UFS 3.0
            tempMap["interface"] = "UFS 3.0";

            // 读取interface版本
            QProcess process;
            process.start("cat /sys/devices/platform/f8200000.ufs/host0/scsi_host/host0/wb_en");
            process.waitForFinished(-1);
            int exitCode = process.exitCode();
            if (exitCode != 127 && exitCode != 126) {
                QString deviceInfo = process.readAllStandardOutput();
                if (deviceInfo.trimmed() == "true") {
                    process.start("cat /sys/block/sdd/device/spec_version");
                    process.waitForFinished(-1);
                    exitCode = process.exitCode();
                    if (exitCode != 127 && exitCode != 126) {
                        deviceInfo = process.readAllStandardOutput();
                        if (deviceInfo.trimmed() == "310") {
                            tempMap["interface"] = "UFS 3.1";
                        }
                    }
                }
            }
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
}
