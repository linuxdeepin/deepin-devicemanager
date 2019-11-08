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
#include "DTableWidget"
#include "DHeaderView"
#include <DStyledItemDelegate>

//class TableWidgetAlwaysActiveHeaderViewDelegate : public Dtk::Widget::DStyledItemDelegate
//{
//    Q_OBJECT
//public:
//    TableWidgetAlwaysActiveHeaderViewDelegate(QAbstractItemView *parent = nullptr);

//    //void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//};

//class TableWidgetAlwaysActiveHeaderView: public Dtk::Widget::DHeaderView
//{
//public:
//    TableWidgetAlwaysActiveHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

//private:
//    //void paintEvent(QPaintEvent *e) override;
//    QStyleOptionViewItem viewOptions() const override;
//};



class TableWidgetAlwaysActive: public Dtk::Widget::DTableWidget
{
public:
    TableWidgetAlwaysActive(QWidget *parent = nullptr);

private:
    QStyleOptionViewItem viewOptions() const override;
};
