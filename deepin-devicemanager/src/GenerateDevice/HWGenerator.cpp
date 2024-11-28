// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "HWGenerator.h"
#include "EDIDParser.h"
#include "commonfunction.h"
#include "DDLog.h"

// Qt库文件
#include <QLoggingCategory>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>

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
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceMemory.h"

static QString boardVendorType;
using namespace DDLog;

HWGenerator::HWGenerator()
{
    boardVendorType = Common::boardVendorType();
}

void HWGenerator::generatorComputerDevice()
{
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer *device = new DeviceComputer() ;

    // home url
    if (cmdInfo.size() > 0) {
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        device->setType(sysInfo[0]["description"]);
//        device->setVendor(sysInfo[0]["vendor"]);
        device->setName(sysInfo[0]["product"]);
    }

    const QList<QMap<QString, QString> >  &dmidecode1List = DeviceManager::instance()->cmdInfo("dmidecode1");
    if (!dmidecode1List.isEmpty() && dmidecode1List[0].contains("Product Name") && !dmidecode1List[0]["Product Name"].isEmpty()) {
        device->setName(dmidecode1List[0]["Product Name"]);
    }

    // set Os Description from /etc/os-version

    QString productName = DeviceGenerator::getProductName();
    device->setOsDescription(productName);

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegExp reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)", Qt::CaseSensitive);
        int index = reg.indexIn(info);
        if (index != -1) {
            QString tmp = reg.cap(0);
            info.remove(tmp);
            info.insert(index, reg.cap(1));
        }

        QRegExp replaceReg("\\(.*\\@.*\\)", Qt::CaseSensitive);
        info.replace(replaceReg, "");
        device->setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
}

void HWGenerator::generatorCpuDevice()
{
    DeviceGenerator::generatorCpuDevice();
    DeviceManager::instance()->setCpuFrequencyIsCur(false);
}

void HWGenerator::generatorAudioDevice()
{
    getAudioInfoFromCatAudio();
}

QList<QMap<QString, QString> > readFileSysWifi()
{
    QList<QMap<QString, QString> >  lstWifiInfo;
    QString wifiDevicesInfoPath("/sys/hisys/wal/wifi_devices_info");
    QFile file(wifiDevicesInfoPath);
    if (file.open(QIODevice::ReadOnly)) {
        QMap<QString, QString>  wifiInfo;
        QString allStr = file.readAll();
        file.close();

        // 解析数据
        QStringList items = allStr.split("\n");
        foreach (const QString &item, items) {
            if (item.isEmpty())
                continue;

            QStringList strList = item.split(':', QString::SkipEmptyParts);
            if (strList.size() == 2)
                wifiInfo[strList[0] ] = strList[1];
        }

        if (!wifiInfo.isEmpty())
            lstWifiInfo.append(wifiInfo);
    }
    return lstWifiInfo;
}
void HWGenerator::generatorBluetoothDevice()
{
    getBluetoothInfoFromHciconfig();
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();

    QList<QMap<QString, QString> >  lstWifiInfo = readFileSysWifi();
    if (lstWifiInfo.size() == 0) {
        return;
    }
    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();
    QMap<QString, QString> tempMap;
    for (; it != lstWifiInfo.end(); ++it) {
        if ((*it).size() < 3) {
            continue;
        }

        // KLU的问题特殊处理
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        // cat /sys/hisys/wal/wifi_devices_info  获取结果为  HI103
        if (tempMap["Chip Type"].contains(Common::specialHString(), Qt::CaseInsensitive)) {
            tempMap["Chip Type"] = tempMap["Chip Type"].remove(Common::specialHString()).trimmed();
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = Common::specialHString();
        tempMap["Type"] = "Bluetooth Device";
        tempMap["Name"] = tempMap["Chip Type"];
    }

    QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Bluetoorh);
    for (int i = 0; i < lst.size(); i++) {
        DeviceBaseInfo *device = lst[i];
        QString  bus = DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Bluetoorh, device, "Bus"); //内置：UART 外接USB：USB
        if (bus.contains("UART",Qt::CaseInsensitive)) {
            DeviceManager::instance()->tomlDeviceMapSet(DT_Bluetoorh, device,tempMap);
        }
    }
}

void HWGenerator::generatorGpuDevice()
{
    QProcess process;
    process.start("gpuinfo");
    process.waitForFinished(-1);
    int exitCode = process.exitCode();
    if (exitCode == 127 || exitCode == 126) {
        return;
    }

    QString deviceInfo = process.readAllStandardOutput();
    deviceInfo.replace("：", ":");
    QStringList items = deviceInfo.split("\n");

    QMap<QString, QString> mapInfo;
    for (QString itemStr : items) {
        if (itemStr.contains(":"))
            continue;
        QString curItemStr = itemStr.trimmed();
        if (!curItemStr.isEmpty()) {
            mapInfo.insert("Name", curItemStr);
            break;
        }
    }

    for (int i = 1; i < items.size(); ++i) {
        QStringList words = items[i].split(":");
        if (words.size() == 2) {
            mapInfo.insert(words[0].trimmed(), words[1].trimmed());
        }
    }
    if (mapInfo.size() < 2)
        return;

    DeviceGpu *device = new DeviceGpu();
    device->setCanUninstall(false);
    device->setForcedDisplay(true);
    device->setGpuInfo(mapInfo);
    DeviceManager::instance()->addGpuDevice(device);
}


void HWGenerator::generatorNetworkDevice()
{
    DeviceGenerator::generatorNetworkDevice();

    QList<QMap<QString, QString> >  lstWifiInfo = readFileSysWifi();
    if (lstWifiInfo.size() == 0) {
        return;
    }

    QList<QMap<QString, QString> >::const_iterator it = lstWifiInfo.begin();
    for (; it != lstWifiInfo.end(); ++it) {
        if ((*it).size() < 3) {
            continue;
        }

        // cat /sys/hisys/wal/wifi_devices_info的问题特殊处理       获取结果为  HI103
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }

        tempMap["Name"] = tempMap["Chip Type"];
        if (tempMap["Chip Type"].contains(Common::specialHString(), Qt::CaseInsensitive)) {
            tempMap["Name"] = tempMap["Chip Type"].remove(Common::specialHString()).trimmed();
        }

        if (tempMap["NIC  Type"].contains("WLAN", Qt::CaseInsensitive)) {
        }

        // 按照华为的需求，设置蓝牙制造商和类型
        tempMap["Vendor"] = "HISILICON";
        tempMap["Type"] = "Wireless network";

        QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Network);
        for (int i = 0; i < lst.size(); i++) {
            DeviceBaseInfo *device = lst[i];
            QString  logicalName = DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Network, device, "Logical Name");
            if (logicalName.contains("wlan", Qt::CaseInsensitive)) {
                DeviceManager::instance()->tomlDeviceMapSet(DT_Network, device,tempMap);
            }
        }
    }
}

void HWGenerator::generatorDiskDevice()
{
    DeviceGenerator::generatorDiskDevice();

    QString bootdevicePath("/proc/bootdevice/product_name");
    QString modelStr = "";
    QFile file(bootdevicePath);
    if (file.open(QIODevice::ReadOnly)) {
        modelStr = file.readLine().simplified();
        file.close();
    }
    if (modelStr.isEmpty())
        return;

    QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Storage);
    for (int i = 0; i < lst.size(); i++) {
        DeviceBaseInfo *device = lst[i];
        QString  name = DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Storage, device, "Name");
        QString  vendor = DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Storage, device, "Vendor");
        if (name == modelStr) {
            QMap<QString, QString> tempMap;
            if (vendor.contains("WDC",Qt::CaseInsensitive))
                tempMap["Vendor"] = "nouse";
            if (name.contains("SDINFDO4-256G",Qt::CaseInsensitive))
                tempMap["Name"] = "nouse";
            // 应HW的要求，将描述固定为   Universal Flash Storage
            tempMap["Description"] = "Universal Flash Storage";
            // 应H的要求，添加interface   UFS 3.1
            tempMap["Interface"] = "UFS 3.1";

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
                        } else if  (deviceInfo.trimmed() == "300")
                            tempMap["interface"] = "UFS 3.0";
                    }
                }
            }

            DeviceManager::instance()->tomlDeviceMapSet(DT_Storage, device,tempMap);
        }
    }

    DeviceManager::instance()->checkDiskSize(); //place in the end
}

void HWGenerator::getAudioInfoFromCatAudio()
{
    const QList<QMap<QString, QString>> lstAudio = DeviceManager::instance()->cmdInfo("cat_audio");
    QList<QMap<QString, QString> >::const_iterator it = lstAudio.begin();
    for (; it != lstAudio.end(); ++it) {
        if ((*it).size() < 2)
            continue;

        QMap<QString, QString> tempMap = *it;
        if (tempMap["Name"].contains("da_combine_v5", Qt::CaseInsensitive)) {
            tempMap["Name"] = "OnBoard Audio";
            tempMap["Model"] = "OnBoard Audio";
        }

        if (tempMap.contains("Vendor")) {
            tempMap["Vendor"]= Common::specialHString();
        }

        DeviceAudio *device = new DeviceAudio();
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        device->setInfoFromCatAudio(tempMap);
        DeviceManager::instance()->addAudioDevice(device);
    }
}

void HWGenerator::getBluetoothInfoFromHciconfig()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hciconfig");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceBluetooth *device = new DeviceBluetooth();
        device->setCanEnale(false);
        device->setForcedDisplay(true);
        device->setInfoFromHciconfig(*it);
        DeviceManager::instance()->addBluetoothDevice(device);
    }
}

void HWGenerator::getBlueToothInfoFromHwinfo()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            continue;
        }
        if ((*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0") {
            if (DeviceManager::instance()->setBluetoothInfoFromHwinfo(*it))
                addBusIDFromHwinfo((*it)["SysFS BusID"]);
        }
    }
}

void HWGenerator::getBluetoothInfoFromLshw()
{
    const QList<QMap<QString, QString>> lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            continue;
        }
        DeviceManager::instance()->setBluetoothInfoFromLshw(*it);
    }
}

void HWGenerator::getMemoryInfoFromLshw()
{
    // 从lshw中获取内存信息
    const QList<QMap<QString, QString>> &lstMemory = DeviceManager::instance()->cmdInfo("lshw_memory");
    QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();

    for (; it != lstMemory.end(); ++it) {

        // bug47194 size属性包含MiB
        // 目前处理内存信息时，bank下一定要显示内存信息，否则无法生成内存
        if (!(*it)["size"].contains("GiB") && !(*it)["size"].contains("MiB"))
            continue;

        // KLU的问题特殊处理
        QMap<QString, QString> tempMap;
        foreach (const QString &key, (*it).keys()) {
            tempMap.insert(key, (*it)[key]);
        }
        if (tempMap.contains("clock")) {
            tempMap.remove("clock");
        }

        DeviceMemory *device = new DeviceMemory();
        device->setInfoFromLshw(tempMap);
        DeviceManager::instance()->addMemoryDevice(device);
    }
}

static void parseEDID(QStringList allEDIDS,QString input)
{
    for (auto edid:allEDIDS) {
        QProcess process;
        process.start(QString("hexdump %1").arg(edid));
        process.waitForFinished(-1);

        QString deviceInfo = process.readAllStandardOutput();
        if (deviceInfo.isEmpty())
            continue;

        QString edidStr;
        QStringList lines = deviceInfo.split("\n");
        for (auto line:lines) {
            QStringList words = line.trimmed().split(" ");
            if (words.size() != 9)
                continue;

            words.removeAt(0);
            QString l = words.join("");
            l.append("\n");
            edidStr.append(l);
        }

        lines = edidStr.split("\n");
        if (lines.size() > 3){
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

void HWGenerator::generatorMonitorDevice()
{
    QMap<QString, QString> mapInfo;
    mapInfo.insert("Name", "LCD");
    mapInfo.insert("Current Resolution", "2160x1440");
    mapInfo.insert("Support Resolution", "2160x1440");
    mapInfo.insert("Size", "14 Inch");
    mapInfo.insert("Interface Type", "eDP");
    mapInfo.insert("Refresh Rate", "60Hz");

    DeviceMonitor *monitor = new  DeviceMonitor();
    monitor->setInfoFromTomlBase(mapInfo);
    monitor->setInfoFromTomlOneByOne(mapInfo);
    DeviceManager::instance()->addMonitor(monitor);

    QString toDir = "/sys/class/drm";
    QDir toDir_(toDir);

    if (!toDir_.exists())
        return;

    QFileInfoList fileInfoList = toDir_.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if (fileInfo.fileName() == "." || fileInfo.fileName() == ".." || !fileInfo.fileName().startsWith("card"))
            continue;

        if (QFile::exists(fileInfo.filePath() + "/" + "edid")) {
            QStringList allEDIDS_all;
            allEDIDS_all.append(fileInfo.filePath() + "/" + "edid");
            QString interface = fileInfo.fileName().remove("card0-").remove("card1-").remove("card2-");
            parseEDID(allEDIDS_all,interface);
         }
    }
}

void HWGenerator::generatorPowerDevice()
{
    DeviceGenerator::generatorPowerDevice();
}

void HWGenerator::generatorKeyboardDevice()
{
    DeviceGenerator::generatorKeyboardDevice();
}

void HWGenerator::generatorMemoryDevice()
{
    DeviceGenerator::generatorMemoryDevice();
}

void HWGenerator::generatorCameraDevice()
{
    DeviceGenerator::generatorCameraDevice();

    QList<DeviceBaseInfo *> lst = DeviceManager::instance()->convertDeviceList(DT_Image);
    for (int i = 0; i < lst.size(); i++) {
        DeviceBaseInfo *device = lst[i];
        QString  vendor = DeviceManager::instance()->tomlDeviceReadKeyValue(DT_Image, device, "Vendor");
        if (vendor.contains("0000058020",Qt::CaseInsensitive)) {
            QMap<QString, QString> tempMap;
            tempMap["Vendor"] = "nouse";
            DeviceManager::instance()->tomlDeviceMapSet(DT_Image, device,tempMap);
        }
    }
}
