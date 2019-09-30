#include "devicelistwidget.h"
#include <QStandardItemModel>
#include <QMargins>
#include <QVariant>

DWIDGET_USE_NAMESPACE

Q_DECLARE_METATYPE(QMargins)

DeviceListWidget::DeviceListWidget(QWidget* parent):DListView(parent)
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

    setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
}

void DeviceListWidget::addDevice(const QString& deviceName, const QString& iconFile)
{
    DStandardItem* item = new DStandardItem;
    item->setIcon(QIcon(iconFile));
    item->setText(deviceName);

    const QMargins ListViweItemMargin(20,8,20,8);
    const QVariant VListViewItemMargin = QVariant::fromValue(ListViweItemMargin);

    item->setData(VListViewItemMargin, Dtk::MarginsRole);

    navModel_->appendRow(item);
}
