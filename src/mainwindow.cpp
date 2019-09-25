#include "mainwindow.h"
#include "devicelistwidget.h"
#include <QHBoxLayout>
#include "DStackedWidget"

#include "computeroverviewwidget.h"
#include "motherboardwidget.h"
#include "cpuwidget.h"
#include "memorywidget.h"
#include "diskwidget.h"
#include "displayadapterwidget.h"
#include "monitorwidget.h"
#include "audiodevicewidget.h"
#include "networkadapterwidget.h"
#include "bluetoothwidget.h"
#include "camerawidget.h"
#include "keyboardwidget.h"
#include "mousewidget.h"
#include "usbdevicewidget.h"
#include "otherinputdevicewidget.h"
#include "powerwidget.h"
#include <QStandardItemModel>
#include "otherpcidevice.h"
#include "portwidget.h"

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    QWidget* mainWidget = new QWidget(this);
    mainWidget->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    setFocus(Qt::FocusReason::NoFocusReason);

    leftDeviceList_ = new DeviceListWidget(this);
    leftDeviceList_->setMinimumWidth(150);

    ly->addWidget(leftDeviceList_);

    rightDeviceInfoWidget_ = new DStackedWidget(this);

    addDeviceWidget(new ComputerOverviewWidget(this));
    addDeviceWidget(new CpuWidget(this));
    addDeviceWidget(new MotherboardWidget(this));
    addDeviceWidget(new MemoryWidget(this));
    addDeviceWidget(new DiskWidget(this));
    addDeviceWidget(new DisplayadapterWidget(this));
    addDeviceWidget(new MonitorWidget(this));
    addDeviceWidget(new AudiodeviceWidget(this));
    addDeviceWidget(new NetworkadapterWidget(this));
    addDeviceWidget(new BluetoothWidget(this));
    addDeviceWidget(new CameraWidget(this));
    addDeviceWidget(new MouseWidget(this));
    addDeviceWidget(new KeyboardWidget(this));
    addDeviceWidget(new UsbdeviceWidget(this));
    addDeviceWidget(new OtherInputdeviceWidget(this));
    addDeviceWidget(new PowerWidget(this));
    addDeviceWidget(new OtherPciDeviceWidget(this));
    addDeviceWidget(new PortWidget(this));


    connect(leftDeviceList_, &DListView::clicked, [this](const QModelIndex& index)
            {
                QString currentDevice = index.data().toString();
                rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);
            }
    );

    ly->addWidget(rightDeviceInfoWidget_);

    mainWidget->setLayout(ly);

    setCentralWidget(mainWidget);

}

MainWindow::~MainWindow()
{

}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase* w)
{
    leftDeviceList_->addDevice(w->getDeviceName(), ":/cpu.svg");
    rightDeviceInfoWidget_->addWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}
