// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

const QString commonFontFamily_ = "Noto Sans CJK TC";
const int leftDeviceListViewMinWidth_ = 177;
const int NameLength_ = 120;
const int DeviceWidgetMargin_ = 10;
const int DeviceWidgetContentMarginLeft_ = 18;
const int DeviceWidgetContentMarginTop_ = 10;
const int DeviceWidgetContentMarginRight_ = 0;
const int DeviceWidgetContentMarginBottom_ = 20;
const int contextMenuWidth_ = 150;

#define GenerateTsItem 0
const QString DEVICEINFO_PATH = "/tmp/device-info";

constexpr const char* AUTH_ACTION = "com.deepin.deepin-devicemanager.checkAuthentication";

//  名称("Name") 厂商("Vendor") 型号("Model") 版本(Version) 显存("Graphics Memory")
constexpr char kName[] { "Name" };
constexpr char kVendor[] { "Vendor" };
constexpr char kModel[] { "Model" };
constexpr char kVersion[] { "Version" };
constexpr char kGraphicsMemory[] { "Graphics Memory" };
