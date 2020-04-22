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

class DiskWidget: public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit DiskWidget(QWidget *parent = nullptr);

    void initWidget() override;

    bool getDiskType(const QString& diskProperty, QString& type);

    // 从hwinfo --disk中获取数据
    void getHwinfoDiskList();

    // 设置介质类型
    // disk:从hwinfo --disk中获取的存储设备信息
    // diskType：从lsblk中的获取的存储设备类型信息
    QString setMediaType(QString &disk,QStringList &diskType);

    // 设置硬盘大小
    // disk:从hwinfo --disk中获取的存储设备信息
    ArticleStruct setDiskSizeInfo(QString &disk);

    // 设置硬盘Model信息
    // disk:从hwinfo --disk中获取的存储设备信息
    ArticleStruct setDiskModelInfo(QString &disk);

    // 添加 smartctl 中获取的其他属性
    void addOtherSmartctlInfo(QString &disk);

    // 设置硬盘借口信息
    ArticleStruct setDiskInterfaceInfo(QString &disk);

    // 设置硬盘序列号
    ArticleStruct setDiskSerialInfo(QString &disk);

private:
//    QMap<QString, QList<ArticleStruct>> m_articlesmap;
    QList<QStringList> m_tabList;
};

