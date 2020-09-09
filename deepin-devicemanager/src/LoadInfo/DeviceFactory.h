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

#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include<QString>
class DeviceGenerator;

/**
 * @brief The DeviceFactory class
 * 用于获取当前的系统架构，并生成对应的生成器
 */

class DeviceFactory
{
public:
    /**
     * @brief getDeviceGenerator:获取设备生成器指针
     * @return  设备生成器指针:
     */
    static DeviceGenerator *getDeviceGenerator();

    /**
     * @brief setGeneratorKey:设置生成器的关键字
     * @param key:关键字
     */
    static void setGeneratorKey(const QString &key);

protected:
    DeviceFactory();

private:
    static QString s_GeneratorKey;
};

#endif // DEVICEFACTORY_H
