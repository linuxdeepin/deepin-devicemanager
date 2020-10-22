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
#include "DebugTimeManager.h"
#include "config.h"

const QString commonFontFamily_ = "Noto Sans CJK TC";
const int leftDeviceListViewMinWidth_ = 177;
const int NameLength_ = 120;
const int DeviceWidgetMargin_ = 10;
const int DeviceWidgetContentMarginLeft_ = 18;
const int DeviceWidgetContentMarginTop_ = 10;
const int DeviceWidgetContentMarginRight_ = 0;
const int DeviceWidgetContentMarginBottom_ = 20;
const int contextMenuWidth_ = 150;

#ifdef PERF_ON
#define PERF_PRINT_BEGIN(point, desc) DebugTimeManager::getInstance()->beginPointLinux(point, desc)
#define PERF_PRINT_END(point) DebugTimeManager::getInstance()->endPointLinux(point)
#define END_SUB_POINT(point) DebugTimeManager::getInstance()->endSubPoint(point)
#define PERF_PRINT_END_SUB(point,sub) DebugTimeManager::getInstance()->endPointLinux(point,sub)
#else
#define PERF_PRINT_BEGIN(point,desc)
#define PERF_PRINT_END(point)
#define END_SUB_POINT(point)
#define PERF_PRINT_END_SUB(point,sub)
#endif

#define GenerateTsItem 0
//#define TEST_DATA_FROM_FILE
const QString DEVICEINFO_PATH = "/home/jxm/Desktop/info/my_deviceinfo";
