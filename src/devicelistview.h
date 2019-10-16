#pragma once

#include <QObject>
#include <QString>
#include "DListView"
#include <DStyledItemDelegate>

class DeviceListviewDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    DeviceListviewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
};

class DeviceListView : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    DeviceListView(QWidget* parent = nullptr);
    void addDevice(const QString& deviceName, const QString& iconFile);
    void addSeperator();

public:
    QStandardItemModel* navModel_ = nullptr;
};
