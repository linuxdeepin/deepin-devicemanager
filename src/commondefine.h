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

//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deepin_uos_arm64_nvme_disk";
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_12417_cpu_bug";//cpu dominant frequency is zero
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_dmidecode_multiProcessor";//dmidecode show lots of processor information
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/longxin_3A400_notebook";//monitor size is in error
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_FT2000-64_server_bug13623";//unclear about usb and power
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_chuangzhicheng_memory_unknow";
//const QString DEVICEINFO_PATH = "../../dde_devicemanager/computers/deviceInfo_loongson_memory";
//const QString DEVICEINFO_PATH = "/home/archermind/Desktop/deviceInfo_chuangzhicheng_cpu_zero";

//const QString DEVICEINFO_PATH = "/home/archermind/dde_devicemanager/computers/loongson_unknown_nvme_";
//const QString DEVICEINFO_PATH = "/home/archermind/dde_devicemanager/computers/Meegopad_T02"; //bluetooth info
//const QString DEVICEINFO_PATH = "/home/archermind/Desktop/deviceinfo_haikangweishi";

//deviceInfo_bug_12495;dmidecode has no memory device
//4Good_GM600;memory has bank
//LENOVO_ThinkCentre M910t-N000_4 //test case like *-class UNCLAIMED in lshw
//CF420Z deviceinfo  longxin_hardware m630  t630 ;disk bug
//test_monitor_refresh_rate,only xrandr.txt and hwinfo.txt come from the target computer
//const QString DEVICEINFO_PATH = "/dde_devicemanager/computers/test_monitor_refresh_rate";

//#define TEST_DATA_FROM_FILE
