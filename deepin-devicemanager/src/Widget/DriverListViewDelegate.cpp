/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DriverListViewDelegate.h"

// DTK
#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>
#include <DFontSizeManager>

// Qt
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QPainterPath>
#include <QFont>

DWIDGET_USE_NAMESPACE

DriverListViewDelegate::DriverListViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

void DriverListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        const QWidget *widget = option.widget;
        QStyle *style = widget ? widget->style() : QApplication::style();
        opt.text = "";
        opt.icon = QIcon();
        opt.palette.setBrush(QPalette::Highlight, Qt::transparent);
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QFont font;
    font = DFontSizeManager::instance()->t8();
    painter->setFont(font);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            cg = DPalette::Active;
        }
    }
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);
    //设置高亮文字色
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));


    painter->setPen(forground);
    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QRect textRect = rect;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        // 左间距
        rect.setX(rect.x() + margin);
    } break;
    case QStyleOptionViewItem::Middle: {
    } break;
    case QStyleOptionViewItem::End: {
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        // 左间距
        rect.setX(rect.x() + margin);
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }
    //绘制图标
    QRect iconRect = rect;
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning &&
            index.data(Qt::DecorationRole).isValid()) {
        iconRect.setWidth(30);
        iconRect.setHeight(30);
        iconRect.setX(rect.x()+6);
        iconRect.setY(rect.y()+6);
        QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();
        ic.paint(painter, iconRect, Qt::AlignVCenter);
    }
    //绘制文字
    textRect = rect;
    textRect.setX(textRect.x() + margin - 2);
    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

QSize DriverListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(36);
    return size;
}
