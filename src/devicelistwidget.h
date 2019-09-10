#pragma once

#include <QObject>
#include <QString>
#include "DTreeWidget"

class DeviceListWidget : public Dtk::Widget::DTreeWidget
{
    Q_OBJECT
public:
    DeviceListWidget(QWidget* parent = nullptr);
    void addDevice(const QString& deviceName, const QString& iconFile);

signals:

public slots:

};
