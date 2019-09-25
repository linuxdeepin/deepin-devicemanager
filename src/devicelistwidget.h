#pragma once

#include <QObject>
#include <QString>
#include "DListView"

class DeviceListWidget : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    DeviceListWidget(QWidget* parent = nullptr);
    void addDevice(const QString& deviceName, const QString& iconFile);

public:
    QStandardItemModel* navModel_ = nullptr;
};
