// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "KLVGenerator.h"
#include "DDLog.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>

// 其它头文件
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceImage.h"

using namespace DDLog;

KLVGenerator::KLVGenerator()
{
    qCDebug(appLog) << "KLVGenerator constructor";

}

void KLVGenerator::generatorMonitorDevice()
{
    qCDebug(appLog) << "KLVGenerator::generatorMonitorDevice start";
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "LCD");
//    mapInfo.insert("Vendor", "HUAWEI");
    mapInfo.insert("CurResolution", "2160x1440@60Hz");
    mapInfo.insert("SupportResolution", "2160x1440@60Hz");
    mapInfo.insert("Size", "14 Inch");
//    mapInfo.insert("Date", "2019年7月");

    DeviceMonitor *monitor = new  DeviceMonitor();
    monitor->setInfoFromSelfDefine(mapInfo);
    DeviceManager::instance()->addMonitor(monitor);

    HWGenerator::generatorMonitorDevice();
    qCDebug(appLog) << "KLVGenerator::generatorMonitorDevice end";
}

void KLVGenerator::generatorNetworkDevice()
{
    qCDebug(appLog) << "KLVGenerator::generatorNetworkDevice start";
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("lshw_network");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        // qCDebug(appLog) << "KLVGenerator::generatorNetworkDevice process lshw network info";
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "KLVGenerator::generatorNetworkDevice network info not enough";
            continue;
        }
        QMap<QString, QString> tempMap = *it;
        /*
        capabilities: ethernet physical wireless
        configuration: broadcast=yes ip=10.4.6.115 multicast=yes wireless=IEEE 802.11
        */
        if ((tempMap["configuration"].indexOf("wireless=IEEE 802.11") > -1) ||
                (tempMap["capabilities"].indexOf("wireless") > -1)) {
            // qCDebug(appLog) << "KLVGenerator::generatorNetworkDevice is wireless interface, skip";
            continue;
        }

        DeviceNetwork *device = new DeviceNetwork();
        device->setInfoFromLshw(*it);
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        DeviceManager::instance()->addNetworkDevice(device);
    }
    // HW 要求修改名称,制造商以及类型
    getNetworkInfoFromCatWifiInfo();
    qCDebug(appLog) << "KLVGenerator::generatorNetworkDevice end";
}

void KLVGenerator::getNetworkInfoFromCatWifiInfo()
{
    qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo start";
    QList<QMap<QString, QString> >  lstWifiInfo;
    QString wifiDevicesInfoPath("/sys/hisys/wal/wifi_devices_info");
    QFile file(wifiDevicesInfoPath);
    if (file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo open wifi info file success";
        QMap<QString, QString>  wifiInfo;
        QString allStr = file.readAll();
        file.close();

        // 解析数据
        QStringList items = allStr.split("\n");
        foreach (const QString &item, items) {
            // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo process item:" << item;
            if (item.isEmpty()) {
                // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo item is empty, skip";
                continue;
            }
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QStringList strList = item.split(':', QString::SkipEmptyParts);
#else
            QStringList strList = item.split(':');
#endif
            if (strList.size() == 2) {
                // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo get wifi info:" << strList[0] << strList[1];
                wifiInfo[strList[0] ] = strList[1];
            }
        }

        if (!wifiInfo.isEmpty()) {
            // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo wifi info not empty, add to list";
            lstWifiInfo.append(wifiInfo);
        }
    } else {
        qCWarning(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo open wifi info file failed";
    }
    if (lstWifiInfo.size() == 0) {
        qCWarning(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo no wifi info";
        return;
    }
    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();

    for (; it != lstWifiInfo.end(); ++it) {
        // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo process wifi info";
        if ((*it).size() < 3) {
            // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo wifi info not enough";
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // cat /sys/hisys/wal/wifi_devices_info  获取结果为 HUAWEI HI103
        if (tempMap["Chip Type"].contains("HUAWEI", Qt::CaseInsensitive)) {
            // qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo fix chip type";
            tempMap["Chip Type"] = tempMap["Chip Type"].remove("HUAWEI").trimmed();
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = "HISILICON";
        tempMap["Type"] = "Wireless network";

        DeviceManager::instance()->setNetworkInfoFromWifiInfo(tempMap);
    }
    qCDebug(appLog) << "KLVGenerator::getNetworkInfoFromCatWifiInfo end";
}

void KLVGenerator::getDiskInfoFromLshw()
{
    qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw start";
    QString bootdevicePath("/proc/bootdevice/product_name");
    QString modelStr = "";
    QFile file(bootdevicePath);
    if (file.open(QIODevice::ReadOnly)) {
        qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw open bootdevice file success";
        modelStr = file.readLine().simplified();
        file.close();
    } else {
        qCWarning(appLog) << "KLVGenerator::getDiskInfoFromLshw open bootdevice file failed";
    }

    const QList<QMap<QString, QString>> lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        // qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw process lshw disk info";
        if ((*dIt).size() < 2) {
            // qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw disk info not enough";
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*dIt).keys()) {
            tempMap.insert(key, (*dIt)[key]);
        }
        // qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw disk product:" << tempMap["product"] << "modelStr:" << modelStr;
//        qCInfo(appLog) << tempMap["product"] << " ***** " << modelStr << " " << (tempMap["product"] == modelStr);
        // HW写死
        if (tempMap["product"] == modelStr) {
            // qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw find boot device";
            // 应HW的要求，将描述固定为   Universal Flash Storage
            tempMap["description"] = "Universal Flash Storage";
            // 应HW的要求，添加interface   UFS 3.1
            tempMap["interface"] = "UFS 3.1";
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(tempMap);
    }
    qCDebug(appLog) << "KLVGenerator::getDiskInfoFromLshw end";
}

void KLVGenerator::getImageInfoFromHwinfo()
{
    qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo start";
    //  加载从hwinfo中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo process hwinfo usb info";
        if ((*it).size() < 3) {
            // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo usb info not enough";
            continue;
        }

        // hwinfo中对camera的分类不明确，通过camera等关键字认定图像设备
        if (!(*it)["Model"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Device"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Driver"].contains("uvcvideo", Qt::CaseInsensitive) &&
                !(*it)["Model"].contains("webcam", Qt::CaseInsensitive) &&
                (*it)["Hardware Class"] != "camera") {
            // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo not camera device";
            continue;
        }

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            if ("Vendor" != key)
                tempMap.insert(key, (*it)[key]);
        }

        // 判断该摄像头是否存在，被禁用的和被拔出
        QString path = pciPath(tempMap);
        if (path.contains("usb")) {
            // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo is usb device";
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo authorized file not exists, skip";
                continue;
            }
        }

        // 判断重复设备数据
        QString unique_id = uniqueID(tempMap);
        DeviceImage *device = dynamic_cast<DeviceImage *>(DeviceManager::instance()->getImageDevice(unique_id));
        if (device) {
            // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo device already exists, update it";
            device->setEnableValue(false);
            device->setInfoFromHwinfo(tempMap);
            continue;
        } else {
            if (tempMap.contains("path")) {
                // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo tempMap contains path, skip";
                continue;
            }
        }

        device = new DeviceImage();
        // qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo add image device";
        device->setInfoFromHwinfo(tempMap);
        DeviceManager::instance()->addImageDevice(device);
        addBusIDFromHwinfo(tempMap["SysFS BusID"]);
    }
    qCDebug(appLog) << "KLVGenerator::getImageInfoFromHwinfo end";
}

void KLVGenerator::getImageInfoFromLshw()
{
    qCDebug(appLog) << "KLVGenerator::getImageInfoFromLshw start";
    //  加载从lshw中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        // qCDebug(appLog) << "KLVGenerator::getImageInfoFromLshw process lshw usb info";
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "KLVGenerator::getImageInfoFromLshw usb info not enough";
            continue;
        }

        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            if ("vendor" != key)
                tempMap.insert(key, (*it)[key]);
        }

        DeviceManager::instance()->setCameraInfoFromLshw(tempMap);
        // qCDebug(appLog) << "KLVGenerator::getImageInfoFromLshw set camera info";
    }
    qCDebug(appLog) << "KLVGenerator::getImageInfoFromLshw end";
}

