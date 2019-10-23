#pragma once

#include "DMainWindow"
#include "dwidgetstype.h"

class DeviceListView;
class DeviceInfoWidgetBase;

class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addAllDeviceinfoWidget();
    void addDeviceWidget(DeviceInfoWidgetBase* w);
    void insertDeviceWidget(int index, DeviceInfoWidgetBase* w);
    void refresh();
    void refreshDatabase();

    bool exportTo(const QString& file, const QString& selectFilter);
private:
    DeviceListView* leftDeviceView_ = nullptr;
    Dtk::Widget::DStackedWidget* rightDeviceInfoWidget_ = nullptr;

    QMap<QString, DeviceInfoWidgetBase*> deviceInfoWidgetMap_;//widgetname　- widget
    bool firstAdd_ = true;
    QString currentDevice_;
};
