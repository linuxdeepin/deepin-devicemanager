#include "devicelistwidget.h"
#include "DTreeWidget"

DWIDGET_USE_NAMESPACE

DeviceListWidget::DeviceListWidget(QWidget* parent):DTreeWidget(parent)
{
    setFocus(Qt::FocusReason::NoFocusReason);
    //setSizePolicy(QSizePolicy::Policy::Preferred);
}

void DeviceListWidget::addDevice(const QString& deviceName, const QString& iconFile)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(this);
    item->setIcon(0, QIcon(iconFile));
    item->setText(0, deviceName);
}
