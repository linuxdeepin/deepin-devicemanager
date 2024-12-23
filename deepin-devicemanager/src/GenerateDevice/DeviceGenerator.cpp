// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceGenerator.h"
#include "DDLog.h"
#include "commondefine.h"
// 其它头文件
#include "CmdTool.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceCpu.h"
#include "DeviceManager/DeviceGpu.h"
#include "DeviceManager/DeviceStorage.h"
#include "DeviceManager/DeviceMemory.h"
#include "DeviceManager/DeviceMonitor.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceAudio.h"
#include "DeviceManager/DeviceBluetooth.h"
#include "DeviceManager/DeviceNetwork.h"
#include "DeviceManager/DeviceImage.h"
#include "DeviceManager/DeviceOthers.h"
#include "DeviceManager/DeviceComputer.h"
#include "DeviceManager/DevicePower.h"
#include "DeviceManager/DeviceCdrom.h"
#include "DeviceManager/DevicePrint.h"
#include "DeviceManager/DeviceInput.h"
#include "MacroDefinition.h"

// Dtk头文件
#include <DSysInfo>

// Qt库文件
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QDir>

using namespace DDLog;

DeviceGenerator::DeviceGenerator(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "DeviceGenerator constructor";
}

DeviceGenerator::~DeviceGenerator()
{
    qCDebug(appLog) << "DeviceGenerator destructor";

}

void DeviceGenerator::generatorComputerDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice start";
    const QList<QMap<QString, QString> >  &cmdInfo = DeviceManager::instance()->cmdInfo("cat_os_release");
    DeviceComputer *device = new DeviceComputer() ;

    // home url
    if (cmdInfo.size() > 0) {
        qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice get home url";
        QString value = cmdInfo[0]["HOME_URL"];
        device->setHomeUrl(value.replace("\"", ""));
    }

    // name type
    const QList<QMap<QString, QString> >  &sysInfo = DeviceManager::instance()->cmdInfo("lshw_system");
    if (sysInfo.size() > 0) {
        qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice get system info";
        device->setType(sysInfo[0]["description"]);
        device->setVendor(sysInfo[0]["vendor"]);
        device->setName(sysInfo[0]["product"]);
    }

    // set Os Description from /etc/os-version

    QString productName = DeviceGenerator::getProductName();
    device->setOsDescription(productName);
    qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice get product name" << productName;

    // os
    const QList<QMap<QString, QString> >  &verInfo = DeviceManager::instance()->cmdInfo("cat_version");
    if (verInfo.size() > 0) {
        qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice get os version";
        QString info = verInfo[0]["OS"].trimmed();
        info = info.trimmed();
        QRegularExpression reg("\\(gcc [\\s\\S]*(\\([\\s\\S]*\\))\\)");
        QRegularExpressionMatch match = reg.match(info);
        if (match.hasMatch()) {
            qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice match os version";
            QString tmp = match.captured(0);
            info.remove(tmp);
            info.insert(match.capturedStart(), match.captured(1));
        }
        device->setOS(info);
    }
    DeviceManager::instance()->addComputerDevice(device);
    qCDebug(appLog) << "DeviceGenerator::generatorComputerDevice end";
}

void mergeSortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu, QList<QMap<QString, QString> > &tmpLst, int begin, int end)
{
    qCDebug(appLog) << "mergeSortCpuInfoByLogicalID start, begin:" << begin << "end:" << end;
    // 合并列表
    int left_length = (end - begin + 1) / 2;
    int left_index = begin;
    int right_index = begin + left_length;
    int result_index = begin;

    // 合并左右区间 左区间未合并结束且右区间未合并结束时
    while (left_index < begin + left_length && right_index < end + 1) {
        // 左右区间,哪个小先排哪个,下标加1
        if (lsCpu[left_index]["processor"].toInt() <= lsCpu[right_index]["processor"].toInt()) {
            // qCDebug(appLog) << "mergeSortCpuInfoByLogicalID left <= right";
            tmpLst[result_index++] = lsCpu[left_index++];
        } else {
            // qCDebug(appLog) << "mergeSortCpuInfoByLogicalID left > right";
            tmpLst[result_index++] = lsCpu[right_index++];
        }
    }

    // 合并左区间剩余数据
    while (left_index < begin + left_length) {
        // qCDebug(appLog) << "mergeSortCpuInfoByLogicalID merge left";
        tmpLst[result_index++] = lsCpu[left_index++];
    }

    // 合并右区间剩余数据
    while (right_index < end + 1) {
        // qCDebug(appLog) << "mergeSortCpuInfoByLogicalID merge right";
        tmpLst[result_index++] = lsCpu[right_index++];
    }
    qCDebug(appLog) << "mergeSortCpuInfoByLogicalID end";
}

void sortCpuInfoByLogicalID(QList<QMap<QString, QString> > &lsCpu, QList<QMap<QString, QString> > &tmpLst, int begin, int end)
{
    qCDebug(appLog) << "sortCpuInfoByLogicalID start, begin:" << begin << "end:" << end;
    // 列表个数为1,直接返回
    if (0 == end - begin) {
        qCDebug(appLog) << "sortCpuInfoByLogicalID only one element";
        return;
    }

    // bug 后台获取CPU信息是按照物理CPU,核心,逻辑CPU顺序获取的
    // 界面上展示顺序混乱实际是按照物理CPU,核心,逻辑CPU顺序展示
    // 与产品沟通后,按照用户的使用感修改,CPU信息按照逻辑CPU的id从小到大显示
    // 区间个数为2
    if (1 == end - begin) {
        qCDebug(appLog) << "sortCpuInfoByLogicalID two elements";
        // 前 processor > 后 processor 时交换位置
        if (lsCpu[begin]["processor"].toInt() > lsCpu[end]["processor"].toInt()) {
            qCDebug(appLog) << "sortCpuInfoByLogicalID swap two elements";
            QMap<QString, QString> tmpMap = lsCpu[begin];
            lsCpu[begin] = lsCpu[end];
            lsCpu[end] = tmpMap;
        }
    } else {
        qCDebug(appLog) << "sortCpuInfoByLogicalID more than two elements, recurse";
        // 区间个数 > 2 递归
        sortCpuInfoByLogicalID(lsCpu, tmpLst, begin, (end - begin) / 2 + begin);
        sortCpuInfoByLogicalID(lsCpu, tmpLst, (end - begin + 1) / 2 + begin, end);
        mergeSortCpuInfoByLogicalID(lsCpu, tmpLst, begin, end);
        lsCpu = tmpLst;
    }
    qCDebug(appLog) << "sortCpuInfoByLogicalID end";
}

void DeviceGenerator::generatorCpuDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice start";
    // 生成CPU
    // get info from lscpu
    const QList<QMap<QString, QString> >  &lsCpu = DeviceManager::instance()->cmdInfo("lscpu");
    QList<QMap<QString, QString> > tmpLst;
    QList<QMap<QString, QString> > srcLst;
    tmpLst.append(lsCpu);
    srcLst.append(lsCpu);

    // 按照processor id 从小到大排序
    if (lsCpu.size() > 1)
        sortCpuInfoByLogicalID(srcLst, tmpLst, 0, lsCpu.size() - 1);

    // get info from lshw
    const QList<QMap<QString, QString> >  &lshwCpu = DeviceManager::instance()->cmdInfo("lshw_cpu");
    const QMap<QString, QString> &lshw = lshwCpu.size() > 0 ? lshwCpu[0] : QMap<QString, QString>();

    // get info from dmidecode -t 4
    const QList<QMap<QString, QString> >  &dmidecode4 = DeviceManager::instance()->cmdInfo("dmidecode4");
    const QMap<QString, QString> &dmidecode = dmidecode4.size() > 0 ? dmidecode4[0] : QMap<QString, QString>();


    //  获取逻辑数和core数  获取cpu个数 获取logical个数
    int coreNum = 0, coreNum_dmi = 0, logicalNum = 0, logicalNum_dmi = 0;
    const QList<QMap<QString, QString> >  &lsCpu_num = DeviceManager::instance()->cmdInfo("lscpu_num");
    if (lsCpu_num.size() <= 0) {
        qCWarning(appLog) << "DeviceGenerator::generatorCpuDevice get lscpu_num failed";
        return;
    }
    const QMap<QString, QString> &map = lsCpu_num[0];
    if (map.find("core") != map.end()) {
        qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice get core num";
        coreNum = map["core"].toInt();
    }
    if (map.find("logical") != map.end()) {
        qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice get logical num";
        logicalNum = map["logical"].toInt();
    }

    // set cpu number
    QSet<QString> allCPUS;
    for (auto dd4 : dmidecode4) {
        if (dd4.contains("Socket Designation")) {
            // qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice get socket designation";
            allCPUS.insert(dd4["Socket Designation"]);
        }
        if (dd4.contains("Core Count")) {
            // qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice get core count";
            coreNum_dmi += dd4["Core Count"].toInt();
        }
        if (dd4.contains("Thread Count")) {
            // qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice get thread count";
            logicalNum_dmi += dd4["Thread Count"].toInt();
        }
    }
    if(logicalNum_dmi > logicalNum && logicalNum_dmi < 1024)  //due to offline policy
        logicalNum = logicalNum_dmi;
    if(coreNum_dmi > coreNum && coreNum_dmi <= 512) //due to offline policy
        coreNum = coreNum_dmi;

    DeviceManager::instance()->setCpuNum(allCPUS.isEmpty() ? dmidecode4.size() : allCPUS.size());
    qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice set cpu num" << (allCPUS.isEmpty() ? dmidecode4.size() : allCPUS.size());

    // set cpu info
    QList<QMap<QString, QString> >::const_iterator it = srcLst.begin();
    for (; it != srcLst.end(); ++it) {
        qCDebug(appLog) << "DeviceGenerator::generatorCpuDevice add cpu device";
        DeviceCpu *device = new DeviceCpu;
        device->setCpuInfo(*it, lshw, dmidecode, coreNum, logicalNum);
        DeviceManager::instance()->addCpuDevice(device);
    }
}

void DeviceGenerator::generatorBiosDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorBiosDevice start";
    // 生成BIOS
    getBiosInfo();
    getSystemInfo();
    getBaseBoardInfo();
    getChassisInfo();
    getBiosMemoryInfo();
    qCDebug(appLog) << "DeviceGenerator::generatorBiosDevice end";
}

void DeviceGenerator::generatorMemoryDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorMemoryDevice start";
    // 生成内存
    getMemoryInfoFromLshw();
    getMemoryInfoFromDmidecode();
    qCDebug(appLog) << "DeviceGenerator::generatorMemoryDevice end";
}

void DeviceGenerator::generatorDiskDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorDiskDevice start";
    // 生成存储设备
    // 添加从hwinfo中获取的信息
    getDiskInfoFromHwinfo();
    // 添加从lshw中获取的信息
    getDiskInfoFromLshw();

    getDiskInfoFromLsblk();
    getDiskInfoFromSmartCtl();
    DeviceManager::instance()->mergeDisk();
    qCDebug(appLog) << "DeviceGenerator::generatorDiskDevice end";
}

void DeviceGenerator::generatorGpuDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorGpuDevice start";
    // 生成显示适配器
    getGpuInfoFromHwinfo();
    getGpuInfoFromLshw();
    getGpuSizeFromDmesg();
    qCDebug(appLog) << "DeviceGenerator::generatorGpuDevice end";
}

void DeviceGenerator::generatorMonitorDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorMonitorDevice start";
    // 生成显示设备
    getMonitorInfoFromHwinfo();
    qCDebug(appLog) << "DeviceGenerator::generatorMonitorDevice end";
}

bool isValidLogicalName(const QString& logicalName)
{
    if (logicalName.contains("p2p", Qt::CaseInsensitive) || logicalName.isEmpty())
        return false;

    QString addressFilePath = "/sys/class/net/" + logicalName;
    QDir dir(addressFilePath);
    if (dir.exists())
        return true;

    qCInfo(appLog) << dir << "not exist in /sys/class/net/";
    return false;
}

bool isValidMAC(const QString& macAddress)
{
    if (macAddress.contains("00:00:00:00:00:00", Qt::CaseInsensitive) || macAddress.contains("ff:ff:ff:ff:ff:ff", Qt::CaseInsensitive) || macAddress.isEmpty())
        return false;

    QRegularExpression macRegex("([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})");
    if (!macRegex.match(macAddress).hasMatch())
        return false;

    return true;
}

void DeviceGenerator::generatorNetworkDevice()
{
    bool hasWlan =false;
    QList<DeviceNetwork *> lstDevice;

    // 设置从lshw中获取的信息
    const QList<QMap<QString, QString>> &lstLshw = DeviceManager::instance()->cmdInfo("lshw_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstLshw.begin(); it != lstLshw.end(); ++it) {
        if ((*it).find("serial") == (*it).end() || (*it).find("logical name") == (*it).end())
            continue;

        const QString &logicalNameLshw =  (*it)["logical name"];
        for (QList<DeviceNetwork *>::iterator itDevice = lstDevice.begin(); itDevice != lstDevice.end(); ++itDevice) {
            const QString &macAddressLshw =  (*it)["serial"];
            if (!isValidLogicalName(logicalNameLshw))
                continue;
            if (!isValidMAC(macAddressLshw))
                continue;
            if (logicalNameLshw.contains("wlan", Qt::CaseInsensitive) && hasWlan) //common sense: one PC only have 1 wlan device
                continue;

            DeviceNetwork *device = new DeviceNetwork();
            device->setInfoFromLshw(*it);
            lstDevice.append(device);
            if (logicalNameLshw.contains("wlan", Qt::CaseInsensitive))
                hasWlan = true;
        }
    }

    const QList<QMap<QString, QString>> &lstHWInfo = DeviceManager::instance()->cmdInfo("hwinfo_network");
    for (QList<QMap<QString, QString> >::const_iterator it = lstHWInfo.begin(); it != lstHWInfo.end(); ++it) {

        // 先判断是否是有效网卡信息
        // 符合两种情况中的一种 1. "HW Address" 和 "Permanent HW Address" 都必须有  2. 有 "unique_id"
        if (((*it).find("HW Address") == (*it).end() && (*it).find("Permanent HW Address") == (*it).end()) && ((*it).find("unique_id") == (*it).end())) {
            continue;
        }

        const QString &macHwinfo =  (*it)["Permanent HW Address"].isEmpty() ? (*it)["HW Address"] : (*it)["Permanent HW Address"];
        const QString &logicalNameHwinfo =  (*it)["Device File"];
        bool hasMatchLogicalName = false;
        for (QList<DeviceNetwork *>::iterator itDevice = lstDevice.begin(); itDevice != lstDevice.end(); ++itDevice) {
            const QString &serialNumberLst =  (*itDevice)->hwAddress();
            const QString &logicalNameLst =  (*itDevice)->logicalName();
            // 如果(*it)中包含unique_id属性，则说明是从数据库里面获取的，否则是从hwinfo中获取的
            if ((*it).find("unique_id") != (*it).end()) {
                const QString &unique_id = (*it)["unique_id"];
                if (!unique_id.isEmpty() && (*itDevice)->uniqueID() == unique_id) {
                    (*itDevice)->setEnableValue(false);
                }

            } else if (macHwinfo == serialNumberLst || logicalNameHwinfo == logicalNameLst) {
                (*itDevice)->setInfoFromHwinfo(*it);
                hasMatchLogicalName = true;
            } else {
                (*itDevice)->setCanUninstall(false);
                (*itDevice)->setForcedDisplay(true);
            }
        }
        //first check it's valid, if LogicalName not exist  but valid please add it .
        if (!hasMatchLogicalName && isValidMAC(macHwinfo) && isValidLogicalName(logicalNameHwinfo)) {
            if (!logicalNameHwinfo.contains("wlan", Qt::CaseInsensitive)) {//add wired net
                DeviceNetwork *device = new DeviceNetwork();
                device->setInfoFromHwinfo(*it);
                lstDevice.append(device);
                hasMatchLogicalName = true;
            } else {    //add wireless net
                if (!hasWlan) { //common sense: one PC only have 1 wlan device
                    DeviceNetwork *device = new DeviceNetwork();
                    device->setInfoFromHwinfo(*it);
                    lstDevice.append(device);
                    hasMatchLogicalName = true;
                    hasWlan = true;
                }
            }
        }
    }

    foreach (DeviceNetwork *device, lstDevice) {
        DeviceManager::instance()->addNetworkDevice(device);
    }
}


void DeviceGenerator::generatorAudioDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorAudioDevice start";
    // 生成音频适配器
    getAudioInfoFromHwinfo();
    // getAudioChipInfoFromDmesg();  //将两个设备芯片显示为一个，该功能代码逻辑错误  被 getAudioInfoFrom_sysFS 取代
    getAudioInfoFromLshw();
    getAudioInfoFrom_sysFS();
    qCDebug(appLog) << "DeviceGenerator::generatorAudioDevice end";
}

void DeviceGenerator::generatorBluetoothDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorBluetoothDevice start";
    // 生成蓝牙
    getBlueToothInfoFromHwinfo();
    getBluetoothInfoFromLshw();
    getBluetoothInfoFromHciconfig();
    qCDebug(appLog) << "DeviceGenerator::generatorBluetoothDevice end";
}

void DeviceGenerator::generatorKeyboardDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorKeyboardDevice start";
    // 生成键盘
    getKeyboardInfoFromHwinfo();
    getKeyboardInfoFromLshw();
    qCDebug(appLog) << "DeviceGenerator::generatorKeyboardDevice end";
}

void DeviceGenerator::generatorMouseDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorMouseDevice start";
    // 生成鼠标
    getMouseInfoFromHwinfo();
    getMouseInfoFromLshw();
    qCDebug(appLog) << "DeviceGenerator::generatorMouseDevice end";
}

void DeviceGenerator::generatorPrinterDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorPrinterDevice start";
    // 生成打印机
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("printer");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping printer with insufficient info";
            continue;
        }

        DevicePrint *device = new DevicePrint() ;
        device->setInfo(*it);
        device->setHardwareClass("printer");
        DeviceManager::instance()->addPrintDevice(device);
        // qCDebug(appLog) << "Added printer device:" << device->name();
    }
    qCDebug(appLog) << "DeviceGenerator::generatorPrinterDevice end";
}

void DeviceGenerator::generatorCameraDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorCameraDevice start";
    // 生成图像设备
    getImageInfoFromHwinfo();
    getImageInfoFromLshw();
    qCDebug(appLog) << "DeviceGenerator::generatorCameraDevice end";
}

void DeviceGenerator::generatorCdromDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorCdromDevice start";
    // 生成光盘
    getCdromInfoFromHwinfo();
    getCdromInfoFromLshw();
    qCDebug(appLog) << "DeviceGenerator::generatorCdromDevice end";
}

void DeviceGenerator::generatorOthersDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorOthersDevice start";
    // 生成其他设备
    getOthersInfoFromHwinfo();
    getOthersInfoFromLshw();
    qCDebug(appLog) << "DeviceGenerator::generatorOthersDevice end";
}

void DeviceGenerator::generatorPowerDevice()
{
    qCDebug(appLog) << "DeviceGenerator::generatorPowerDevice start";
    // 生成电池
    const QList<QMap<QString, QString> > &daemon = DeviceManager::instance()->cmdInfo("Daemon");
    bool hasDaemon = false;
    // 守护进程信息
    if (daemon.size() > 0)
        hasDaemon = true;

    // 电池信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("upower");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping power device with insufficient info";
            continue;
        }

        DevicePower *device = new DevicePower();
        if (!device->setInfoFromUpower(*it)) {
            // qCDebug(appLog) << "Failed to set power info from upower, deleting device";
            continue;
        }

        // 设置守护进程信息
        if (hasDaemon)
            device->setDaemonInfo(daemon[0]);

        DeviceManager::instance()->addPowerDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::generatorPowerDevice end";
}

void DeviceGenerator::getBiosInfo()
{
    qCDebug(appLog) << "DeviceGenerator::getBiosInfo start";
    // 获取BIOS 信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode0");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping BIOS info with insufficient data";
            continue;
        }

        DeviceBios *device = new DeviceBios();
        device->setBiosInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }

    const QList<QMap<QString, QString>> &lanInfo = DeviceManager::instance()->cmdInfo("dmidecode13");
    QList<QMap<QString, QString> >::const_iterator iter = lanInfo.begin();
    for (; iter != lanInfo.end(); ++iter) {
        if ((*iter).size() < 2) {
            // qCDebug(appLog) << "Skipping language info with insufficient data";
            continue;
        }

        DeviceManager::instance()->setLanguageInfo(*iter);
    }

    qCDebug(appLog) << "DeviceGenerator::getBiosInfo end";
}

void DeviceGenerator::getSystemInfo()
{
    qCDebug(appLog) << "DeviceGenerator::getSystemInfo start";
    // 获取系统信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode1");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping system info with insufficient data";
            continue;
        }

        DeviceBios *device = new DeviceBios();
        device->setSystemInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::getSystemInfo end";
}

void DeviceGenerator::getBaseBoardInfo()
{
    qCDebug(appLog) << "DeviceGenerator::getBaseBoardInfo start";
    // 获取主板信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode2");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping baseboard info with insufficient data";
            continue;
        }

        DeviceBios *device = new DeviceBios();
        device->setBaseBoardInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::getBaseBoardInfo end";
}

void DeviceGenerator::getChassisInfo()
{
    qCDebug(appLog) << "DeviceGenerator::getChassisInfo start";
    // 获取机箱信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode3");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping chassis info with insufficient data";
            continue;
        }

        DeviceBios *device = new DeviceBios();
        device->setChassisInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::getChassisInfo end";
}

void DeviceGenerator::getBiosMemoryInfo()
{
    qCDebug(appLog) << "DeviceGenerator::getBiosMemoryInfo start";
    // 获取内存插槽信息
    const QList<QMap<QString, QString>> &lstInfo = DeviceManager::instance()->cmdInfo("dmidecode16");
    QList<QMap<QString, QString> >::const_iterator it = lstInfo.begin();
    for (; it != lstInfo.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping BIOS memory info with insufficient data";
            continue;
        }

        DeviceBios *device = new DeviceBios();
        device->setMemoryInfo(*it);
        DeviceManager::instance()->addBiosDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::getBiosMemoryInfo end";
}

void DeviceGenerator::getMemoryInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getMemoryInfoFromLshw start";
    // 从lshw中获取内存信息
    const QList<QMap<QString, QString>> &lstMemory = DeviceManager::instance()->cmdInfo("lshw_memory");
    QList<QMap<QString, QString> >::const_iterator it = lstMemory.begin();

    for (; it != lstMemory.end(); ++it) {
        if ((*it)["description"].contains("System Memory") && (*it).find("vendor") == (*it).end()) {
            // qCDebug(appLog) << "Skipping system memory entry without vendor";
            continue;
        }
        // bug47194 size属性包含MiB
        // 目前处理内存信息时，bank下一定要显示内存信息，否则无法生成内存
        if (!(*it)["size"].contains("GiB") && !(*it)["size"].contains("MiB")) {
            // qCDebug(appLog) << "Skipping memory entry without proper size unit";
            continue;
        }

        DeviceMemory *device = new DeviceMemory();
        device->setInfoFromLshw(*it);
        DeviceManager::instance()->addMemoryDevice(device);
    }
    qCDebug(appLog) << "DeviceGenerator::getMemoryInfoFromLshw end";
}

void DeviceGenerator::getMemoryInfoFromDmidecode()
{
    qCDebug(appLog) << "DeviceGenerator::getMemoryInfoFromDmidecode start";
    // 加载从dmidecode获取的内存信息
    const QList<QMap<QString, QString>> &dmiMemory = DeviceManager::instance()->cmdInfo("dmidecode17");
    QList<QMap<QString, QString> >::const_iterator dIt = dmiMemory.begin();
    for (; dIt != dmiMemory.end(); ++dIt) {
        if ((*dIt).size() < 2 || (*dIt)["size"] == "No Module Installed") {
            // qCDebug(appLog) << "Skipping empty or not installed memory module from dmidecode";
            continue;
        }

        DeviceManager::instance()->setMemoryInfoFromDmidecode(*dIt);
    }
    qCDebug(appLog) << "DeviceGenerator::getMemoryInfoFromDmidecode end";
}

void DeviceGenerator::getDiskInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromHwinfo start";
    // 从hwinfo中获取的存储设备信息
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("hwinfo_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            // qCDebug(appLog) << "Skipping disk from hwinfo with insufficient data";
            continue;
        }

        DeviceStorage *device = new DeviceStorage();
        if (device->setHwinfoInfo(*dIt) && device->isValid()) {
            DeviceManager::instance()->addStorageDeivce(device);
        } else {
            delete device;
            device = nullptr;
        }

        // Add all disk including no size disk to filter list
        if ((*dIt).find("SysFS BusID") != (*dIt).end())
            addBusIDFromHwinfo((*dIt)["SysFS BusID"]);
    }
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromHwinfo end";
}

void DeviceGenerator::getDiskInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromLshw start";
    // 从lshw中获取的存储设备信息
    // lshw -C disk
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("lshw_disk");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            // qCDebug(appLog) << "Skipping disk from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->addLshwinfoIntoStorageDevice(*dIt);
    }

    // lshw -C storage
    const QList<QMap<QString, QString>> &lstStorage = DeviceManager::instance()->cmdInfo("lshw_storage");
    QList<QMap<QString, QString> >::const_iterator sIt = lstStorage.begin();
    for (; sIt != lstStorage.end(); ++sIt) {
        if ((*sIt).size() < 2) {
            // qCDebug(appLog) << "Skipping storage from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->addLshwinfoIntoNVMEStorageDevice(*sIt);
    }
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromLshw end";
}

void DeviceGenerator::getDiskInfoFromLsblk()
{
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromLsblk start";
    // 通过lsblk信息设备存储设备介质类型
    const QList<QMap<QString, QString>> &lstblk = DeviceManager::instance()->cmdInfo("lsblk_d");
    if (lstblk.size() > 0) {
        QMap<QString, QString>::const_iterator it = lstblk[0].begin();
        for (; it != lstblk[0].end(); ++it) {
            // qCDebug(appLog) << "Setting storage media type for" << it.key() << "to" << it.value();
            DeviceManager::instance()->setStorageDeviceMediaType(it.key(), it.value());
        }
    }
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromLsblk end";
}

void DeviceGenerator::getDiskInfoFromSmartCtl()
{
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromSmartCtl start";
    // 加载从smartctl中获取的存储设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("smart");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        // 剔除未识别的磁盘
        if (!(*it).contains("ln"))
            continue;
        // qCDebug(appLog) << "Setting storage info from smartctl for" << (*it)["ln"];
        DeviceManager::instance()->setStorageInfoFromSmartctl((*it)["ln"], *it);
    }
    qCDebug(appLog) << "DeviceGenerator::getDiskInfoFromSmartCtl end";
}

void DeviceGenerator::getGpuInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getGpuInfoFromHwinfo start";
    // 加载从hwinfo获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_display");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            // qCDebug(appLog) << "Skipping GPU from hwinfo with insufficient data";
            continue;
        }

        DeviceGpu *device = new DeviceGpu();
        device->setForcedDisplay(true);
        device->setHwinfoInfo(*it);
        DeviceManager::instance()->addGpuDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    qCDebug(appLog) << "DeviceGenerator::getGpuInfoFromHwinfo end";
}

void DeviceGenerator::getGpuInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getGpuInfoFromLshw start";
    // 加载从lshw获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_display");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            // qCDebug(appLog) << "Skipping GPU from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setGpuInfoFromLshw(*it);
    }
    qCDebug(appLog) << "DeviceGenerator::getGpuInfoFromLshw end";
}

void DeviceGenerator::getGpuInfoFromXrandr()
{
    qCDebug(appLog) << "DeviceGenerator::getGpuInfoFromXrandr start";
    // 加载从xrandr获取的显示适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("xrandr");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping GPU from xrandr with insufficient data";
            continue;
        }

        DeviceManager::instance()->setGpuInfoFromXrandr(*it);
    }
}

void DeviceGenerator::getGpuSizeFromDmesg()
{
    qCDebug(appLog) << "DeviceGenerator::getGpuSizeFromDmesg start";
    // 加载从dmesg获取的显示适配器信息，设置显存大小
    const QList<QMap<QString, QString> > &lstMap = DeviceManager::instance()->cmdInfo("dmesg");
    if (lstMap.size() > 0 && lstMap[0].size() > 0) {
        for (QMap<QString, QString> curMap : lstMap) {
            // qCDebug(appLog) << "Setting GPU size from dmesg";
            DeviceManager::instance()->setGpuSizeFromDmesg(curMap);
        }
    } else {
        // dmesg无法获取显存信息时从nvidia-settings获取
        qCDebug(appLog) << "No dmesg info for GPU size, trying nvidia-settings";
        const QList<QMap<QString, QString>> &nvidiaMap = DeviceManager::instance()->cmdInfo("nvidia");
        if (nvidiaMap.size() > 0 && nvidiaMap[0].size() > 0) {
            for (QMap<QString, QString> curMap : nvidiaMap) {
                // qCDebug(appLog) << "Setting GPU size from nvidia-settings";
                DeviceManager::instance()->setGpuSizeFromDmesg(curMap);
            }
        }
    }
    qCDebug(appLog) << "DeviceGenerator::getGpuSizeFromDmesg end";
}
void DeviceGenerator::getMonitorInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getMonitorInfoFromHwinfo start";
    // 加载从hwinfo获取的显示设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_monitor");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            // qCDebug(appLog) << "Skipping monitor from hwinfo with insufficient data";
            continue;
        }

        DeviceMonitor *device = new DeviceMonitor();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addMonitor(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    qCDebug(appLog) << "DeviceGenerator::getMonitorInfoFromHwinfo end";
}

void DeviceGenerator::getMonitorInfoFromXrandrVerbose()
{
    // 加载从xrandr --verbose中获取的显示设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("xrandr_verbose");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping monitor from xrandr_verbose with insufficient data";
            continue;
        }

        DeviceManager::instance()->setMonitorInfoFromXrandr((*it)["mainInfo"], (*it)["edid"]);
    }
}

void DeviceGenerator::getAudioInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getAudioInfoFromHwinfo start";
    // 加载从hwinfo中获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_sound");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5 && (*it).find("path") == (*it).end()) {
            // qCDebug(appLog) << "Skipping sound device from hwinfo with insufficient data";
            continue;
        }

        QString path = pciPath(*it);
        DeviceAudio *device = dynamic_cast<DeviceAudio *>(DeviceManager::instance()->getAudioDevice(path));
        if (device) {
            // qCDebug(appLog) << "Found existing audio device, setting enable value to false.";
            device->setEnableValue(false);
            continue;
        }

        device = new DeviceAudio();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addAudioDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    DeviceManager::instance()->deleteDisableDuplicate_AudioDevice();
    qCDebug(appLog) << "DeviceGenerator::getAudioInfoFromHwinfo end";
}

void DeviceGenerator::getAudioInfoFrom_sysFS()
{
    // 加载从sysFS中获取的音频适配器信息，设备声卡型号
    // QMap<QString, QString> mapInfo_0;
    // QMap<QString, QString> mapInfo_1;
    // QMap<QString, QString> mapInfo_2;
    QMap<QString, QString> mapInfo;
    QString path;

    for (int i = 0; i < 5; i++) {
        mapInfo.clear();
        DeviceAudio *device = new DeviceAudio();
        bool getcard = device->setInfoFrom_sysFS(mapInfo, i);
        // qCDebug(appLog) << "Getting audio card info from sysFS, index:" << i << "result:" << getcard;
        if (getcard) {
            path = mapInfo["SysFS ID"];
            DeviceAudio *tmpdevice = dynamic_cast<DeviceAudio *>(DeviceManager::instance()->getAudioDevice(path));
            if (!tmpdevice) { // 没有该设备 说加新的
                // qCDebug(appLog) << "Adding new audio device from sysFS:" << path;
                DeviceManager::instance()->addAudioDevice(device);
            } else { //找到有就 加芯片等信息
                // qCDebug(appLog) << "Updating existing audio device from sysFS:" << path;
                QString chip = mapInfo["chip"];
                // DeviceManager::instance()->setAudioChipFromDmesg(chip);
                tmpdevice->setAudioChipFromDmesg(chip);
                delete device;
                device = nullptr;
            }
        }
    }
}

void DeviceGenerator::getAudioInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getAudioInfoFromLshw start";
    // 加载从lshw中获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_multimedia");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            // qCDebug(appLog) << "Skipping audio device from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setAudioInfoFromLshw(*it);
    }
    DeviceManager::instance()->deleteDisableDuplicate_AudioDevice();  //bug 150331  禁用显卡音频，重启后音频信息异常增多
    qCDebug(appLog) << "DeviceGenerator::getAudioInfoFromLshw end";
}

void DeviceGenerator::getAudioInfoFromCatInput()
{
    // 加载从 cat /proc/bus/input/devices获取的音频适配器信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("cat_devices");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {

        // 判断输入设备属于音频适配器
        if (true == (*it)["Sysfs"].contains("sound", Qt::CaseInsensitive)) {
            // qCDebug(appLog) << "Found sound device from cat_devices";
            DeviceAudio *device = new DeviceAudio();
            device->setInfoFromCatDevices(*it);
            DeviceManager::instance()->addAudioDevice(device);
        }
    }
}

void DeviceGenerator::getAudioChipInfoFromDmesg()
{
    qCDebug(appLog) << "Getting audio chip info from dmesg";
    // 加载从dmesg中获取的音频适配器信息，设备声卡型号
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("audiochip");
    if (lstMap.size() > 0) {
        qCDebug(appLog) << "Setting audio chip from dmesg:" << lstMap[0]["chip"];
        DeviceManager::instance()->setAudioChipFromDmesg(lstMap[0]["chip"]);
    }
}

void DeviceGenerator::getBluetoothInfoFromHciconfig()
{
    qCDebug(appLog) << "Getting bluetooth info from hciconfig";
    //  加载从hciconfig中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hciconfig");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    int index = 0;
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping bluetooth device from hciconfig with insufficient data";
            continue;
        }

        DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothAtIndex(index)) ;
        if (device) {
            // qCDebug(appLog) << "Setting bluetooth info from hciconfig for device at index" << index;
            device->setInfoFromHciconfig(*it);
        }
        index++;
    }
}

void DeviceGenerator::getBlueToothInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getBluetoothInfoFromHwinfo start";
    //  加载从hwinfo中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping bluetooth device from hwinfo with insufficient data";
            continue;
        }

        if ((*it)["Hardware Class"] == "hub" || (*it)["Hardware Class"] == "mouse" || (*it)["Hardware Class"] == "keyboard") {
            // qCDebug(appLog) << "Skipping hub, mouse, or keyboard device when looking for bluetooth";
            continue;
        }

        if ((*it)["Model"].contains("bluetooth", Qt::CaseInsensitive) || (*it)["Hardware Class"] == "bluetooth" || (*it)["Driver"] == "btusb" || (*it)["Device"] == "BCM20702A0" || (*it)["Device"] == "SCM2625 BT 5.2 network adapter") {
            // qCDebug(appLog) << "Found bluetooth device from hwinfo";
            // 判断重复设备数据
            QString unique_id = uniqueID(*it);
            DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothDevice(unique_id));
            if (device) {
                // qCDebug(appLog) << "Updating existing bluetooth device:" << unique_id;
                device->setEnableValue(false);
                device->setInfoFromHwinfo(*it);
                continue;
            }

            device = new DeviceBluetooth();
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addBluetoothDevice(device);
            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        } else if (!it->contains("Driver") && it->contains("Driver Status") && (*it)["Driver Status"].contains("is")) {
            QStringList driverStatus = (*it)["Driver Status"].split("is");
            if (!driverStatus.isEmpty() && driverStatus.first().contains("btusb")) {
                // qCDebug(appLog) << "Found bluetooth device from hwinfo by driver status";
                // 判断重复设备数据
                QString unique_id = uniqueID(*it);
                DeviceBluetooth *device = dynamic_cast<DeviceBluetooth *>(DeviceManager::instance()->getBluetoothDevice(unique_id));
                if (device) {
                    // qCDebug(appLog) << "Updating existing bluetooth device:" << unique_id;
                    device->setEnableValue(false);
                    device->setInfoFromHwinfo(*it);
                    continue;
                }

                device = new DeviceBluetooth();
                device->setInfoFromHwinfo(*it);
                DeviceManager::instance()->addBluetoothDevice(device);
                addBusIDFromHwinfo((*it)["SysFS BusID"]);
            }
        }
    }
    qCDebug(appLog) << "DeviceGenerator::getBluetoothInfoFromHwinfo end";
}

void DeviceGenerator::getBluetoothInfoFromLshw()
{
    qCDebug(appLog) << "Getting bluetooth info from lshw";
    //  加载从lshw中获取的蓝牙信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping bluetooth device from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setBluetoothInfoFromLshw(*it);
    }
}

void DeviceGenerator::getKeyboardInfoFromHwinfo()
{
    qCDebug(appLog) << "Getting keyboard info from hwinfo";
    //  加载从hwinfo中获取的键盘信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_keyboard");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping keyboard from hwinfo with insufficient data";
            continue;
        }

        DeviceInput *device = new DeviceInput();
        device->setInfoFromHwinfo(*it);
        device->setHardwareClass("keyboard");
        device->setCanEnale(false);
        DeviceManager::instance()->addKeyboardDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
}


void DeviceGenerator::getKeyboardInfoFromLshw()
{
    qCDebug(appLog) << "Getting keyboard info from lshw";
    //  加载从lshw中获取的键盘信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping keyboard from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setKeyboardInfoFromLshw(*it);
    }
}

void DeviceGenerator::getMouseInfoFromHwinfo()
{
    qCDebug(appLog) << "Getting mouse info from hwinfo";
    //  加载从hwinfo中获取的鼠标信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_mouse");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping mouse from hwinfo with insufficient data";
            continue;
        }

        // 不让数位板显示在鼠标设备里面(武汉那边的数位板)
        if ((*it)["Device"].contains("PM")) {
            // qCDebug(appLog) << "Skipping PM device (Wacom tablet)";
            continue;
        }

        // 不让数位板显示在鼠标设备里面(nanjing的数位板)
        if ((*it)["Device"].contains("T70")) {
            // qCDebug(appLog) << "Skipping T70 device (Wacom tablet)";
            continue;
        }

        //过滤触摸板，如果不是触摸板再检查
        if ((*it)["Hotplug"] != "PS/2") {
            // 先判断是否存在
            QString path = pciPath(*it);
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                // qCDebug(appLog) << "Skipping mouse device without authorized file:" << path;
                continue;
            }
        }

        QString unique_id = uniqueID(*it);
        DeviceInput *device = dynamic_cast<DeviceInput *>(DeviceManager::instance()->getMouseDevice(unique_id));
        if (device) {
            // qCDebug(appLog) << "Updating existing mouse device:" << unique_id;
            device->setEnableValue(false);
            continue;
        } else {
            if ((*it).find("path") != (*it).end()) {
                // qCDebug(appLog) << "Skipping mouse device with path attribute";
                continue;
            }
        }

        device = new DeviceInput();
        device->setInfoFromHwinfo(*it);
        device->setHardwareClass("mouse");
        if (device->bluetoothIsConnected()) {
            // qCDebug(appLog) << "Adding bluetooth mouse device";
            DeviceManager::instance()->addMouseDevice(device);
            addBusIDFromHwinfo((*it)["SysFS BusID"]);
        } else {
            // qCDebug(appLog) << "Skipping non-bluetooth connected mouse";
            delete device;
            device = nullptr;
        }
    }

    //  加载从hwinfo --usb中获取的触摸屏信息具有鼠标功能，放到鼠标设备中
    const QList<QMap<QString, QString>> &lstMapUSB = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator iter = lstMapUSB.begin();
    for (; iter != lstMapUSB.end(); ++iter) {
        if ((*iter).size() < 1) {
            // qCDebug(appLog) << "Skipping mouse from hwinfo_usb with insufficient data";
            continue;
        }

        // 指定型号触摸屏，显示在鼠标设备中
        if ((*iter)["Model"].contains("Melfas LGDisplay Incell Touch")) {
            // qCDebug(appLog) << "Adding Melfas LGDisplay Incell Touch as mouse device";
            DeviceInput *device = new DeviceInput();
            device->setInfoFromHwinfo(*iter);
            DeviceManager::instance()->addMouseDevice(device);
            addBusIDFromHwinfo((*iter)["SysFS BusID"]);
        }
    }
}

void DeviceGenerator::getMouseInfoFromLshw()
{
    qCDebug(appLog) << "Getting mouse info from lshw";
    //  加载从lshw中获取的鼠标信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 1) {
            // qCDebug(appLog) << "Skipping mouse from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->addMouseInfoFromLshw(*it);
    }
}

void DeviceGenerator::getMouseInfoFromCatDevices()
{
    qCDebug(appLog) << "Getting mouse info from cat_devices";

}

void DeviceGenerator::getImageInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getImageInfoFromHwinfo start";
    //  加载从hwinfo中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 3) {
            // qCDebug(appLog) << "Skipping image device from hwinfo_usb with insufficient data";
            continue;
        }

        // hwinfo中对camera的分类不明确，通过camera等关键字认定图像设备
        if (!(*it)["Model"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Device"].contains("camera", Qt::CaseInsensitive) &&
                !(*it)["Driver"].contains("uvcvideo", Qt::CaseInsensitive) &&
                !(*it)["Model"].contains("webcam", Qt::CaseInsensitive) &&
                (*it)["Hardware Class"] != "camera") {
            // qCDebug(appLog) << "Skipping non-camera device from hwinfo_usb";
            continue;
        }

        // 判断该摄像头是否存在，被禁用的和被拔出
        QString path = pciPath(*it);
        if (path.contains("usb")) {
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                // qCDebug(appLog) << "Skipping camera without authorized file:" << path;
                continue;
            }
        }

        // 判断重复设备数据
        QString unique_id = uniqueID(*it);
        DeviceImage *device = dynamic_cast<DeviceImage *>(DeviceManager::instance()->getImageDevice(unique_id));
        if (device) {
            // qCDebug(appLog) << "Updating existing image device:" << unique_id;
            device->setEnableValue(false);
            device->setInfoFromHwinfo(*it);
            continue;
        } else {
            if ((*it).find("path") != (*it).end()) {
                // qCDebug(appLog) << "Skipping image device with path attribute";
                continue;
            }
        }

        device = new DeviceImage();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addImageDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    qCDebug(appLog) << "DeviceGenerator::getImageInfoFromHwinfo end";
}

void DeviceGenerator::getImageInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getImageInfoFromLshw start";
    //  加载从lshw中获取的图像设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping image device from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setCameraInfoFromLshw(*it);
    }
    qCDebug(appLog) << "DeviceGenerator::getImageInfoFromLshw end";
}

void DeviceGenerator::getCdromInfoFromHwinfo()
{
    qCDebug(appLog) << "DeviceGenerator::getCdromInfoFromHwinfo start";
    //  加载从hwinfo中获取的cdrom设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_cdrom");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 5) {
            // qCDebug(appLog) << "Skipping cdrom from hwinfo with insufficient data";
            continue;
        }

        DeviceCdrom *device = new DeviceCdrom();
        device->setInfoFromHwinfo(*it);
        DeviceManager::instance()->addCdromDevice(device);
        addBusIDFromHwinfo((*it)["SysFS BusID"]);
    }
    qCDebug(appLog) << "DeviceGenerator::getCdromInfoFromHwinfo end";
}

void DeviceGenerator::getCdromInfoFromLshw()
{
    qCDebug(appLog) << "DeviceGenerator::getCdromInfoFromLshw start";
    //  加载从lshw中获取的cdrom设备信息
    const QList<QMap<QString, QString>> &lstDisk = DeviceManager::instance()->cmdInfo("lshw_cdrom");
    QList<QMap<QString, QString> >::const_iterator dIt = lstDisk.begin();
    for (; dIt != lstDisk.end(); ++dIt) {
        if ((*dIt).size() < 2) {
            // qCDebug(appLog) << "Skipping cdrom from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->addLshwinfoIntoCdromDevice(*dIt);
    }
    qCDebug(appLog) << "DeviceGenerator::getCdromInfoFromLshw end";
}

void DeviceGenerator::getOthersInfoFromHwinfo()
{
    qCDebug(appLog) << "Getting others info from hwinfo";
    //  加载从hwinfo中获取的其他设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("hwinfo_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 3) {
            // qCDebug(appLog) << "Skipping other device from hwinfo_usb with insufficient data";
            continue;
        }
        /*  bug 141439 不可见功能设计需要再次思考*/
        if ((*it).find("Device") != (*it).end() &&
                ((*it)["Device"].contains("fingerprint", Qt::CaseInsensitive)  ||
                 (*it)["Device"].contains("MOH", Qt::CaseInsensitive)
                )) {
            // qCDebug(appLog) << "Found fingerprint or MOH device, adding as other device";
            DeviceOthers *device = new DeviceOthers();
            device->setForcedDisplay(true);
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
            continue;
        }

        bool isOtherDevice = true;
        QString curBus = (*it)["SysFS BusID"];
        curBus.replace(QRegularExpression("\\.[0-9]{1,2}$"), "");
        const QStringList &lstBusId = DeviceManager::instance()->getBusId();
        // 判断该设备是否已经在其他类别中显示
        if ((*it).find("unique_id") != (*it).end() && (*it)["Hardware Class"] != "others") {
            // qCDebug(appLog) << "Device with unique_id is not other device";
            isOtherDevice = false;
        } else if ((*it).find("unique_id") == (*it).end()) {
            if (curBus.isEmpty() || lstBusId.indexOf(curBus) != -1) {
                // qCDebug(appLog) << "Device with existing busId is not other device";
                isOtherDevice = false;
            }
        }

        // 添加其他设备
        if (isOtherDevice) {
            // qCDebug(appLog) << "Found other device";
            // 先判断是否存在
            QString path = pciPath(*it);
            // 判断authorized是否存在，不存在则直接返回
            if (!QFile::exists("/sys" + path + "/authorized")) {
                // qCDebug(appLog) << "Skipping other device without authorized file:" << path;
                continue;
            }

            QString unique_id = uniqueID(*it);
            DeviceOthers *device = dynamic_cast<DeviceOthers *>(DeviceManager::instance()->getOthersDevice(unique_id));
            if (device) {
                // qCDebug(appLog) << "Updating existing other device:" << unique_id;
                device->setEnableValue(false);
                continue;
            } else {
                if ((*it).find("path") != (*it).end()) {
                    // qCDebug(appLog) << "Skipping other device with path attribute";
                    continue;
                }
            }


            device = new DeviceOthers();
            device->setInfoFromHwinfo(*it);
            DeviceManager::instance()->addOthersDevice(device);
        }
    }
}

void DeviceGenerator::getOthersInfoFromLshw()
{
    qCDebug(appLog) << "Getting others info from lshw";
    // 加载从lshw中获取的其他设备信息
    const QList<QMap<QString, QString>> &lstMap = DeviceManager::instance()->cmdInfo("lshw_usb");
    QList<QMap<QString, QString> >::const_iterator it = lstMap.begin();
    for (; it != lstMap.end(); ++it) {
        if ((*it).size() < 2) {
            // qCDebug(appLog) << "Skipping other device from lshw with insufficient data";
            continue;
        }

        DeviceManager::instance()->setOthersDeviceInfoFromLshw(*it);
    }
}

void DeviceGenerator::addBusIDFromHwinfo(const QString &sysfsBusID)
{
    qCDebug(appLog) << "Adding bus ID from hwinfo:" << sysfsBusID;
    // 添加hwinfo中唯一标识设备的字符串
    if (sysfsBusID.isEmpty()) {
        qCDebug(appLog) << "Bus ID is empty, returning";
        return;
    }

    QString busID = sysfsBusID;
    busID.replace(QRegularExpression("\\.[0-9]+$"), "");

    m_ListBusID.append(busID);
}

const QStringList &DeviceGenerator::getBusIDFromHwinfo()
{
    qCDebug(appLog) << "Getting bus IDs from hwinfo";
    return m_ListBusID;
}

const QString DeviceGenerator::getProductName()
{
    qCDebug(appLog) << "DeviceGenerator::getProductName start";
    // 由DTK接口获取系统名称
    QString name = DSysInfo::uosSystemName(QLocale(QLocale::English));
    QString productType = DSysInfo::uosProductTypeName(QLocale(QLocale::English));

    if (!productType.contains("Server", Qt::CaseInsensitive)) {
        // 非服务器版 "产品名称"+"大版本号"+"版本名称"
        qCDebug(appLog) << "Generating product name for non-server version";
        name += " " + DSysInfo::majorVersion();
        name += " " + DSysInfo::uosEditionName(QLocale(QLocale::English));
    } else {
        // 服务器版 "产品名称"+"大版本号"+"（完整版本号识别码版本识别码）"
        qCDebug(appLog) << "Generating product name for server version";
        name += " " + DSysInfo::majorVersion();
        name += " (";
        name += DSysInfo::minorVersion();
        name += DSysInfo::uosEditionName(QLocale(QLocale::English));
        name += ")";
    }

    qCDebug(appLog) << "DeviceGenerator::getProductName end, name:" << name;
    return name;
}

QString DeviceGenerator::pciPath(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Getting PCI path from map info";
    if (mapInfo.find("path") != mapInfo.end()) { // SysFS Device Link
        return mapInfo["path"];
    } else if (mapInfo.find("SysFS Device Link") != mapInfo.end()) {
        return mapInfo["SysFS Device Link"];
    } else if (mapInfo.find("SysFS ID") != mapInfo.end()) {
        return mapInfo["SysFS ID"];
    } else {
        if (mapInfo["Device Files"].contains("platform")) {
            qCDebug(appLog) << "Found platform device";
            return "platform";
        }
        qCDebug(appLog) << "No PCI path found";
        return "";
    }
}

QString DeviceGenerator::uniqueID(const QMap<QString, QString> &mapInfo)
{
    qCDebug(appLog) << "Getting unique ID from map info";
    if (mapInfo.find("unique_id") != mapInfo.end()) {
        return mapInfo["unique_id"];
    } else if (mapInfo.find("Unique ID") != mapInfo.end()) {
        return mapInfo["Unique ID"];
    } else if (mapInfo.find("Permanent HW Address") != mapInfo.end()) {
        return mapInfo["Permanent HW Address"];
    } else if (mapInfo.find("Serial ID") != mapInfo.end()) {
        return mapInfo["Serial ID"];
    }
    qCDebug(appLog) << "No unique ID found";
    return "";
}

void DeviceGenerator::generatorInfoFromToml(DeviceType deviceType)
{
    qCDebug(appLog) << "Generator info from toml";
    DeviceManager::instance()->tomlDeviceSet(deviceType);
}

