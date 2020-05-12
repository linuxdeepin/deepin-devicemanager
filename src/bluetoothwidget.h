/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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

#pragma once

#include "deviceinfowidgetbase.h"

class BluetoothWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit BluetoothWidget(QWidget *parent = nullptr);
    void initWidget() override;

    void initWidgetEX();

    /**
     * @brief:从命令中获取蓝牙列表
     * @return[bool]:如果没有蓝牙则返回false
     */
    bool getBlueToothList(QStringList &lshwList, QStringList &hcionfigList);

    /**
     * @brief[getBlueToothsInfo]:根据蓝牙设备列表获取蓝牙设备信息
     * @brief[getBlueToothInfo]:根据蓝牙设备名称获取蓝牙设备信息
     * @param[in]:lshwList/hcionfigList   蓝牙设备列表
     * @param[in]:lshwitem/hcionfigitem
     * @param[in/out]:tabList --> 蓝牙设备的基本信息，也就是存放在表格里面的内容
     */
    void getBlueToothsInfo(const QStringList &lshwList, const QStringList &hcionfigList, QList<QStringList> &tabList);
    void getBlueToothInfo(const QString &lshwitem, const QString &hcionfigitem, QList<ArticleStruct> &articles, QSet<QString> &existArticles, QSet<QString> &existArticles2, QSet<QString> &existArticles3, QStringList &tabItem);

    /**
     * @brief:获取总览信息
     */
    void getOverviewInfo(QList<QStringList> &lshwList);
};

