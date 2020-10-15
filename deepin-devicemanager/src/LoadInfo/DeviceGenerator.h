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

#ifndef DEVICEGENERATOR_H
#define DEVICEGENERATOR_H

#include <QObject>
#include <DApplication>
#include <DSysInfo>
#include <QMutex>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

/**
 * @brief The DeviceGenerator class
 * 将获取的设备信息生成设备对象
 */
class DeviceGenerator : public QObject
{
    Q_OBJECT
public:
    explicit DeviceGenerator(QObject *parent = nullptr);
    virtual ~DeviceGenerator();

    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice();

    /**
     * @brief generatorCpuDevice:生成CPU设备
     */
    virtual void generatorCpuDevice();

    /**
     * @brief generatorBiosDevice:生成主板
     */
    virtual void generatorBiosDevice();

    /**
     * @brief generatorMemoryDevice:生成内存设备
     */
    virtual void generatorMemoryDevice();

    /**
     * @brief generatorDiskDevice生成存储设备信息
     */
    virtual void generatorDiskDevice();

    /**
     * @brief generatorGpuDevice:生成显卡信息
     */
    virtual void generatorGpuDevice();

    /**
     * @brief generatorMonitorDevice:生成显示设备
     */
    virtual void generatorMonitorDevice();

    /**
     * @brief generatorNetworkDevice:生成网卡
     */
    virtual void generatorNetworkDevice();

    /**
     * @brief generatorAudioDevice:生成音频适配器
     */
    virtual void generatorAudioDevice();

    /**
     * @brief generatorBluetoothDevice:生成蓝牙设备
     */
    virtual void generatorBluetoothDevice();

    /**
     * @brief generatorKeyboardDevice:生成键盘设备
     */
    virtual void generatorKeyboardDevice();

    /**
     * @brief generatorMouseDevice:生成鼠标设备
     */
    virtual void generatorMouseDevice();

    /**
     * @brief generatorPrinterDevice:生成打印机设备
     */
    virtual void generatorPrinterDevice();

    /**
     * @brief generatorCameraDevice:生成图像设备
     */
    virtual void generatorCameraDevice();

    /**
     * @brief generatorCdromDevice:生成CDrom设备
     */
    virtual void generatorCdromDevice();

    /**
     * @brief generatorOthersDevice:生成其他设备
     */
    virtual void generatorOthersDevice();

    /**
     * @brief generatorPowerDevice:生成电池设备
     */
    virtual void generatorPowerDevice();

    /**
     * @brief addBusIDFromHwinfo:添加从hwinfo中获取的BusID
     * @param sysfsBusID:被添加的BusID
     */
    void addBusIDFromHwinfo(const QString &sysfsBusID);

    /**
     * @brief getBusIDFromHwinfo:获取所有从hwinfo中添加的BusID
     * @return 所有BusID组成的StringList
     */
    const QStringList &getBusIDFromHwinfo();

protected:

    /**
     * @brief getBiosInfo:获取BIOS信息
     */
    virtual void getBiosInfo();

    /**
     * @brief getSystemInfo:获取系统信息
     */
    virtual void getSystemInfo();

    /**
     * @brief getBaseBoardInfo:获取主板信息
     */
    virtual void getBaseBoardInfo();

    /**
     * @brief getChassisInfo:获取机箱信息
     */
    virtual void getChassisInfo();

    /**
     * @brief getBiosMemoryInfo:获取内存插槽信息
     */
    virtual void getBiosMemoryInfo();

    /**
     * @brief getMemoryInfoFromLshw:从lshw获取内存信息
     */
    virtual void getMemoryInfoFromLshw();

    /**
     * @brief getMemoryInfoFromDmidecode:从dmidecode获取内存信息
     */
    virtual void getMemoryInfoFromDmidecode();

    /**
     * @brief getDiskInfoFromHwinfo:从hwinfo获取存储设备信息
     */
    virtual void getDiskInfoFromHwinfo();

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw();

    /**
     * @brief getDiskInfoFromLsblk:从lsblk获取存储设备信息
     */
    virtual void getDiskInfoFromLsblk();

    /**
     * @brief getDiskInfoFromSmartCtl:从smartctl获取存储设备信息
     */
    virtual void getDiskInfoFromSmartCtl();

    /**@brief:generator gpu info*/
    /**
     * @brief getGpuInfoFromHwinfo:从hwinfo获取显卡信息
     */
    virtual void getGpuInfoFromHwinfo();

    /**
     * @brief getGpuInfoFromLshw:从lshw获取显卡信息
     */
    virtual void getGpuInfoFromLshw();

    /**
     * @brief getGpuInfoFromXrandr:从xrandr获取显卡信息
     */
    virtual void getGpuInfoFromXrandr();

    /**
     * @brief getGpuSizeFromDmesg:从Dmesg获取显存信息
     */
    virtual void getGpuSizeFromDmesg();

    /**@brief:generator monitor info*/
    /**
     * @brief getMonitorInfoFromHwinfo:从hwinfo获取显示设备信息
     */
    virtual void getMonitorInfoFromHwinfo();

    /**
     * @brief getMonitorInfoFromXrandrVerbose:从xrandr --verbose获取显示设备信息
     */
    virtual void getMonitorInfoFromXrandrVerbose();

    /**
     * @brief getMonitorRefreshRateFromXrandr:从xrandr获取显示设备刷新率
     */
    virtual void getMonitorRefreshRateFromXrandr();

    /**@brief:generator audio info*/
    /**
     * @brief getAudioInfoFromHwinfo:从hwinfo获取声卡信息
     */
    virtual void getAudioInfoFromHwinfo();

    /**
     * @brief getAudioInfoFromLshw:从lshw获取声卡信息
     */
    virtual void getAudioInfoFromLshw();

    /**
     * @brief getAudioInfoFromCatInput:从cat /proc/bus/input/devices获取音频设备信息
     */
    virtual void getAudioInfoFromCatInput();

    /**
     * @brief getAudioChipInfoFromDmesg:从dmesg获取声卡芯片信息
     */
    virtual void getAudioChipInfoFromDmesg();

    /**
     * @brief getBluetoothInfoFromHciconfig:hciconfig获取蓝牙信息
     */
    virtual void getBluetoothInfoFromHciconfig();

    /**
     * @brief getBlueToothInfoFromHwinfo:从hwinfo获取蓝牙信息
     */
    virtual void getBlueToothInfoFromHwinfo();

    /**
     * @brief getBluetoothInfoFromLshw:从lshw获取蓝牙信息
     */
    virtual void getBluetoothInfoFromLshw();

    /**
     * @brief getKeyboardInfoFromHwinfo:从hwinfo获取键盘信息
     */
    virtual void getKeyboardInfoFromHwinfo();

    /**
     * @brief getKeyboardInfoFromLshw:从lshw获取键盘信息
     */
    virtual void getKeyboardInfoFromLshw();

    /**
     * @brief getKeyboardInfoFromCatDevices:从cat /proc/bus/input/devices获取键盘信息
     */
    virtual void getKeyboardInfoFromCatDevices();

    /**
     * @brief getMouseInfoFromHwinfo:从hwinfo中获取鼠标信息
     */
    virtual void getMouseInfoFromHwinfo();

    /**
     * @brief getMouseInfoFromLshw:从lshw中获取鼠标信息
     */
    virtual void getMouseInfoFromLshw();

    /**
     * @brief getMouseInfoFromCatDevices
     */
    virtual void getMouseInfoFromCatDevices();

    /**
     * @brief getImageInfoFromHwinfo:从hwinfo中获取图像设备信息
     */
    virtual void getImageInfoFromHwinfo();

    /**
     * @brief getImageInfoFromLshw:从lshw中获取图像设备信息
     */
    virtual void getImageInfoFromLshw();

    /**
     * @brief getCdromInfoFromHwinfo:从hwinfo中获取CDrom信息
     */
    virtual void getCdromInfoFromHwinfo();

    /**
     * @brief getCdromInfoFromLshw:从lshw中获取CDRom信息
     */
    virtual void getCdromInfoFromLshw();

    /**
     * @brief getOthersInfoFromHwinfo:从hwinfo中获取其他设备信息
     */
    virtual void getOthersInfoFromHwinfo();

    /**
     * @brief getOthersInfoFromLshw:从lshw中获取其他设备信息
     */
    virtual void getOthersInfoFromLshw();

protected:
    QStringList m_ListBusID;
};

#endif // DEVICEGENERATOR_H
