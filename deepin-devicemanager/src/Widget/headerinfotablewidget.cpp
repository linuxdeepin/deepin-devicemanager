// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "headerinfotablewidget.h"
#include "headerinfotableDelegate.h"
#include "MacroDefinition.h"

#include <DApplication>
#include <DPaletteHelper>

#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

static constexpr int kColumnCount { 2 };

HeaderInfoTableWidget::HeaderInfoTableWidget(DWidget *parent)
    : DTableWidget(parent)
{
    initUI();
}

void HeaderInfoTableWidget::updateData(const QList<QPair<QString, QString>> &data)
{
    resetTableContents();
    int nRow = data.size();
    setRowCount(nRow);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg = wnd ? DPalette::Active : DPalette::Inactive;
    auto palette = DPaletteHelper::instance()->palette(this);
    QColor colorEven = palette.color(cg, DPalette::Base);
    QColor colorOdd = palette.color(cg, DPalette::ItemBackground);

    for (int i = 0; i < nRow; ++i) {
        QTableWidgetItem *itemFirst = new QTableWidgetItem(data[i].first);
        itemFirst->setFlags(itemFirst->flags() & ~Qt::ItemIsSelectable);
        itemFirst->setBackground(i % 2 == 0 ? colorEven : colorOdd);
        setItem(i, 0, itemFirst);
        QTableWidgetItem *itemSecond = new QTableWidgetItem(data[i].second);
        itemSecond->setFlags(itemSecond->flags() & ~Qt::ItemIsSelectable);
        itemSecond->setBackground(i % 2 == 0 ? colorEven : colorOdd);
        setItem(i, 1, itemSecond);
    }

    // 调整控件高度，使内容刚好完全显示，不出现垂直滚动条
    if (nRow > 0) {
        // 为了保持底部圆角，给表格高度留出 2px 余量；避免内容铺满viewport导致圆角被覆盖
        setFixedHeight(ROW_HEIGHT * nRow + 2);
    } else {
        setFixedHeight(0);
    }
}

void HeaderInfoTableWidget::initUI()
{
    setColumnCount(kColumnCount);
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
    setSelectionMode(QAbstractItemView::NoSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    this->setFocusPolicy(Qt::NoFocus);
    // 设置无边框, 避免原始方形外框干扰圆角绘制
    this->setFrameStyle(QFrame::NoFrame);
    this->setShowGrid(false);
    this->viewport()->setAutoFillBackground(true);
    m_delegate = new HeaderInfoDelegate(this);
    this->setItemDelegate(m_delegate);
    setAlternatingRowColors(true);
    this->verticalHeader()->setVisible(false);
    this->horizontalHeader()->setVisible(false);
    this->setSortingEnabled(false);
    this->horizontalHeader()->setStretchLastSection(true);
    this->verticalHeader()->setDefaultSectionSize(ROW_HEIGHT);
    this->horizontalHeader()->setDefaultSectionSize(180);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);
}

void HeaderInfoTableWidget::paintEvent(QPaintEvent *event)
{
    DTableWidget::paintEvent(event);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg = wnd ? DPalette::Active : DPalette::Inactive;

    auto palette = DPaletteHelper::instance()->palette(this);

    QRect rect = viewport()->rect().adjusted(0, 0, -1, -1);
    int width = 1;
    int radius = 8;

    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath paintPathOut;
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + width, rect.y() + width, rect.width() - width * 2, rect.height() - width * 2);
    QPainterPath paintPathIn;
    paintPathIn.addRoundedRect(rectIn, radius, radius);

    QPainterPath paintPath = paintPathOut.subtracted(paintPathIn);
    QBrush bgBrush(palette.color(cg, DPalette::FrameBorder));
    painter.fillPath(paintPath, bgBrush);

    QPen pen = painter.pen();
    pen.setWidth(width);
    pen.setColor(palette.color(cg, DPalette::FrameBorder));
    painter.setPen(pen);
    painter.drawRoundedRect(rectIn, radius, radius);

    // Draw vertical divider between first and second columns (same as DetailTreeView)
    QLine vline(rect.topLeft().x() + 179, rect.topLeft().y(), rect.bottomLeft().x() + 179, rect.bottomLeft().y());
    painter.drawLine(vline);
}

void HeaderInfoTableWidget::resetTableContents()
{
    DTableWidget::clear();
    setRowCount(0);
}
