// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverListViewDelegate.h"
#include "DDLog.h"

// DTK
#include <DApplication>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>
#include <DFontSizeManager>

// Qt
#include <QLoggingCategory>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QPainterPath>
#include <QFont>

DWIDGET_USE_NAMESPACE

using namespace DDLog;

DriverListViewDelegate::DriverListViewDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    qCDebug(appLog) << "DriverListViewDelegate instance created";
}

void DriverListViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qCDebug(appLog) << "Painting driver list view delegate";
    if (!index.isValid()) {
        qCWarning(appLog) << "Invalid index, painting default";
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }
    {
        QStyleOptionViewItem opt = option;
        opt.state = opt.state & (~QStyle::State_Selected);   //bug  106152 去掉选中的option state
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
        // qCDebug(appLog) << "Item is disabled";
    } else {
        if (!wnd) {
            cg = DPalette::Inactive;
            // qCDebug(appLog) << "Window is inactive";
        } else {
            cg = DPalette::Active;
        }
    }
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);
    //设置高亮文字色
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));


    painter->setPen(forground);
    QRect rect = opt.rect;
    QFontMetrics fm(opt.font);
    QRect textRect = rect;
    switch (opt.viewItemPosition) {
    case QStyleOptionViewItem::Beginning: {
        // qCDebug(appLog) << "Painting first item";
        // 左间距
        rect.setX(rect.x() + margin);
    } break;
    case QStyleOptionViewItem::Middle: {
        // qCDebug(appLog) << "Painting middle item";
    } break;
    case QStyleOptionViewItem::End: {
        // qCDebug(appLog) << "Painting last item";
        // 右间距
        rect.setWidth(rect.width() - margin);
    } break;
    case QStyleOptionViewItem::OnlyOne: {
        // qCDebug(appLog) << "Painting only one item";
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
        // qCDebug(appLog) << "Painting icon for first item";
        iconRect.setWidth(30);
        iconRect.setHeight(30);
        iconRect.setX(rect.x() + 6);
        iconRect.setY(rect.y() + 6);
        QIcon ic = index.data(Qt::DecorationRole).value<QIcon>();
        ic.paint(painter, iconRect, Qt::AlignVCenter);
    }
    //绘制文字
    textRect = rect;
    textRect.setX(textRect.x() + margin - 2);
    QString text = fm.elidedText(opt.text, Qt::ElideMiddle, textRect.width());
    painter->drawText(textRect, Qt::TextSingleLine | static_cast<int>(opt.displayAlignment), text);
    painter->restore();
}

QSize DriverListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qCDebug(appLog) << "Getting size hint";
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(36);
    return size;
}
