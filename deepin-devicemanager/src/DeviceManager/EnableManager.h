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

#ifndef ENABLEMANAGER_H
#define ENABLEMANAGER_H

#include <QObject>
#include "DeviceInfo.h"

/**
 * @brief The EnableManager class
 * 封装启用和禁用设备的接口
 */

class EnableManager : public QObject
{
    Q_OBJECT
public:
    static EnableManager *instance()
    {
        if (!s_Instance)
            s_Instance = new EnableManager;
        return s_Instance;
    }

    /**
     * @brief enableDeviceByInput : 用来启用禁用设备
     * @param name : 该设备名称
     * @return : 返回是否禁用成功
     */
    EnableDeviceStatus enableDeviceByInput(const QString &name, bool enable, int index = 0);

    /**
     * @brief isDeviceEnable : 判断设备是否可用
     * @param name:设备名称
     * @return true可用,false不可用
     */
    bool isDeviceEnable(const QString &name);

    /**
     * @brief isDeviceEnable:判断设备是否可用
     * @param id:设备ID
     * @return true可用,false不可用
     */
    bool isDeviceEnable(int id);

    /**
     * @brief enableDeviceByDriver:过驱动禁用设备
     * @param enable:设备禁用false,启用true
     * @param driver:驱动名称
     * @return 设备禁用状态
     */
    EnableDeviceStatus enableDeviceByDriver(bool enable, const QString &driver);

    /**
     * @brief isDeviceEnableByDriver:判断是否通过驱动禁用设备
     * @param name:设备名称
     * @return true:该设备通过驱动禁用
     */
    bool isDeviceEnableByDriver(const QString &driver);

    /**
     * @brief enablePrinter : 禁用打印机
     * @param name : 打印机名称
     * @return　: 禁用是否成功
     */
    EnableDeviceStatus enablePrinter(const QString &name, bool enable);

    /**
     * @brief enableNetworkByIfconfig : 禁用网络设备
     * @param logicalName : 该网卡的逻辑名称
     * @param enable:false禁用,true启用
     * @return 设备禁用状态
     */
    EnableDeviceStatus enableNetworkByIfconfig(const QString &logicalName, bool enable);

    /**
     * @brief isNetworkEnableByIfconfig　: 判断该设备是否被禁用
     * @param logicalName : 该网卡的逻辑名称
     * @return true:已被禁用,false:未被禁用
     */
    bool isNetworkEnableByIfconfig(const QString &logicalName);

    /**
     * @brief getDeviceID:获取设备ID号
     * @param name:设备名称
     * @param index:设备索引号
     * @return 设备ID
     */
    int getDeviceID(const QString &name, bool enable, int index);

protected:
    EnableManager();

private:

    /**
     * @brief getDriverPath:获取驱动路径
     * @param driver:驱动名称
     * @return 驱动路径
     */
    QString getDriverPath(const QString &driver);


private:
    static EnableManager *s_Instance;
};

#endif // ENABLEMANAGER_H
