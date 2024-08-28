// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef M009GENERATOR_H
#define M009GENERATOR_H

#include <QObject>
#include "HWGenerator.h"
#include "PanguVGenerator.h"

/**
 * @brief The PanguVGenerator class
 * 将获取的设备信息生成设备对象，M009 下的生成器
 */

class M009Generator : public PanguVGenerator  //HWGenerator
{
public:
    M009Generator();

    /**
     * @brief generatorMonitorDevice:生成显示设备信息
     */
    virtual void generatorMonitorDevice() override;
protected:
    virtual void generatorNetworkDevice() override;
    virtual QStringList getNetworkInfoFromifconfig() override;
    virtual void getAudioInfoFromCatAudio() override;  /*this is for PAGU M009 special, conflict to master*/
    virtual void generatorCpuDevice() override;  /*this is for PAGU M009 special, conflict to master*/
};

#endif // M009GENERATOR_H
