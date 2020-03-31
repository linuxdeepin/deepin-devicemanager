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

class UsbdeviceWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit UsbdeviceWidget(QWidget *parent = nullptr);

    void initWidget() override;

    void loadUSBWidgetFromhwinfo();

    // 添加信息
//    ArticleStruct addArticleStruct(const QString &name, const QString &command, const QString &item, const QString &attri);

    // 当有多个usb信息时设置TabList
    void setTabList(int size, ArticleStruct &name, ArticleStruct &vendor, ArticleStruct &description);


private:
    QList<QStringList> m_tabList;
//    QList<ArticleStruct> m_articles;
//    QSet<QString> m_existArticles;
};

