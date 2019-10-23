#include "devicelistview.h"
#include <QStandardItemModel>
#include <QMargins>
#include <QVariant>
#include <QPainter>
#include <qdrawutil.h>
#include "DApplicationHelper"

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
        return QSize(option.rect.width(), 5);;
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
    qDrawShadeLine(painter, 8, yPoint, option.rect.width() - 8, yPoint, option.palette);

    painter->restore();
}

DeviceListView::DeviceListView(DWidget* parent):DListView(parent)
{
//    setResizeMode(QListView::Adjust);
//    setOrientation(QListView::LeftToRight, true);
//    setTextElideMode(Qt::ElideMiddle);
//    setSelectionBehavior(QAbstractItemView::SelectRows);

    //DListView::setSelectionRectVisible(false);

    navModel_ = new QStandardItemModel(navModel_);

    setVerticalScrollMode(ScrollPerPixel);
    setIconSize(QSize(24, 24));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    setMouseTracking(true);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDragDropOverwriteMode(false);

    setModel(navModel_);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setFrameShape(QFrame::Shape::NoFrame);

    setEditTriggers(QListView::NoEditTriggers);
    setResizeMode(QListView::Adjust);
    //setAutoScroll(false);

    //setSpacing(0);
    //setViewMode(QListView::ListMode);
    //setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    viewport()->setAutoFillBackground(true);

    setFocus(Qt::FocusReason::NoFocusReason);


    setItemDelegate( new DeviceListviewDelegate(this) );

    setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    setAutoFillBackground(true);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    QColor base_color = palette().base().color();
    DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);

    if (ct == DGuiApplicationHelper::LightType) {
        pa.setBrush(DPalette::ItemBackground, palette().base());
    } else {
        base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
        pa.setColor(DPalette::ItemBackground, base_color);
    }

    DApplicationHelper::instance()->setPalette(this, pa);

    //setMaximumWidth(150);
}

void DeviceListView::addDevice(const QString& deviceName, const QString& iconFile)
{
    DStandardItem* item = new DStandardItem;
    QFont itemFont = item->font();
    itemFont.setPointSize(13);

    item->setFont(itemFont);

    item->setIcon(QIcon(iconFile));
    item->setText(deviceName);

    item->setTextAlignment(Qt::AlignLeft);

    const QMargins ListViweItemMargin(20,8,20,8);
    //const QMargins ListViweItemMargin( 10, 5, 10, 5 );
    const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

    item->setData(VListViewItemMargin, Dtk::MarginsRole);

    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren);

    navModel_->appendRow(item);
}

void DeviceListView::addSeperator()
{
    DStandardItem* item = new DStandardItem;
    item->setText("Seperator");
    navModel_->appendRow(item);
}

QString DeviceListView::currentDevice()
{
    return currentDevice_;
}

void DeviceListView::setCurrentDevice(QString& device)
{
    currentDevice_ = device;
}

QString DeviceListView::indexAt(int index)
{
    return navModel_->index(index, 0).data().toString();

    //return navModel_->index(index).data().toString();
}
