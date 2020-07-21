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

#include "logtreeview.h"
#include <DApplication>
#include <DApplicationHelper>
#include <DStyledItemDelegate>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QScrollBar>
//#include "structdef.h"

DWIDGET_USE_NAMESPACE

LogTreeView::LogTreeView(QWidget *parent)
    : DTreeView(parent)
    , mp_Model(nullptr)
    , mp_ItemDelegate(nullptr)
    , mp_HeaderView(nullptr)
{
    initUI();
}

void LogTreeView::setHeaderLabels(const QStringList &lst)
{
    if (mp_Model) {
        mp_Model->setHorizontalHeaderLabels(lst);
    }
}

void LogTreeView::setItem(int row, int column, QStandardItem *item)
{
    if (mp_Model) {
        mp_Model->setItem(row, column, item);

        // 设置每一行等宽
        int colCount = mp_HeaderView->count();
        int avgColWidth = width() / colCount;
        for (int i = 0; i < colCount; i++) {
            setColumnWidth(i, avgColWidth);
        }
    }
}

void LogTreeView::clear()
{
    if (mp_Model)
        mp_Model->clear();
}

void LogTreeView::initUI()
{
    mp_Model = new QStandardItemModel(this);
    setModel(mp_Model);

    mp_ItemDelegate = new LogViewItemDelegate(this);
    setItemDelegate(mp_ItemDelegate);

    mp_HeaderView = new LogViewHeaderView(Qt::Horizontal, this);
    setHeader(mp_HeaderView);

    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setRootIsDecorated(false);

    // this is setting is necessary,because scrollperpixel is default in dtk!!
    this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    this->header()->setStretchLastSection(true);

    this->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->header()->setFixedHeight(37);
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setFrameStyle(QFrame::NoFrame);
    this->viewport()->setAutoFillBackground(false);

    setAlternatingRowColors(false);
    setAllColumnsShowFocus(false);
    //  setFocusPolicy(Qt::TabFocus);
}

void LogTreeView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    //    auto style = dynamic_cast<DStyle *>(DApplication::style());
    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionFrame option;
    initStyleOption(&option);
    //    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    QRect rect = viewport()->rect();
    QRectF clipRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    //    clipPath.addRoundedRect(clipRect, radius, radius);
    //    subPath.addRect(subRect);
    //    clipPath = clipPath.subtracted(subPath);
    clipPath.addRect(rect);

    painter.fillPath(clipPath, bgBrush);

    painter.restore();

    DTreeView::paintEvent(event);
}

void LogTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
#endif
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());

    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);

    auto palette = options.palette;
    QBrush background;
    if (!(index.row() & 1)) {
        background = palette.color(cg, DPalette::AlternateBase);
    } else {
        background = palette.color(cg, DPalette::Base);
    }
    if (options.state & DStyle::State_Enabled) {
        if (selectionModel()->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // draw row background
    QPainterPath path;
    QRect rowRect { options.rect.x() - header()->offset(),
                    options.rect.y() + 1,
                    header()->length() - header()->sectionPosition(0),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    QTreeView::drawRow(painter, options, index);

    // draw focus
//    if (hasFocus() && currentIndex().row() == index.row()) {
//        QStyleOptionFocusRect o;
//        o.QStyleOption::operator=(options);
//        o.state |= QStyle::State_KeyboardFocusChange | QStyle::State_HasFocus;
//        o.rect = style->visualRect(layoutDirection(), viewport()->rect(), rowRect);
//        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, painter);
//    }

    painter->restore();
}

void LogTreeView::keyPressEvent(QKeyEvent *event)
{
    DTreeView::keyPressEvent(event);
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        emit clicked(this->currentIndex());
    }
}
