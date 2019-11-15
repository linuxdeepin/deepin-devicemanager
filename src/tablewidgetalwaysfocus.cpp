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

#include "tablewidgetalwaysfocus.h"
#include "mainwindow.h"
#include <QStyledItemDelegate>
#include <qdrawutil.h>
#include "deviceinfowidgetbase.h"
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

////TableWidgetAlwaysActiveHeaderViewDelegate::TableWidgetAlwaysActiveHeaderViewDelegate(QAbstractItemView *parent)
////    : DStyledItemDelegate(parent)
////{

////}

////void TableWidgetAlwaysActiveHeaderViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
////{
////    painter->save();

////    int yPoint = option.rect.top() + option.rect.height() / 2;
////    qDrawShadeRect(painter, 8, yPoint, option.rect.width() - 8, yPoint, option.palette);

////    painter->restore();
////}

//TableWidgetAlwaysActiveHeaderView::TableWidgetAlwaysActiveHeaderView(Qt::Orientation orientation, QWidget *parent ):DHeaderView(orientation, parent)
//{

//}

////void TableWidgetAlwaysActiveHeaderView::paintEvent(QPaintEvent *e)
////{
////    return DHeaderView::paintEvent(e);
////}

//QStyleOptionViewItem TableWidgetAlwaysActiveHeaderView::viewOptions() const
//{
//    auto styleOptViewItem = TableWidgetAlwaysActiveHeaderView::viewOptions();

//    QBrush brush;
//    brush.setColor(QColor(255, 255, 255, 255));

//    styleOptViewItem.palette.setBrush(QPalette::Background, brush);

//    return styleOptViewItem;
//}

TableWidgetAlwaysActive::TableWidgetAlwaysActive(QWidget *parent): DTableWidget(parent)
{

}

int TableWidgetAlwaysActive::sizeHintForColumn(int column) const
{
    int maxWidth = 50;
    for(int i = 0; i < rowCount(); ++i)
    {
        int width = QFontMetrics( DeviceInfoWidgetBase::tableContentFont_ ).width(item(i, column)->text());
        if(maxWidth < width)
        {
            maxWidth = width;
        }
    }

    return (maxWidth + 50 > DeviceWidgetDownWidgehWidth_/columnCount()*2) ? DeviceWidgetDownWidgehWidth_/columnCount()*2 : (maxWidth + 50);
}

QStyleOptionViewItem TableWidgetAlwaysActive::viewOptions() const
{
    auto styleOptViewItem = DTableWidget::viewOptions();
    styleOptViewItem.state |= QStyle::State_Active;

    return styleOptViewItem;
}
