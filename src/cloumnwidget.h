/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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

#include "DWidget"
#include <QContextMenuEvent>
#include <QClipboard>
#include "DLabel"
#include "deviceinfowidgetbase.h"
#include "DPalette"

class ColumnWidget;

class ColumnLabel: public Dtk::Widget::DLabel
{
public:
    ColumnLabel(const QString& text = "", ColumnWidget* parent = nullptr);

private:
    //void mousePressEvent(QMouseEvent *event) override;
    //void contextMenuEvent(QContextMenuEvent *event) override;
private:
    ColumnWidget* columnWidget_ = nullptr;
};

class ColumnWidget: public Dtk::Widget::DWidget
{
public:
    ColumnLabel* l1 = nullptr;
    ColumnLabel* l2 = nullptr;
    bool st = false;

public:
    ColumnWidget(const QString& strLeft, const QString& strRight, const QFont& font, int fontSizetype, int columnHeight, bool isRightLink = false, Dtk::Widget::DWidget* parent = nullptr, DeviceInfoWidgetBase* di = nullptr);

    //void labelMousePressEvent(QMouseEvent *event);
    void labelContextMenuEvent(QContextMenuEvent *event);

    void setHilight(bool highLight);

    void changeTheme();

private:
    DeviceInfoWidgetBase* deviceInfoWidget_ = nullptr;

    static bool isPaletteInit_;
    static Dtk::Gui::DPalette paHighlight_;
    static Dtk::Gui::DPalette paNormal_;
};
