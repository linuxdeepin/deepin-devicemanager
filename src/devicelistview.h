#pragma once

#include <QObject>
#include <QString>
#include "DListView"
#include <DStyledItemDelegate>
#include "DWidget"

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
    DeviceListView(Dtk::Widget::DWidget* parent = nullptr);
    void addDevice(const QString& deviceName, const QString& iconFile);
    void addSeperator();

    QString currentDevice();
    void setCurrentDevice(QString& device);

    QString indexAt(int index);

private:
    QStandardItemModel* navModel_ = nullptr;
    QString currentDevice_;
};
