// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

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

// QString::SkipEmptyParts was moved to Qt::SkipEmptyParts in Qt6
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#define QT_SKIP_EMPTY_PARTS QString::SkipEmptyParts
#elif QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define QT_SKIP_EMPTY_PARTS Qt::SplitBehaviorFlags(Qt::SkipEmptyParts)
#else
#define QT_SKIP_EMPTY_PARTS Qt::SkipEmptyParts
#endif
