// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
