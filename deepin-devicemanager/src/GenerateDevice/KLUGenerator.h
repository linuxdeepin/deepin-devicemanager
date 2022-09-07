// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef KLUGENERATOR_H
#define KLUGENERATOR_H

#include"DeviceGenerator.h"

/**
 * @brief The KLUGenerator class
 * 将获取的设备信息生成设备对象，KLU笔记本下的生成器
 */
class KLUGenerator : public DeviceGenerator
{
public:
    KLUGenerator();

protected:
    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice() override;

    /**
     * @brief generatorGpuDevice:生成显卡信息
     */
    virtual void generatorGpuDevice() override;

    /**
     * @brief generatorMonitorDevice:生成显示设备
     */
    virtual void generatorMonitorDevice() override;

    /**
     * @brief generatorAudioDevice:生成音频适配器
     */
    virtual void generatorAudioDevice() override;

    /**
     * @brief generatorPowerDevice:生成电池设备
     */
    virtual void generatorPowerDevice() override;

protected:
    /**
     * @brief getKeyboardInfoFromHwinfo:从hwinfo获取键盘信息
     */
    virtual void getKeyboardInfoFromHwinfo() override;

    /**
     * @brief getOthersInfoFromHwinfo:从hwinfo中获取其他设备信息
     */
    virtual void getOthersInfoFromHwinfo() override;

    /**
     * @brief getDiskInfoFromHwinfo:从hwinfo获取存储设备信息
     */
    virtual void getDiskInfoFromHwinfo() override;

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw() override;

    /**
     * @brief getDiskInfoFromLsblk:从lsblk获取存储设备信息
     */
    virtual void getDiskInfoFromLsblk() override;

    /**
     * @brief getDiskInfoFromSmartCtl:从smartctl获取存储设备信息
     */
    virtual void getDiskInfoFromSmartCtl() override;

    /**
     * @brief getAudioInfoFromCatInput:从cat /proc/bus/input/devices获取音频设备信息
     */
    virtual void getAudioInfoFromCatAudio();
};

#endif // KLUGENERATOR_H
