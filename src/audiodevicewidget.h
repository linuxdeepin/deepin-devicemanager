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

class AudiodeviceWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit AudiodeviceWidget(QWidget *parent = nullptr);
    void initWidget() override;

    /**
     * @brief:获取音频设备列表
     * @param[in/out]:lshwList --> 从lshw中获取的音频信息
     * @param[in/out]:catList  --> 从cat /proc/bus/input/devices 中获取音频信息
     * @return[int]:返回一共有多少个音频，lshwList + catList
     */
    int getAudioList(QStringList& lshwList,QStringList& catList);
    void addLshwAudioInfo(const QString& item,QStringList& tabItem);
    void addCatAudioInfo(const QString& item,QStringList& tabItem);
    void getBusInfoAndName(const QString& item,ArticleStruct& busInfo,ArticleStruct& name,QSet<QString>& existArticles);

    /**
     * @brief:从lshw中获取音频设备信息
     * @param[in/out]:tabList --> 用于记录存储在表格里面的内容
     */
    void getInfoFromLshw(QList<QStringList>& tabList,const QStringList& lshwList);

    /**
     * @brief:从cat /proc/bus/input/devices中获取音频设备信息
     * @param[in/out]:tabList --> 用于记录存储在表格里面的内容
     */
    void getInfoFromCatInput(QList<QStringList>& tabList,const QStringList& catList);
};

