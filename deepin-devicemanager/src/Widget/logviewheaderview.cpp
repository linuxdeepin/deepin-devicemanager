// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QLoggingCategory>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include "logviewheaderview.h"
#include "DDLog.h"

using namespace DDLog;

static const int kSpacingMargin = 4;

LogViewHeaderView::LogViewHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    qCDebug(appLog) << "LogViewHeaderView instance created";
    viewport()->setAutoFillBackground(false);
}
void LogViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    // qCDebug(appLog) << "Painting section";
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    DPalette::ColorGroup cg;
#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
    if (!wnd) {
        cg = DPalette::Inactive;
        // qCDebug(appLog) << "Window is inactive";
    } else {
        cg = DPalette::Active;
        // qCDebug(appLog) << "Window is active";
    }
#else
    cg = DPalette::Active;
#endif

    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style) {
        qCWarning(appLog) << "Failed to get DStyle";
        return;
    }

    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height(), rect.width(),
                       rect.height() - contentRect.height());

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // vertical spacing
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin, m_spacing,
                        rect.height() - kSpacingMargin * 2);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

    painter->fillRect(hSpacingRect, clearBrush);
    painter->fillRect(hSpacingRect, hSpacingBrush);

    if (visualIndex(logicalIndex) > 0) {
        painter->fillRect(vSpacingRect, clearBrush);
        painter->fillRect(vSpacingRect, vSpacingBrush);
    }

    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    // TODO: dropdown icon (8x5)
    QRect textRect;
    if (sortIndicatorSection() == logicalIndex) {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin * 3 - 8,
                    contentRect.height()
                   };
    } else {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin,
                    contentRect.height()};
    }
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    //    int align = model()->headerData(logicalIndex, orientation(),
    //    Qt::TextAlignmentRole).toInt();
    int align = Qt::AlignLeft | Qt::AlignVCenter;

    painter->setPen(forground);

    if (logicalIndex == 0) {
        QRect col0Rect = textRect;
        col0Rect.setX(textRect.x() + margin - 2);
        painter->drawText(col0Rect, static_cast<int>(align), title);
    } else {
        painter->drawText(textRect, static_cast<int>(align), title);
    }

    // sort indicator
    if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
        // qCDebug(appLog) << "Drawing sort indicator";
        // TODO: arrow size (8x5)
        QRect sortIndicator(textRect.x() + textRect.width() + margin,
                            textRect.y() + (textRect.height() - 5) / 2, 8, 5);
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }

    painter->restore();
}

void LogViewHeaderView::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "Painting event";
    QPainter painter(viewport());
    painter.save();

    DPalette::ColorGroup cg;
#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
    if (!wnd) {
        cg = DPalette::Inactive;
        // qCDebug(appLog) << "Window is inactive";
    } else {
        // qCDebug(appLog) << "Window is active";
        cg = DPalette::Active;
    }
#else
    cg = DPalette::Active;
#endif

    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style) {
        qCWarning(appLog) << "Failed to get DStyle";
        return;
    }

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionHeader option;
    initStyleOption(&option);
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    QRect rect = viewport()->rect();
    QRectF clipRect(rect.x(), rect.y(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    clipPath.addRoundedRect(clipRect, radius, radius);
    subPath.addRect(subRect);
    clipPath = clipPath.subtracted(subPath);

    painter.fillPath(clipPath, bgBrush);

    painter.restore();
    DHeaderView::paintEvent(event);
}

QSize LogViewHeaderView::sizeHint() const
{
    // qCDebug(appLog) << "Getting size hint";
    return QSize(width(), 36 + m_spacing);
}

int LogViewHeaderView::sectionSizeHint(int logicalIndex) const
{
    qCDebug(appLog) << "Getting section size hint";
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QFontMetrics fm(DApplication::font());
    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (sortIndicatorSection() == logicalIndex) {
        return fm.width(buf) + margin * 3 + 8;
    } else {
        return fm.width(buf) + margin * 2;
    }
#else
    if (sortIndicatorSection() == logicalIndex) {
        return fm.horizontalAdvance(buf) + margin * 3 + 8;
    } else {
        return fm.horizontalAdvance(buf) + margin * 2;
    }
#endif
}
