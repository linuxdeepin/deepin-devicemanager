#ifndef KLUGENERATOR_H
#define KLUGENERATOR_H

#include"DeviceGenerator.h"
class KLUGenerator : public DeviceGenerator
{
public:
    KLUGenerator();

protected:
    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice();

    /**
     * @brief generatorGpuDevice:生成显卡信息
     */
    virtual void generatorGpuDevice();

    /**
     * @brief generatorMonitorDevice:生成显示设备
     */
    virtual void generatorMonitorDevice();

    /**
     * @brief generatorAudioDevice:生成音频适配器
     */
    virtual void generatorAudioDevice();

    /**
     * @brief generatorPowerDevice:生成电池设备
     */
    virtual void generatorPowerDevice();

protected:
    /**
     * @brief getKeyboardInfoFromHwinfo:从hwinfo获取键盘信息
     */
    virtual void getKeyboardInfoFromHwinfo();

    /**
     * @brief getOthersInfoFromHwinfo:从hwinfo中获取其他设备信息
     */
    virtual void getOthersInfoFromHwinfo();

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

    /**
     * @brief getAudioInfoFromCatInput:从cat /proc/bus/input/devices获取音频设备信息
     */
    virtual void getAudioInfoFromCatAudio();
};

#endif // KLUGENERATOR_H
