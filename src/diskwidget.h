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
    void NoDiskFound();
    bool getDiskInfo();
    QList<QString> getAllDiskFromLsblk();


    bool getDiskType(const QString& diskProperty, QString& type);
private:
    QList<ArticleStruct> m_translatedArticles;
};

