#pragma once

#include <QMap>
#include <QString>

const QString Deviceype_Computer = "Computer";
const QString Devicetype_Name = "Name";
const QString Devicetype_lshw_Class_Prefix = "*-";
const QString Devicetype_Separator = ":";
const QString Devicetype_Stitching_Symbol = "_";
const char DeviceType_CatDevice_Separator = '=';
const QString Devicetype_CatDeviceSysfs = "Sysfs";

class DeviceInfoParser
{
public:
    DeviceInfoParser();

    // get os
    bool getOSInfo(QString& osInfo);
    // dmidecode parse
    bool loadDemicodeDatabase();
    // lshw parse
    bool loadLshwDatabase();
    // lscpu parse
    bool loadLscpuDatabase();
    // smartctl www.smartmontools.org get disk info
    bool loadSmartctlDatabase();
    // cat /proc/bus/input/devices
    bool loadCatInputDatabase();

public:
    bool executeProcess(const QString& cmd);
    QString standOutput_;

    // dimdecode
    QMap<QString, QMap<QString, QString>> dimdecodeDatabase_;
    // lshw
    QMap<QString, QMap<QString, QString>> lshwDatabase_;
    // lscpu
    QMap<QString, QString> lscpuDatabase_;
    // smartctl
    QMap<QString, QString> smartctlDatabase_;
    // cat /proc/bus/input/devices
    QMap<QString, QMap<QString, QString>> catInputDeviceDatabase_;
};
