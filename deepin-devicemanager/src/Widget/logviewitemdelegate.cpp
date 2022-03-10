/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>
#include <QDebug>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QPainterPath>

#include "logviewitemdelegate.h"

DWIDGET_USE_NAMESPACE

LogViewItemDelegate::LogViewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void LogViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

//    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
//        if (!wnd) {
//            cg = DPalette::Inactive;
//        } else {
//            cg = DPalette::Active;
//        }
        cg = DPalette::Active;
    }

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option) - 4;

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;
    QPen forground;
    if (opt.features & QStyleOptionViewItem::Alternate) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    forground.setColor(palette.color(cg, DPalette::Text));
    if (opt.state & DStyle::State_Enabled) {
        if (opt.state & DStyle::State_Selected) {
            background = palette.color(cg, DPalette::Highlight);
            forground.setColor(palette.color(cg, DPalette::HighlightedText));
        }
    }
    painter->setPen(forground);

    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QPainterPath path, clipPath;
    QRect textRect = rect;

    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        rect.setX(rect.x() + margin);  // left margin

        QPainterPath rectPath, roundedPath;
        roundedPath.addRoundedRect(rect.x(), rect.y() + 1, rect.width() * 2, rect.height() - 2, radius,
                                   radius);
        rectPath.addRect(rect.x() + rect.width(), rect.y() + 1, rect.width(), rect.height() - 2);
        clipPath = roundedPath.subtracted(rectPath);
        painter->setClipPath(clipPath);
        path.addRect(rect);
    } break;
    case QStyleOptionViewItem::Middle: {
        QRect rect2(rect.left(), rect.top() + 1, rect.width(), rect.height() - 2);
        path.addRect(rect2);
    } break;
    case QStyleOptionViewItem::End: {
        rect.setWidth(rect.width() - margin);  // right margin

        QPainterPath rectPath, roundedPath;
        roundedPath.addRoundedRect(rect.x() - rect.width(), rect.y() + 1, rect.width() * 2,
                                   rect.height() - 2, radius, radius);
        rectPath.addRect(rect.x() - rect.width(), rect.y() + 1, rect.width(), rect.height() - 2);
        clipPath = roundedPath.subtracted(rectPath);
        painter->setClipPath(clipPath);
        path.addRect(rect);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        rect.setX(rect.x() + margin);          // left margin
        rect.setWidth(rect.width() - margin);  // right margin
        path.addRoundedRect(rect, radius, radius);
    } break;
    default: {
        painter->restore();
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    }
    painter->fillPath(path, background);

    QRect iconRect = rect;
    if (opt.viewItemPosition == QStyleOptionViewItem::Beginning &&
            index.data(Qt::DecorationRole).isValid()) {
        iconRect.setX(rect.x() - margin);
        iconRect.setWidth(64);
        QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();
        ic.paint(painter, iconRect);
    }
    textRect = rect;
    textRect.setX(textRect.x() + margin + 3);

    /*
    *@author yaobin
    *@date 209-12-21
    *@Modify Reason:省略符(...)后预留两个英文字母的空间
    */
    int leftElideSpace = fm.width(QString("yy"));
    textRect.setWidth(textRect.width() - leftElideSpace);

    QString text = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
    painter->drawText(textRect, static_cast<int>(opt.displayAlignment), text);

    painter->restore();
}

QWidget *LogViewItemDelegate::createEditor(QWidget *, const QStyleOptionViewItem &,
                                           const QModelIndex &) const
{
    return nullptr;
}

QSize LogViewItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(36, size.height()));
    return size;
}

void LogViewItemDelegate::initStyleOption(QStyleOptionViewItem *option,
                                          const QModelIndex &index) const
{
    option->showDecorationSelected = true;
    bool ok = false;
    if (index.data(Qt::TextAlignmentRole).isValid()) {
        uint value = index.data(Qt::TextAlignmentRole).toUInt(&ok);
        option->displayAlignment = static_cast<Qt::Alignment>(value);
    }

    if (!ok)
        option->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
    option->textElideMode = Qt::ElideRight;
    option->features = QStyleOptionViewItem::HasDisplay;
    if (index.row() % 2 == 0)
        option->features |= QStyleOptionViewItem::Alternate;
    if (index.data(Qt::DisplayRole).isValid())
        option->text = index.data().toString();
}
