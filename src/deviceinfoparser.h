/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QMap>
#include <QString>
#include "singletondef.h"
#include "deviceattributedefine.h"
#include <QObject>
#include <QSet>

const QString Deviceype_Computer = "Computer";
const QString Devicetype_Name = "Name";
const QString Devicetype_lshw_Class_Prefix = "*-";
const char Devicetype_Separator = ':';
const QString Devicetype_Stitching_Symbol = "_";
const char DeviceType_CatDevice_Separator = '=';
const QString Devicetype_CatDeviceSysfs = "Sysfs";

const QString Devicetype_Xrandr_Screen = "Screen";
const QString Devicetype_Xrandr_Connected = " connected";
const QString Devicetype_Xrandr_Disconnected = " disconnected";
const char Devicetype_Xrandr_Space = ' ';
const QString Devicetype_Xrandr_Screen_Separator = ", ";
const QString Devicetype_Xrandr_Minimum = "minimum";
const QString Devicetype_Xrandr_Current = "current";
const QString Devicetype_Xrandr_Maximum = "maximum";
const QString Devicetype_Xrandr_Tab = "\t";
const QString Devicetype_Xrandr_Twotab = "\t\t";
const QString Devicetype_Xrandr_Twospace = "  ";
const QString Devicetype_Xrandr_TabAndspace = "\t  ";

const QString Devicetype_Lspci_Seperator = ": ";
const QString Devicetype_Lspci_Tab = "\t";
const QString Devicetype_Lspci_Memory = "Memory";
const QString Devicetype_Lspci_non_prefetchable = "non-prefetchable";
const QString Devicetype_Lspci_prefetchable = "prefetchable";

const QString Devicetype_Hciconfig_Hci = "hci";
const QString Devicetype_Hciconfig_Multispace = "    ";
const QString Devicetype_Hciconfig_Tab = "\t";

const QString Devicetype_HwInfo_Twospace = "  ";
const QString Devicetype_HwInfo_Fourspace = "    ";
const QString Devicetype_HwInfo_Resolution = "Resolution";
const QString Devicetype_HwInfo_ResolutionList = "Support Resolution";
const QString Devicetype_HwInfo_Currentresolution = "Current Resolution";

const QString Devicetype_lpstat_4Space = "    ";
const QString Devicetype_lpstat_Tab = "\t";

typedef QMap<QString, QMap<QString, QString>> DatabaseMap;

//class LogPasswordAuth;

typedef bool (*checkValueFun_t)(const QString &);

class DeviceInfoParser: public QObject
{
    Q_OBJECT
public:
    static DeviceInfoParser &Instance()
    {
        static DeviceInfoParser _instance;
        return _instance;
    }

    DeviceInfoParser();
    ~DeviceInfoParser();

    void refreshDabase();

signals:
    void loadFinished(const QString &msg);

public:
    bool isToolSuccess(const QString &toolname);

    const QString &queryData(const QString &toolname, const QString &firstKey, const QString &secondKey);
    const QString &fuzzyQueryData(const QString &toolname, const QString &firstKey, const QString &secondKey);

    bool fuzzeyQueryKey(const QString &toolname, const QString &fuzzeyKey, QString &key);

    bool queryDeviceInfo(const QString &toolname, const QString &deviceName, QList<ArticleStruct> &articles);
    bool queryRemainderDeviceInfo(const QString &toolname, const QString &deviceName,
                                  QList<ArticleStruct> &articles, const QSet<QString> &existArticles = QSet<QString>(),
                                  const char *context = nullptr, const char *disambiguation = nullptr);

    bool checkValue(const QString &toolName, const QString &device, const QString &key, const QString &contains);

private:
    QStringList getMatchToolDeviceList(const QString &toolName, checkValueFun_t *checkFunc = nullptr);

public:
    QStringList getCatcpuCpuList();
    QStringList getlscpuCpuList();
    /**@brief:FT2000+的环境无法获取cpu频率，只能通过dmidecode命令获取*/
    QStringList getDmidecodeCpuList();

    QStringList getDmidecodePhysicMemory();
    QStringList getDmidecodeMemoryList();
    //be careful about function above,the constructor and compare function of class ArticleStruct may not support some container function,such as bool contains(ArticleStruct)
    QMap<QString, QMap<QString, QString>> getLshwMeoryList();

    QStringList getLshwDiskNameList();

    QStringList getLshwDiaplayadapterList();
    QStringList getXrandrScreenName();
    QStringList getXrandrDisplayInterfaceList();

    QStringList getHwinfoDiskList();
    QStringList getHwinfoKeyboardList();
    QStringList getHwinfoOtherUSBList();
    QStringList getHwinfoPrinterList();
    QStringList getHwinfoMonitorList();
    QStringList getXrandrMonitorList();
    //获得“xrandr”的信息
    QStringList getSomeXrandrMonitorList();

    QString     getEDID(int index = 0);

    QStringList getLshwMultimediaList();
    QStringList getCatinputAudioDeviceList();

    QStringList getCatinputInputdeviceList();

    QStringList getLshwNetworkadapterList();

    QStringList getLshwBluetoothList();

    //获得显存的大小
    QStringList getDmesgVram();

    QStringList getHciconfigBluetoothControllerList();
    QStringList getOtherBluetoothctlPairedAndConnectedDevicesList();

    QStringList getLshwCameraList();
    QStringList getHwinfoCameraList(); // 从hwinfo 命令中获取 cameralist

    QStringList getLshwOtherUsbdeviceList();
    QStringList getLshwUsbKeyboardDeviceList();

    QStringList getInputdeviceMouseList();
    QString getCorrespondLshwMouse(const QString &inputMouse);
    QString getCorrespondBluetoothMouse(const QString &inputMouse);

    QStringList getInputdeviceKeyboardList();
    QString getCorrespondLshwKeyboard(const QString &inputKeyboard);
    QString getCorrespondBluetoothKeyboard(const QString &inputKeyboard);

    QString getCorrespondUpower(const QString &bluetoothDevice);

    QStringList getLshwSwitchingpowerList();
    QStringList getDemidecodeSwitchingpowerList();
    QStringList getUpowerSwitchingList();

    QStringList getLshwBatteryList();
    QStringList getDemidecodeBatteryList();
    QStringList getUpowerBatteryList();

    QStringList getLshwOtherDeviceList();
    QStringList getLshwOtherPciDeviceList();

    QStringList getLshwPrinterList();
    QStringList getCupsPrinterList();

    QStringList getLshwCDRomList();

    // get os
    QString getOsInfo();
    bool isHuaweiAndroidUos();
    QString getLsbRelease();
    QString getHomeUrl();

    bool loadCatosrelelease();
    bool loadlsb_release();
    bool loadOSInfo();
    QString getOsVersion();

    // dmidecode parse
    bool loadDmidecodeDatabase();
    // cat /proc/baseboard
    bool loadCatBoardinfoDatabase();
    // lshw parse
    bool loadLshwDatabase();
    // sudo lsblk -l stdout parse
    bool loadLsblKDatabase();

    // 获取 存储设备 介质类型
    QStringList getLsblkDiskTypeList();

    QStringList getLsblkDiskNameList();
    // lscpu parse
    bool loadLscpuDatabase();
    // cat proc/cpu
    bool loadCatcpuDatabase();
    // smartctl www.smartmontools.org get disk info
    bool loadAllSmartctlDatabase();
    bool loadSmartctlDatabase(const QString &diskLogical);
    // cat /proc/bus/input/devices
    bool loadCatInputDatabase();
    // xrandr ,xrandr --verbose,command
    bool loadXrandrDatabase();
    bool loadSomeXrandrDatabase();
    // get power settings
    bool loadPowerSettings();

    /**
     * @brief[loadUpowerDatabase]:通过命令 upower --dump 获取电源信息
     * @brief[parsePowerInfo]:解析所有电源信息中的一段
     */
    bool loadUpowerDatabase();
    void parsePowerInfo(const QString &info, DatabaseMap &powerDataBase, QStringList &secondOrder);


    // lspci parse
    bool loadLspciDatabase();
    // bluetooth
    bool loadHciconfigDatabase();
    bool loadAllBluetoothctlControllerDatabase();
    bool loadBluetoothctlDatabase(const QString &controller);
    QStringList getAllBluetoothctlPairedDevices();
    bool loadAllBluethctlPairedDeviceDatabase();
    bool loadBluetoothctlPairedDeviceDatabase(const QString &mac);
    // lsusb
    bool loadLsusbDatabase();
    // hwinfo
    bool loadHwinfoDatabase();// 这里面加载的是 sodu hwinfo --monitor
    /**
     * @brief:获取外接摄像头信息，在龙芯上lshw无法获取摄像头信息(如何升级了bios或许可以获取)，因此需要通过hwinfo来获取摄像头信息，
     *        但是 sudo hwinfo --camera 无法获取外接的摄像头设备(可能只能获取内置摄像头，本人没有验证)，所以通过运行 sudo hwinfo --usb
     *        然后 从中获取摄像机相关的信息，后期若有更好方法，请修改注释
     */
    bool loadHwinfoDatabaseOfCamera(); // 这里面执行的是 sudo hwinfo --usb
    /**@brief:添加单个摄像机*/
    void addACameraInfo(const QString &name, const QString &content);

    bool loadGpuInfo();
    // lpstat
    bool loadLpstatDatabase();
    // cups
    bool loadCupsDatabase();

    // printers
    bool loadPrinterinfoDatabase();

    // USB hwinfo --usb
    bool loadHwinfoUSBDatabase();

    // USB hwinfo --keybooard
    bool loadKeyboardinfoDatabase();

    //获得显存的大小
    bool loadDmesgVram();

    // storage hwinfo --disk
    bool loadhwinfoDiskDatabase();

    /**
     * @brief：获取键盘信息,键盘信息的获取只要通过两个命令获取
     *         sudo lshw -C input
     *         sudo hwinfo --keyboard
     * @brief[loadKeyboardInfoFromlshwInput]:从 sudo lshw -C input 中获取键盘信息
     * @brief[loadKeyBoardInfoFromHwinfoKeyboard]:从 sudo hwinfo --keyboard 中获取键盘信息
     */
//    bool loadKeyboardInfoFromlshwInput();
//    bool loadKeyBoardInfoFromHwinfoKeyboard();


    // 判断分辨率
    int judgeResolution(QString &current, QString &max, QString &min);

public:
    bool getRootPassword();
    bool executeProcess(const QString &cmd);
    bool runCmd(const QString &cmd);
    bool runCmd(const QStringList &cmdList);

    //LogPasswordAuth* autoDialog = nullptr;
    QString standOutput_;

    QMap<QString, DatabaseMap> toolDatabase_;
    QMap<QString, QStringList> toolDatabaseSecondOrder_;

    // Power settings
    // Switchingpower
    int switchingpowerScreenSuspendDelay_ = 0;      // screen suspend
    int switchingpowerComputerSuspendDelay_ = 0;    // computer suspend
    int switchingpowerAutoLockScreenDelay_ = 0;     // auto lock screen
    // Battery
    int batteryScreenSuspendDelay_ = 0;
    int batteryComputerSuspendDelay_ = 0;
    int batteryAutoLockScreenDelay_ = 0;

    QString osInfo_;
    QString homeUrl_;
    QString lsbRelease_;
    QString osVersion_;
    // 获取显存的大小
    QString varmSize = "Unknown";
    // 获取屏幕的分辨率和刷新率，一个屏幕
    QString currentResolutionRefresh;

    DatabaseMap                      m_KeyBoardInfo;

public:
    QSet<QString> orderedDevices;
};

