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
#ifndef MIPSGENERATOR_H
#define MIPSGENERATOR_H
#include "DeviceGenerator.h"

/**
 * @brief The MipsGenerator class
 * 将获取的设备信息生成设备对象，mips架构下的生成器
 */

class MipsGenerator : public DeviceGenerator
{
public:
    MipsGenerator();

    /**
     * @brief generatorComputerDevice:生成计算机概况
     */
    virtual void generatorComputerDevice() override;
};

#endif // MIPSGENERATOR_H
