#pragma once

#include "DMainWindow"
#include "dwidgetstype.h"
#include <QSplashScreen>
#include "DSpinner"

class DeviceListView;
class DeviceInfoWidgetBase;

class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadDeviceWidget();
    void refreshDeviceWidget();

    void addAllDeviceinfoWidget();

    void addDeviceWidget(DeviceInfoWidgetBase* w, const QString& icon);
    void insertDeviceWidget(int index, DeviceInfoWidgetBase* w);

    void refresh();
    void refreshDatabase();

    bool exportTo(const QString& file, const QString& selectFilter);

signals:
    void startLoadingWidgetSignal();

public slots:
    void initLoadingWidget();

private:
    void showSplashMessage(const QString& message);

private:
    DeviceListView* leftDeviceView_ = nullptr;
    Dtk::Widget::DStackedWidget* rightDeviceInfoWidget_ = nullptr;

    QMap<QString, DeviceInfoWidgetBase*> deviceInfoWidgetMap_;//widgetname　- widget
    bool firstAdd_ = true;
    QString currentDevice_;

    Dtk::Widget::DMenu* contextMenu_ = nullptr;
    Dtk::Widget::DLabel* loadLabel_ = nullptr;
    Dtk::Widget::DWidget* mainWidget_ = nullptr;
    Dtk::Widget::DWidget* loadingWidget_ = nullptr;
};
