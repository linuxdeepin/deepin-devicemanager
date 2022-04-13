/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef ENABLEUTILS_H
#define ENABLEUTILS_H

#include <QString>

class EnableUtils
{
public:
    EnableUtils();

    /**
     * @brief disableDevice 禁用外设
     * @param info
     */
    static void disableOutDevice(const QString& info);

    /**
     * @brief disableDevice 禁用外设
     */
    static void disableOutDevice();

    /**
     * @brief disableInDevice 禁用非外设
     */
    static void disableInDevice();

    /**
     * @brief ioctlOperateNetworkLogicalName 通过ioctl设置网卡是否可用
     * @param logicalName
     * @param enable
     * @return
     */
    static bool ioctlOperateNetworkLogicalName(const QString& logicalName, bool enable);

    /**
     * @brief getMapInfo 解析usb信息
     * @param item
     * @param mapInfo
     * @return
     */
    static bool getMapInfo(const QString& item,QMap<QString,QString>& mapInfo);
};

#endif // ENABLEUTILS_H
