// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DPaletteHelper>
#include <DPalette>

#include "headerinfotableDelegate.h"

DWIDGET_USE_NAMESPACE

HeaderInfoDelegate::HeaderInfoDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void HeaderInfoDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg = wnd ? DPalette::Active : DPalette::Inactive;
    auto palette = DPaletteHelper::instance()->palette(option.widget);

    if (opt.state & QStyle::State_Selected) {
        QColor highlightColor = palette.color(cg, DPalette::HighlightedText);
        opt.palette.setColor(QPalette::Text, highlightColor);
        opt.palette.setColor(QPalette::WindowText, highlightColor);
    } else {
        QColor normalColor = palette.color(cg, DPalette::Text);
        opt.palette.setColor(QPalette::Text, normalColor);
        opt.palette.setColor(QPalette::WindowText, normalColor);
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
