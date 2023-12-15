// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef M900GENERATOR_H
#define M900GENERATOR_H

#include <QObject>
#include "HWGenerator.h"
#include "PanguVGenerator.h"

/**
 * @brief The PanguVGenerator class
 * 将获取的设备信息生成设备对象，M900 下的生成器
 */

class M900Generator : public PanguVGenerator  //HWGenerator
{
public:
    M900Generator();


protected:
    virtual void generatorNetworkDevice() override;
    virtual QStringList getNetworkInfoFromifconfig() override;
    virtual void getAudioInfoFromCatAudio() override;  /*this is for PAGU M900 special, conflict to master*/
    virtual void generatorCpuDevice() override;  /*this is for PAGU M900 special, conflict to master*/
    virtual void generatorMonitorDevice() override;
};

#endif // M900GENERATOR_H