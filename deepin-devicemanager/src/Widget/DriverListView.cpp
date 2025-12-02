// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DriverListView.h"
#include "DriverListViewDelegate.h"
#include "DDLog.h"

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DStyle>

#include <QHeaderView>
#include <QPainter>
#include <QStandardItemModel>
#include <QPainterPath>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <DApplicationHelper>
#endif

using namespace DDLog;

DriverListView::DriverListView(QWidget *parent) : DTreeView(parent)
{
    qCDebug(appLog) << "DriverListView instance created";

    initUI();
}

void DriverListView::initUI()
{
    qCDebug(appLog) << "Initializing UI";
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFrameStyle(QFrame::NoFrame);
    setHeaderHidden(true);
    DriverListViewDelegate *delegate = new DriverListViewDelegate(this);
    setItemDelegate(delegate);

    setRootIsDecorated(false);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void DriverListView::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "Painting event";
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);
    //根据窗口激活状态设置颜色
    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
        qCDebug(appLog) << "Window is inactive";
    } else {
        qCDebug(appLog) << "Window is active";
        cg = DPalette::Active;
    }
    auto *dAppHelper = DGuiApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    //绘制背景
    QStyleOptionFrame option;
    initStyleOption(&option);
    QRect rect = viewport()->rect();
    QRectF clipRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    clipPath.addRect(rect);
    painter.fillPath(clipPath, bgBrush);
    painter.restore();
    DTreeView::paintEvent(event);
}

void DriverListView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    // qCDebug(appLog) << "Drawing row";
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
        // qCDebug(appLog) << "Item is disabled";
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
            // qCDebug(appLog) << "Window is inactive";
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
                // qCDebug(appLog) << "Window is modal";
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
    //根据实际情况设置颜色，奇数行为灰色
    auto palette = options.palette;
    QBrush background;
    if (!(index.row() & 1)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        auto dpa = DApplicationHelper::instance()->palette(this);
        background = dpa.color(DPalette::ItemBackground);
#else
        // 当Qt版本为6.0或更高时，直接使用palette.color(cg, DPalette::Base)来获取背景颜
        background = palette.color(cg, DPalette::Base);
#endif
    } else {
        background = palette.color(cg, DPalette::Base);
    }
    // 绘制整行背景，高度-2以让高分屏非整数缩放比例下无被选中的蓝色细线，防止原来通过delegate绘制单元格交替颜色背景出现的高分屏非整数缩放比例下qrect精度问题导致的横向单元格间出现白色边框
    QPainterPath path;
    QRect rowRect { options.rect.x(),
                    options.rect.y() + 1,
                    header()->length(),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x());
    rowRect.setWidth(rowRect.width());

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    QTreeView::drawRow(painter, options, index);

    painter->restore();
}

/**
 * @brief keyPressEvent 重写按钮事件
 */
void DriverListView::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "Key pressed:" << event->key();

    DTreeView::keyPressEvent(event);
    Qt::KeyboardModifiers modifiers = event->modifiers();
    if (modifiers != Qt::NoModifier) {
        if (modifiers.testFlag(Qt::ControlModifier) || modifiers.testFlag(Qt::ShiftModifier) || modifiers.testFlag(Qt::AltModifier)) {
            qCDebug(appLog) << "Modifier key pressed, ignoring key event";
            return;
        }
    }
    QStandardItemModel *pModel = static_cast<QStandardItemModel *>(model());

    int curRow = this->currentIndex().row();
    switch (event->key()) {
    case Qt::Key_Home:
        qCDebug(appLog) << "Home key pressed, moving to first item";
        curRow = 0;
        break;
    case Qt::Key_End:
        qCDebug(appLog) << "End key pressed, moving to last item";
        curRow = pModel->rowCount() - 1;
        break;
    case Qt::Key_Up:
    case Qt::Key_Down:
        qCDebug(appLog) << "Up/Down key pressed, navigating to next/previous item";
        break;
    default:
        return;
    }
    emit clicked(pModel->index(curRow, 1));
}
