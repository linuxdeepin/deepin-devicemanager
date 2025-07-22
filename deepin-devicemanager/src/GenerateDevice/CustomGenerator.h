// Copyright (C) 2025 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMGENERATOR_H
#define CUSTOMGENERATOR_H

#include "DeviceGenerator.h"

class CustomGenerator : public DeviceGenerator
{
public:
    CustomGenerator(QObject *parent = nullptr);

    /**
     * @brief generatorGpuDevice:生成显卡信息
     */
    void generatorGpuDevice() override;

    /**
     * @brief generatorMonitorDevice:生成显示设备信息
     */
    void generatorMonitorDevice() override;
};

#endif // CUSTOMGENERATOR_H
