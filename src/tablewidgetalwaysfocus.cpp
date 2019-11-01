#include "tablewidgetalwaysfocus.h"
#include "mainwindow.h"
#include <QStyledItemDelegate>
#include <qdrawutil.h>

//DWIDGET_USE_NAMESPACE

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

//TableWidgetAlwaysActive::TableWidgetAlwaysActive(QWidget *parent): DTableWidget(parent)
//{

//}

//QStyleOptionViewItem TableWidgetAlwaysActive::viewOptions() const
//{
//    auto styleOptViewItem = DTableWidget::viewOptions();
//    styleOptViewItem.state |= QStyle::State_Active;

//    return styleOptViewItem;
//}
