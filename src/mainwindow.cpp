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
#include "deviceinfoparser.h"
#include "DApplication"

DWIDGET_USE_NAMESPACE

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    //setAutoFillBackground(false);
    DWidget* mainWidget = new DWidget(this);
    //mainWidget->setAutoFillBackground(false);
    mainWidget->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    ly->setMargin(0);
    ly->setContentsMargins(0, 0, 0, 0);
    ly->setSpacing(0);
    ly->setSizeConstraint(QLayout::SetMaximumSize);
    setFocus(Qt::FocusReason::NoFocusReason);

    leftDeviceList_ = new DeviceListWidget(this);
    leftDeviceList_->setMinimumWidth(150);

    ly->addWidget(leftDeviceList_);

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget);
    //rightDeviceInfoWidget_->setAutoFillBackground(false);

    addAllDeviceinfoWidget();

    connect(leftDeviceList_, &DListView::clicked, [this](const QModelIndex& index)
            {
                deviceInfoWidgetMap_[index.data().toString()]->deviceListClicked();
                rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[index.data().toString()]);
            }
    );

    ly->addWidget(rightDeviceInfoWidget_);

    mainWidget->setLayout(ly);

    setCentralWidget(mainWidget);
}

MainWindow::~MainWindow()
{

}

void MainWindow::addAllDeviceinfoWidget()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    refreshDatabase();

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

    firstAdd_ = false;
    QApplication::restoreOverrideCursor();
}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase* w)
{
    if(firstAdd_ == true)
    {
        leftDeviceList_->addDevice(w->getDeviceName(), ":/cpu.svg");
    }
    rightDeviceInfoWidget_->addWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}

void MainWindow::refresh()
{
    QString currentDevice = leftDeviceList_->currentIndex().data().toString();

    QMap<QString, DeviceInfoWidgetBase*> oldWidgetMap;
    std::swap(deviceInfoWidgetMap_, oldWidgetMap);

    addAllDeviceinfoWidget();

    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);

    foreach(const QString& widgetName, oldWidgetMap.keys())
    {
        rightDeviceInfoWidget_->removeWidget(oldWidgetMap[widgetName]);
        delete oldWidgetMap[widgetName];
    }
}

void MainWindow::refreshDatabase()
{
    QString osInfo;
    DeviceInfoParserInstance.getOSInfo(osInfo);
    DeviceInfoParserInstance.loadDemicodeDatabase();
    DeviceInfoParserInstance.loadLshwDatabase();
    DeviceInfoParserInstance.loadLscpuDatabase();
    DeviceInfoParserInstance.loadSmartctlDatabase();
    DeviceInfoParserInstance.loadCatInputDatabase();
    DeviceInfoParserInstance.loadPowerSettings();
    DeviceInfoParserInstance.loadXrandrDatabase();
    DeviceInfoParserInstance.loadLspciDatabase();
    DeviceInfoParserInstance.loadHciconfigDatabase();
    DeviceInfoParserInstance.loadLsusbDatabase();
    DeviceInfoParserInstance.loadHwinfoDatabase();
}
