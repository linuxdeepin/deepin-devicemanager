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

class MemoryWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit MemoryWidget(QWidget *parent = nullptr);

    void initWidget() override;

    // "设 计 师"
    void init_l_Designer_l_TableWdiget();
    bool update_l_Designer_l_WholeDownWidget();

    void initTableWdiget_Good();
    void updateWholeDownWidget_Good();

    bool isSlotValid(const QString &size, const QString &speed );
    //确定有几个插槽插有内存条
    int hasNumerMemory();
    //从sysinfo中的DBus获取总的内存大小
    double totalMemorySize();
    static QString formatCap(qulonglong cap,const int size = 1024,quint8 precision = 1);

private:
    int canUpgrade_ = -1;
    QList<ArticleStruct> summaryInfo_;
    QMap<QString,int> sizeMap;
};
