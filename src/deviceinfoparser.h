#pragma once

#include <QMap>
#include <QString>
#include "singletondef.h"
#include "deviceattributedefine.h"

const QString Deviceype_Computer = "Computer";
const QString Devicetype_Name = "Name";
const QString Devicetype_lshw_Class_Prefix = "*-";
const char Devicetype_Separator = ':';
const QString Devicetype_Stitching_Symbol = "_";
const char DeviceType_CatDevice_Separator = '=';
const QString Devicetype_CatDeviceSysfs = "Sysfs";

const QString Devicetype_Xrandr_Screen = "Screen";
const QString Devicetype_Xrandr_Connected = "connected";
const QString Devicetype_Xrandr_Disconnected = "disconnected";
const char Devicetype_Xrandr_Space = ' ';
const QString Devicetype_Xrandr_Screen_Separator = ",";
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

const QString Devicetype_HwInfo_Twospace = "  ";
const QString Devicetype_HwInfo_Fourspace = "    ";
const QString Devicetype_HwInfo_Resolution = "Resolution";
const QString Devicetype_HwInfo_ResolutionList = "Resolution list";
const QString Devicetype_HwInfo_Currentresolution = "Current Resolution";

const QString Devicetype_lpstat_4Space = "    ";
const QString Devicetype_lpstat_Tab = "\t";

typedef QMap<QString, QMap<QString, QString>> DatabaseMap;

class DeviceInfoParser
{
public:
    Declare_Singleton(DeviceInfoParser);

public:
    bool isToolSuccess(const QString& toolname);

    const QString& queryData(const QString& toolname, const QString& firstKey, const QString& secondKey);
    const QString& fuzzyQueryData(const QString& toolname, const QString& firstKey, const QString& secondKey);

    bool fuzzeyQueryKey(const QString& toolname, const QString& fuzzeyKey, QString& key);

    bool queryDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles);
    bool queryRemainderDeviceInfo(const QString& toolname, const QString& deviceName, QList<ArticleStruct>& articles, const QSet<QString>& existArticles);

    QStringList getCpuList();
    QStringList getMemorynameList();
    QStringList getDisknameList();
    QStringList getDiaplayadapterList();
    QStringList getDisplayInterfaceList();
    QStringList getMonitorList();
    QStringList getConnectedMonitorList();
    QStringList getInputdeviceList();
    QStringList getNetworkadapterList();
    QStringList getBluetoothList();
    QStringList getCameraList();
    QStringList getUsbdeviceList();

    QStringList getSwitchingpowerList();
    QStringList getBatteryList();

    QStringList getOtherInputdeviceList();
    QStringList getOtherPciDeviceList();
    QStringList getPrinterList();
    
    // get os
    bool getOSInfo(QString& osInfo);
    // dmidecode parse
    bool loadDemicodeDatabase();
    // lshw parse
    bool loadLshwDatabase();
    // lscpu parse
    bool loadLscpuDatabase();
    // cat proc/cpu
    bool loadCatcpuDatabase();
    // smartctl www.smartmontools.org get disk info
    bool loadSmartctlDatabase(const QString& diskLogical);
    // cat /proc/bus/input/devices
    bool loadCatInputDatabase();
    // xrandr
    bool loadXrandrDatabase();
    // get power settings
    bool loadPowerSettings();
    // lspci parse
    bool loadLspciDatabase();
    // hciconfig
    bool loadHciconfigDatabase();
    // lsusbss
    bool loadLsusbDatabase();
    // hwinfo
    bool loadHwinfoDatabase();
    // lpstat
    bool loadLpstatDatabase();
    // cups
    bool loadCupsDatabase();

public:
    bool executeProcess(const QString& cmd);
    bool runCmd(const QString& cmd);
    bool runCmd(const QStringList& cmdList);

    QString standOutput_;

    QMap<QString, DatabaseMap> toolDatabase_;

    // Power settings
    // Switchingpower
    int switchingpowerScreenSuspendDelay_ = 0;      // screen suspend
    int switchingpowerComputerSuspendDelay_ = 0;    // computer suspend
    int switchingpowerAutoLockScreenDelay_ = 0;     // auto lock screen
    // Battery
    int batteryScreenSuspendDelay_ = 0;
    int batteryComputerSuspendDelay_ = 0;
    int batteryAutoLockScreenDelay_ = 0;
};

#define DeviceInfoParserInstance (DeviceInfoParser::Instance())
