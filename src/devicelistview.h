#pragma once

#include <QObject>
#include <QString>
#include "DListView"
#include <DStyledItemDelegate>
#include "DWidget"

class QListViewItem;
class QAction;

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

    void setSelectNull();

    QString indexAt(int index);

public slots:
    bool onExportToFile();
    void OnlvOpRightBtn( const QPoint&);
    void changeThemeIcon();

private:
    void initContextMenu();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

private:
    QStandardItemModel* navModel_ = nullptr;
    QString currentDevice_;
    Dtk::Widget::DMenu* contextMenu_ = nullptr;
    QAction* refreshAction_ = nullptr;
};
