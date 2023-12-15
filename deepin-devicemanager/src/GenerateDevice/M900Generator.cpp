// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "M900Generator.h"

#include <QFile>
#include <QDir>
#include <QDebug>

// 其它头文件
#include "../DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceMonitor.h"
#include "commontools.h"
#include "EDIDParser.h"

#include <QProcess>
#include <QDir>
#include <QFileInfoList>


M900Generator::M900Generator()
{

}
/*
:/sys/class/net/wlan0$ ls
addr_assign_type  carrier             device    duplex             ifindex    name_assign_type  phys_port_id    proto_down  subsystem     wireless
address           carrier_changes     dev_id    flags              iflink     netdev_group      phys_port_name  queues      tx_queue_len
addr_len          carrier_down_count  dev_port  gro_flush_timeout  link_mode  operstate         phys_switch_id  speed       type
broadcast         carrier_up_count    dormant   ifalias            mtu        phy80211          power           statistics  uevent
:/sys/class/net/wlan0$ ls device
class  device  driver  ieee80211  modalias  net  power  subsystem  uevent  vendor
:/sys/class/net/wlan0$ cat device/vendor
0x024c
uos@uos-PC:/sys/class/net/wlan0$ cat device/device
0xa822
*/

void M900Generator::generatorNetworkDevice()
{
    QStringList ifconfigCardName =  getNetworkInfoFromifconfig();
    const QList<QMap<QString, QString>> lstInfo = DeviceManager::instance()->cmdInfo("lshw_network");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            continue;
        }
        QMap<QString, QString> tempMap = *it;

        QString logicalName = tempMap["logical name"].trimmed();
        if (! ifconfigCardName.contains(logicalName))
            continue;

        if (logicalName.contains("wlan0")) {
            QFile file("/sys/class/net/wlan0/device/vendor");
            if (file.open(QIODevice::ReadOnly)) {

                QString vendorInfo = file.readAll();
                if (vendorInfo.contains("024c", Qt::CaseInsensitive)) {
                    tempMap["vendor"] = "Realtek";
                } else if (vendorInfo.contains("12d1", Qt::CaseInsensitive)) {
                    tempMap["vendor"] = "HiSilicon";
                }
            }
        } else
            tempMap["vendor"] = "HiSilicon";

        DeviceNetwork *device = new DeviceNetwork();
        device->setInfoFromLshw(tempMap);
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        DeviceManager::instance()->addNetworkDevice(device);
    }
}

QStringList M900Generator::getNetworkInfoFromifconfig()
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
            if (line.size() < 2)
                continue;
            if (line.at(0).isEmpty())
                continue;
            else
                ret.append(line.at(0));
        }
    }
    return ret;
}
/*
~$ cat /proc/asound/card0/codec#0
Name: PANGU M900 Intergrated Audio
Vendor: HUAWEI
Model: PANGU M900 Intergrated Audio

uos@uos-PC:~$ cat /sys/firmware/devicetree/base/vendor_info/Name
PANGU M900 Intergrated Audio
uos@uos-PC:~$ cat /sys/firmware/devicetree/base/vendor_info/Vendor
HUAWEI
*/
void M900Generator::getAudioInfoFromCatAudio()  /*this is for PAGU M900 special, conflict to master*/
{
    QMap<QString, QString> tempMap;
    QFile file_vendor("/sys/firmware/devicetree/base/vendor_info/Vendor");
    if (file_vendor.open(QIODevice::ReadOnly)) {
        QString vendorInfo = file_vendor.readAll();
        file_vendor.close();
        if (!vendorInfo.isEmpty()) {
            tempMap["Vendor"] = vendorInfo;
        }
    }

    QFile file_name("/sys/firmware/devicetree/base/vendor_info/Name");
    if (file_name.open(QIODevice::ReadOnly)) {
        QString nameInfo = file_name.readAll();
        file_name.close();
        if (!nameInfo.isEmpty()) {
            tempMap["Name"] = nameInfo;
        }
    }

    if (tempMap.contains("Vendor") && tempMap.contains("Name")) {
        DeviceAudio *device = new DeviceAudio();
        device->setCanEnale(false);
        device->setCanUninstall(false);
        device->setForcedDisplay(true);
        device->setInfoFromCatAudio(tempMap);
        DeviceManager::instance()->addAudioDevice(device);
    }
}

void M900Generator::generatorCpuDevice()   /*this is for PAGU M900 special, conflict to master*/
{
    // 生成CPU
    // get info from lscpu
    const QList<QMap<QString, QString> >  &lsCpu = DeviceManager::instance()->cmdInfo("lscpu");
    QList<QMap<QString, QString> > tmpLst;
    QList<QMap<QString, QString> > srcLst;
    tmpLst.append(lsCpu);
    srcLst.append(lsCpu);

    // 按照processor id 从小到大排序
    if (lsCpu.size() > 1)
        CommonTools::sortCpuInfoByLogicalID(srcLst, tmpLst, 0, lsCpu.size() - 1);

    // get info from lshw
    const QList<QMap<QString, QString> >  &lshwCpu = DeviceManager::instance()->cmdInfo("lshw_cpu");
    QMap<QString, QString> lshw = lshwCpu.size() > 0 ? lshwCpu[0] : QMap<QString, QString>();
    lshw["vendor"] = "HiSilicon";

    // get info from dmidecode -t 4
    const QList<QMap<QString, QString> >  &dmidecode4 = DeviceManager::instance()->cmdInfo("dmidecode4");
    QMap<QString, QString> dmidecode = dmidecode4.size() > 0 ? dmidecode4[0] : QMap<QString, QString>();
    dmidecode["Manufacturer"] = "HiSilicon";

    //  获取逻辑数和core数  获取cpu个数 获取logical个数
    int coreNum = 0, logicalNum = 0;
    const QList<QMap<QString, QString> >  &lsCpu_num = DeviceManager::instance()->cmdInfo("lscpu_num");
    if (lsCpu_num.size() <= 0)
        return;
    const QMap<QString, QString> &map = lsCpu_num[0];
    if (map.find("core") != map.end())
        coreNum = map["core"].toInt();
    if (map.find("logical") != map.end())
        logicalNum = map["logical"].toInt();

    // set cpu number
    QSet<QString> allCPUS;
    for (auto dd4 : dmidecode4) {
        if (dd4.contains("Socket Designation"))
            allCPUS.insert(dd4["Socket Designation"]);
    }
    DeviceManager::instance()->setCpuNum(allCPUS.isEmpty() ? dmidecode4.size() : allCPUS.size());

    // set cpu info
    QList<QMap<QString, QString> >::const_iterator it = srcLst.begin();
    for (; it != srcLst.end(); ++it) {
        DeviceCpu *device = new DeviceCpu;
        device->setCpuInfo(*it, lshw, dmidecode, coreNum, logicalNum);
        DeviceManager::instance()->addCpuDevice(device);
    }

    DeviceManager::instance()->setCpuFrequencyIsCur(false);
}

static void parseEDID(QStringList allEDIDS,QString input)
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

void M900Generator::generatorMonitorDevice()
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