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

#ifndef PAGEBOARDINFO_H
#define PAGEBOARDINFO_H

#include <QObject>
#include "PageSingleInfo.h"

class DeviceBios;
class RichTextDelegate;

/**
 * @brief The PageBoardInfo class
 * 主板的界面展示类
 */

class PageBoardInfo : public PageSingleInfo
{
    Q_OBJECT
public:
    explicit PageBoardInfo(QWidget *parent = nullptr);

    /**
     * @brief updateInfo:更新主板信息
     * @param lst:主板信息lst
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief loadDeviceInfo:加载设备信息
     * @param devices:设备列表
     * @param lst:设备信息列表
     */
    void loadDeviceInfo(const QList<DeviceBaseInfo *> &devices, const QList<QPair<QString, QString>> &lst);

    /**
     * @brief getOtherInfoPair:获取其他信息列表
     * @param lst:所有信息列表
     * @param lstPair:其他信息列表
     */
    void getOtherInfoPair(const QList<DeviceBaseInfo *> &lst, QList<QPair<QString, QString>> &lstPair);

    /**
     * @brief getKeyValueInfo 将设备信息直接转换为key-value值
     * @param device:设备指针
     * @param pair:信息键值对
     */
    void getValueInfo(DeviceBaseInfo *device, QPair<QString, QString> &pair);

    /**
     * @brief isBaseBoard:是否是主板界面
     * @return true:是主板界面
     */
    bool isBaseBoard() override {return true;}


private:
    RichTextDelegate     *mp_ItemDelegate;
};

#endif // PAGEBOARDINFO_H
