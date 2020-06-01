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

#include <QString>

const QString commonFontFamily_ = "Noto Sans CJK TC";
const int mainWindowMinWidth_ = 1070;
const int mainWindowMinHeight_ = 790;

//const int mainWindowMinWidth_ = 768;
//const int mainWindowMinHeight_ = 562;

const int leftDeviceListViewMinWidth_ = 177;

const int NameLength_ = 120;

const int DeviceWidgetMargin_ = 10;

const int DeviceWidgetContentMarginLeft_ = 18;
const int DeviceWidgetContentMarginTop_ = 10;
const int DeviceWidgetContentMarginRight_ = 0;
const int DeviceWidgetContentMarginBottom_ = 20;

const int DeviceWidgetDownWidgehWidth_ = mainWindowMinWidth_ - leftDeviceListViewMinWidth_ - DeviceWidgetMargin_ * 2 - NameLength_ - DeviceWidgetContentMarginLeft_ - DeviceWidgetContentMarginRight_;
const int DeviceWidgetContentWidth_ = DeviceWidgetDownWidgehWidth_ - NameLength_;

const int contextMenuWidth_ = 150;

const int spinnerWidth = 32;
const int spinnerHeight = 32;

const int TableViewRowHeight_ = 36;
#define GenerateTsItem 0
//#define TEST_DATA_FROM_FILE

//const QString DEVICEINFO_PATH = "/home/liujun/Work/CommonlyUsed/newDeviceinfo/TongXin/LX_Keyboard";

