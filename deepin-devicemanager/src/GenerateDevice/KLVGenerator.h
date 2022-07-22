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

#ifndef KLVGENERATOR_H
#define KLVGENERATOR_H

#include"HWGenerator.h"

/**
 * @brief The KLVGenerator class
 * 将获取的设备信息生成设备对象，KLU笔记本下的生成器
 */
class KLVGenerator : public HWGenerator
{
public:
    KLVGenerator();

protected:
    /**
     * @brief generatorMonitorDevice:生成显示设备
     */
    virtual void generatorMonitorDevice() override;

    /**@brief:generator network info*/
    virtual void generatorNetworkDevice() override;

protected:
    /**@brief:generator network info*/
    virtual void getNetworkInfoFromCatWifiInfo();

    /**
     * @brief getDiskInfoFromLshw:从lshw获取存储设备信息
     */
    virtual void getDiskInfoFromLshw() override;
};

#endif // KLVGENERATOR_H
