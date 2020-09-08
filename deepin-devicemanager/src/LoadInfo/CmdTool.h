/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

#ifndef CMDTOOL_H
#define CMDTOOL_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include <QFile>
#include <cups.h>

class CmdTool
{
public:
    CmdTool();

    /**
     * @brief loadCmdInfo:通过命令获取设备信息,[loadCmdInfo]:一般的处理方式
     * @param key:与命令对应的关键字
     * @param cmd:命令
     * @param debugFile:调试时所需文件名
     */
    void loadCmdInfo(const QString &key, const QString &cmd, const QString &debugFile = QString(""));

    /**
     * @brief cmdInfo:获取命令的解析结果
     * @return 解析结果以map形式返回
     */
    QMap<QString, QList<QMap<QString, QString> > > &cmdInfo();

private:
    /**
     * @brief loadLshwInfo:加载lshw获取的信息
     * @param cmd:lshw
     * @param debugFile:调试文件名
     */
    void loadLshwInfo(const QString &cmd, const QString &debugFile = QString("lshw.txt"));

    /**
     * @brief loadLsblkInfo:加载lsblk -d -o name,rota获取的信息
     * @param cmd:lsblk -d -o name,rota
     * @param debugfile:调试文件名
     */
    void loadLsblkInfo(const QString &cmd, const QString &debugfile);

    /**
     * @brief loadSmartCtlInfo:加载smartctl获取的信息
     * @param cmd:smartctl
     * @param logicalName:逻辑名称
     * @param debugfile:调试文件名
     */
    void loadSmartCtlInfo(const QString &cmd, const QString &logicalName, const QString &debugfile);

    /**
     * @brief loadXrandrInfo:加载xrandr获取的信息
     * @param cmd:xrandr
     * @param debugfile:调试文件名
     */
    void loadXrandrInfo(const QString &cmd, const QString &debugfile);

    /**
     * @brief loadXrandrVerboseInfo:加载xrandr --verbose获取的信息
     * @param cmd:xrandr --verbose
     * @param debugfile:调试文件名
     */
    void loadXrandrVerboseInfo(const QString &cmd, const QString &debugfile);

    /**
     * @brief loadDmesgInfo:加载dmesg获取的信息
     * @param cmd:dmesg
     * @param debugfile:调试文件名
     */
    void loadDmesgInfo(const QString &cmd, const QString &debugfile);

    /**
     * @brief loadHciconfigInfo:加载hciconfig -a获取的信息
     * @param cmd:hciconfig -a
     * @param debugfile:调试文件名
     */
    void loadHciconfigInfo(const QString &cmd, const QString &debugfile);

    /**
     * @brief loadBluetoothCtlInfo:这个函数是对LoadHciconfigInfo的扩展
     * @param mapInfo:hciconfig -a获取的信息map
     */
    void loadBluetoothCtlInfo(QMap<QString, QString> &mapInfo); // 这个函数是对LoadHciconfigInfo的扩展

    /**
     * @brief loadPrinterInfo:加载打印机信息
     */
    void loadPrinterInfo();

    /**
     * @brief loadHwinfoInfo:加载hwinfo信息
     * @param key:与cmd对应的关键字
     * @param cmd:hwinfo --xxx
     * @param debugfile:调试文件名
     */
    void loadHwinfoInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadHwinfoUsbInfo:加载hwinfo --usb信息
     * @param item:段落信息
     * @param mapInfo:hwinfo --usb解析得到的信息map
     */
    void loadHwinfoUsbInfo(const QString &item, const QMap<QString, QString> &mapInfo);

    /**
     * @brief loadDmidecodeInfo:加载dmidecode信息
     * @param key:与cmd对应的关键字
     * @param cmd:dmidecode -t xxx
     * @param debugfile:调试文件名
     */
    void loadDmidecodeInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadDmidecode2Info:加载dmidecode -t 2信息
     * @param key:dmidecode2
     * @param cmd:dmidecode -t 2
     * @param debugfile:调试文件名
     */
    void loadDmidecode2Info(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadCatInfo:加载cat xxx信息
     * @param key:与cmd对应的关键字
     * @param cmd: cat xxx
     * @param debugfile:调试文件名
     */
    void loadCatInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadUpowerInfo:加载upower --dump
     * @param key:upower
     * @param cmd:upower --dump
     * @param debugfile:调试文件名
     */
    void loadUpowerInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadBiosInfoFromLspci:从lspci加载BIOS芯片组信息
     * @param chipsetFamliy:【out】芯片组信息
     */
    void loadBiosInfoFromLspci(QString &chipsetFamliy);

    /**
     * @brief loadCatInputDeviceInfo:加载cat /proc/bus/input/devices信息
     * @param key:cat_devices
     * @param cmd:cat /proc/bus/input/devices
     * @param debugfile:调试文件名
     */
    void loadCatInputDeviceInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadCatAudioInfo:加载cat /proc/asound/card0/codec#0信息
     * @param key:cat_audio
     * @param cmd:cat /proc/asound/card0/codec#0
     * @param debugfile:调试文件名
     */
    void loadCatAudioInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadEdidInfo
     * @param key:EDID_HDMI、EDID_VGA
     * @param cmdhexdump /sys/devices/platform/hisi-drm/drm/card0/card0-HDMI-A-1/edid
     * hexdump /sys/devices/platform/hisi-drm/drm/card0/card0-VGA-1/edid
     * @param debugfile:调试文件名
     */
    void loadEdidInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadGpuInfo:加载gpuinfo信息
     * @param key:gpuinfo
     * @param cmd:gpuinfo
     * @param debugfile:调试文件名
     */
    void loadGpuInfo(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief loadBootDeviceManfid:加载本机自带硬盘
     * @param key:bootdevice
     * @param cmd:cat /proc/bootdevice/product_name
     * @param debugfile:调试文件名
     */
    void loadBootDeviceManfid(const QString &key, const QString &cmd, const QString &debugfile);

    /**
     * @brief getSMBIOSVersion:获取SMBIOS版本号
     * @param info:dmidecode获取的片断信息
     * @param version:SMBIOS版本号
     */
    void getSMBIOSVersion(const QString &info, QString &version);

private:

    /**
     * @brief getMapInfoFromCmd:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromCmd(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromInput:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromInput(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromLshw:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromLshw(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromHwinfo:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromHwinfo(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromDmidecode:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromDmidecode(const QString &info, QMap<QString, QString> &mapInfo, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromSmartctl:将通过命令获取的信息字符串，转化为map形式
     * @param info:命令获取的信息字符串
     * @param mapInfo:解析字符串保存为map形式
     * @param ch:分隔符
     */
    void getMapInfoFromSmartctl(QMap<QString, QString> &mapInfo, const QString &info, const QString &ch = QString(": "));

    /**
     * @brief getMapInfoFromHciconfig:将通过命令获取的信息字符串，转化为map形式
     * @param mapInfo:解析字符串保存为map形式
     * @param info:命令获取的信息字符串
     */
    void getMapInfoFromHciconfig(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief getMapInfoFromBluetoothCtl:将通过命令获取的信息字符串，转化为map形式
     * @param mapInfo:解析字符串保存为map形式
     * @param info:命令获取的信息字符串
     */
    void getMapInfoFromBluetoothCtl(QMap<QString, QString> &mapInfo, const QString &info);

    /**
     * @brief addMapInfo:添加map信息
     * @param key:与命令对应的关键字
     * @param mapInfo:解析得到的设备信息map
     */
    void addMapInfo(const QString &key, const QMap<QString, QString> &mapInfo);

    /**
     * @brief getMapInfo:解析打印机cups第三方库获取的信息
     * @param mapInfo:解析得到的map信息
     * @param src:cups获取的原始信息
     */
    void getMapInfo(QMap<QString, QString> &mapInfo, cups_dest_t *src);

    /**
     * @brief getDeviceInfo:通过命令获取设备信息字符串
     * @param command:命令
     * @param deviceInfo:设备信息
     * @param debugFile:调试文件名称
     * @return
     */
    bool getDeviceInfo(const QString &command, QString &deviceInfo, const QString &debugFile = QString(""));

    /**
     * @brief executeProcess执行命令获取设备信息
     * @param cmd:命令
     * @param deviceInfo:设备信息命令执行结果
     * @return 布尔值:true执行成功；false执行失败
     */
    bool executeProcess(const QString &cmd, QString &deviceInfo);

    /**
     * @brief runCmd:运行命令 获取设备信息
     * @param cmd:命令
     * @param deviceInfo:设备信息命令执行结果
     * @return 布尔值:true执行成功；false执行失败
     */
    bool runCmd(const QString &cmd, QString &deviceInfo);

    /**
     * @brief getPKStr:生成授权码
     * @param dtStr:时间日期字符串格式
     * @param dtInt:时间日期int格式
     * @return 授权码
     */
    QString getPKStr(const QString &dtStr, const QString &dtInt);

    /**
     * @brief getPKStr:调试生成授权码
     * @param dtStr:解析时间日期字符串
     * @param dtInt:解析日期时间int格式
     * @param cStr:授权码
     */
    void getPKStr(QString &dtStr, QString &dtInt, const QString &cStr);

private:
    QMap<QString, QList<QMap<QString, QString> > > m_cmdInfo;
};

#endif // CMDTOOL_H
