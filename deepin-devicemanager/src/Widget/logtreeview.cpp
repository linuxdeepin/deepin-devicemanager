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
#include <QPainterPath>
//#include "structdef.h"
#include "deviceinfowidgetbase.h"
#include "../commondefine.h"

DWIDGET_USE_NAMESPACE

LogTreeView::LogTreeView(QWidget *parent)
    : DTreeView(parent)
{
    initUI();

    m_pModel = new QStandardItemModel(this);

    setModel(m_pModel);

}

void LogTreeView::initUI()
{
    m_itemDelegate = new LogViewItemDelegate(this);
    setItemDelegate(m_itemDelegate);

    m_headerDelegate = new LogViewHeaderView(Qt::Horizontal, this);
    setHeader(m_headerDelegate);

    this->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->setRootIsDecorated(false);

    // this is setting is necessary,because scrollperpixel is default in dtk!!
    //this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    this->header()->setStretchLastSection(true);

    this->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    this->header()->setFixedHeight(TableViewRowHeight_);
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setFrameStyle(QFrame::NoFrame);
    this->viewport()->setAutoFillBackground(true);
    //setBack
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

    QBrush bgBrush(palette.color(cg, DPalette::Background));

    QStyleOptionFrame option;
    initStyleOption(&option);

    QRect rect = viewport()->rect();
    QRectF clipRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() - rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    /*
    *@author yaobin
    *@date 2020-01-03
    *@Modify Reason:底部不需要圆角
    */
//    clipPath.addRoundedRect(clipRect, 8, 8);
    clipPath.addRoundedRect(clipRect, 0, 0);
    subPath.addRect(subRect);
    clipPath = clipPath.subtracted(subPath);
    clipPath.addRect(rect);

    painter.fillPath(clipPath, bgBrush);

    painter.restore();

    DTreeView::paintEvent(event);
}

void LogTreeView::keyPressEvent(QKeyEvent *event)
{
    DTreeView::keyPressEvent(event);
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        emit clicked(this->currentIndex());
    }
}

int LogTreeView::calulateColumnSize() const
{
    /*
    int columnCount = m_headerDelegate->count();

    int maxColumnWidth = static_cast<float>(width())*(static_cast<float>(2*columnCount-1))/(static_cast<float>(columnCount*columnCount));

    static int margin = 25;

    int maxWidth = static_cast<float>(width())/static_cast<float>(columnCount*2);

    for(int i = 0; i < m_pModel->rowCount(); ++i)
    {
        int width = QFontMetrics( DeviceInfoWidgetBase::tableContentFont_ ).width(m_pModel->item(i, column)->text()) + margin;
        if(maxWidth < width)
        {
            maxWidth = width;
        }
    }

    if(maxWidth > maxColumnWidth)
    {
        maxWidth  = maxColumnWidth;
    }

    return maxWidth;
    */
//    int colWidthSum = 0;
//    for(int i = 0;i < header()->count();i++){
//        colWidthSum += columnWidth(i);
//    }
    return  width() / header()->count();
}

void LogTreeView::resizeEvent(QResizeEvent *event)
{
    setEachColunmWidthSame();
    DTreeView::resizeEvent(event);
}

void LogTreeView::setEachColunmWidthSame()
{
    int colCount = header()->count();
    int avgColWidth = width() / colCount;
    for (int i = 0; i < colCount; i++) {
        setColumnWidth(i, avgColWidth);
    }
}

//QStyleOptionViewItem LogTreeView::viewOptions() const
//{
//    auto styleOptViewItem = DTreeView::viewOptions();
//    styleOptViewItem.state |= QStyle::State_Active;

//    return styleOptViewItem;
//}

//void LogTreeView::initStyleOption(QStyleOptionFrame *option) const
//{
//    DTreeView::initStyleOption(option);
//    option->state |= QStyle::State_Active;

//}


