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

typedef bool (*checkValueFun_t)(const QString&);

class DeviceInfoParser: public QObject
{
    Q_OBJECT
public:
    static DeviceInfoParser& Instance()
    {
        static DeviceInfoParser _instance;
        return _instance;
    }

    DeviceInfoParser();
    ~DeviceInfoParser();

    void refreshDabase();

signals:
    void loadFinished(const QString& msg);

public:
    bool isToolSuccess(const QString& toolname);

    const QString& queryData(const QString& toolname, const QString& firstKey, const QString& secondKey);
    const QString& fuzzyQueryData(const QString& toolname, const QString& firstKey, const QString& secondKey);

    bool fuzzeyQueryKey(const QString& toolname, const QString& fuzzeyKey, QString& key);

    bool queryDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles);
    bool queryRemainderDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles, const QSet<QString>& existArticles = QSet<QString>());

    bool checkValue(const QString& toolName, const QString& device, const QString& key, const QString& contains);

private:
    QStringList getMatchToolDeviceList(const QString& toolName, checkValueFun_t* checkFunc = nullptr);

public:
    QStringList getCatcpuCpuList();
    QStringList getlscpuCpuList();

    QStringList getDmidecodePhysicMemory();
    QStringList getDmidecodeMemoryList();
    QStringList getDmidecodeMemoryArrayMappedAddress();

    QStringList getLshwDisknameList();

    QStringList getLshwDiaplayadapterList();
    QStringList getXrandrScreenName();
    QStringList getXrandrDisplayInterfaceList();

    QStringList getHwinfoMonitorList();
    QStringList getXrandrMonitorList();
    QString     getEDID();

    QStringList getLshwMultimediaList();
    QStringList getCatinputAudioDeviceList();

    QStringList getCatinputInputdeviceList();

    QStringList getLshwNetworkadapterList();

    QStringList getLshwBluetoothList();

    QStringList getHciconfigBluetoothControllerList();
    QStringList getOtherBluetoothctlPairedAndConnectedDevicesList();

    QStringList getLshwCameraList();

    QStringList getLshwOtherUsbdeviceList();
    QStringList getLshwUsbKeyboardDeviceList();

    QStringList getInputdeviceMouseList();
    QString getCorrespondLshwMouse(const QString& inputMouse);
    QString getCorrespondBluetoothMouse(const QString& inputMouse);

    QStringList getInputdeviceKeyboardList();
    QString getCorrespondLshwKeyboard(const QString& inputKeyboard);
    QString getCorrespondBluetoothKeyboard(const QString& inputKeyboard);

    QString getCorrespondUpower(const QString& bluetoothDevice);

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
    QString getLsbRelease();
    QString getHomeUrl();

    bool loadCatosrelelease();
    bool loadlsb_release();
    bool loadOSInfo();

    // dmidecode parse
    bool loadDemicodeDatabase();
    // cat /proc/baseboard
    bool loadCatBoardinfoDatabase();
    // lshw parse
    bool loadLshwDatabase();
    // lscpu parse
    bool loadLscpuDatabase();
    // cat proc/cpu
    bool loadCatcpuDatabase();
    // smartctl www.smartmontools.org get disk info
    bool loadAllSmartctlDatabase();
    bool loadSmartctlDatabase(const QString& diskLogical);
    // cat /proc/bus/input/devices
    bool loadCatInputDatabase();
    // xrandr
    bool loadXrandrDatabase();
    // get power settings
    bool loadPowerSettings();
    bool loadUpowerDatabase();
    // lspci parse
    bool loadLspciDatabase();
    // bluetooth
    bool loadHciconfigDatabase();
    bool loadAllBluetoothctlControllerDatabase();
    bool loadBluetoothctlDatabase(const QString& controller);
    QStringList getAllBluetoothctlPairedDevices();
    bool loadAllBluethctlPairedDeviceDatabase();
    bool loadBluetoothctlPairedDeviceDatabase(const QString& mac);
    // lsusb
    bool loadLsusbDatabase();
    // hwinfo
    bool loadHwinfoDatabase();
    // lpstat
    bool loadLpstatDatabase();
    // cups
    bool loadCupsDatabase();

public:
    bool getRootPassword();
    bool executeProcess(const QString& cmd);
    bool runCmd(const QString& cmd);
    bool runCmd(const QStringList& cmdList);

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

public:
    QSet<QString> orderedDevices;
};

