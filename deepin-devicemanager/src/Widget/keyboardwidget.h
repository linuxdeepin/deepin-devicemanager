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
class DeviceKeyboard;

class KeyboardWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit KeyboardWidget(QWidget *parent = nullptr);
private:
    /**重构后的代码************************************************************/
    void initWidgetEX();
    void initTable(const QList<DeviceKeyboard>& devices);
    void initDetails(const QList<DeviceKeyboard>& devices);
    void addDeviceDetail(const DeviceKeyboard& device,bool withTitle = false);
    void setOverView(const QList<DeviceKeyboard>& devices);
    //***********************************************************************
};

