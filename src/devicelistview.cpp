#include "devicelistview.h"
#include <QStandardItemModel>
#include <QMargins>
#include <QVariant>
#include <QPainter>
#include <qdrawutil.h>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

DeviceListviewDelegate::DeviceListviewDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
{

}

void DeviceListviewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return paintSeparator(painter, option);
    }

    return DStyledItemDelegate::paint(painter, option, index);
}

QSize DeviceListviewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if( index.data().toString() == "Seperator" )
    {
        return QSize(200, 5);;
    }
    else
    {
        return DStyledItemDelegate::sizeHint(option, index);
    }
}

void DeviceListviewDelegate::paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const
{
    painter->save();

    int yPoint = option.rect.top() + option.rect.height() / 2;
    qDrawShadeLine(painter, 0, yPoint, option.rect.width(), yPoint, option.palette);

    painter->restore();
}

DeviceListView::DeviceListView(QWidget* parent):DListView(parent)
{
    setFrameShape(QFrame::Shape::NoFrame);
    setEditTriggers(QListView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    setAutoScroll(false);

    setSpacing(0);
    setViewMode(QListView::ListMode);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    //viewport()->setAutoFillBackground(false);

    setFocus(Qt::FocusReason::NoFocusReason);

    navModel_ = new QStandardItemModel(navModel_);
    setModel(navModel_);

    setItemDelegate( new DeviceListviewDelegate(this) );

    setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    setAutoFillBackground(true);
}

void DeviceListView::addDevice(const QString& deviceName, const QString& iconFile)
{
    DStandardItem* item = new DStandardItem;
    item->setIcon(QIcon(iconFile));
    item->setText(deviceName);

    const QMargins ListViweItemMargin(20,8,20,8);
    const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

    item->setData(VListViewItemMargin, Dtk::MarginsRole);

    navModel_->appendRow(item);
}

void DeviceListView::addSeperator()
{
    DStandardItem* item = new DStandardItem;
    item->setText("Seperator");
    navModel_->appendRow(item);
}
