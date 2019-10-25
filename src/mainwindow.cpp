#include "mainwindow.h"
#include "devicelistview.h"
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
#include "printerwidget.h"
#include "deviceinfoparser.h"
#include "DApplication"
#include "DApplicationHelper"
#include <QSplashScreen>
#include <QFileSystemModel>
#include "DFrame"
#include <QProgressDialog>

DWIDGET_USE_NAMESPACE

QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    //setWindowTitle("");
    setAutoFillBackground(true);
    DWidget* mainWidget = new DWidget(this);
    mainWidget->setAutoFillBackground(true);
    mainWidget->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    ly->setMargin(0);
    //ly->setContentsMargins(0, 0, 0, 0);
    ly->setSpacing(0);
    ly->setSizeConstraint(QLayout::SetMinAndMaxSize);
    setFocus(Qt::FocusReason::NoFocusReason);

    leftDeviceView_ = new DeviceListView(this);
    leftDeviceView_->setMaximumWidth(220);
    leftDeviceView_->setMinimumWidth(100);

    ly->addWidget(leftDeviceView_);

    this->setFocusProxy(leftDeviceView_);

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget);

    addAllDeviceinfoWidget();

    connect(leftDeviceView_, &DeviceListView::clicked, [this](const QModelIndex& index)
                {
                    QString device = index.data().toString();
                    if( false == deviceInfoWidgetMap_.contains(device) )
                    {
                        return;
                    }

                    deviceInfoWidgetMap_[device]->deviceListClicked();
                    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[index.data().toString()]);
                    leftDeviceView_->setCurrentDevice(device);
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
    //QProgressDialog progress("Copying files...", "Abort Copy", 0, 100, this);
    //progress.setWindowModality(Qt::NonModal);

    staticArticles.clear();

    //progress.show();

    //progress.setValue(1);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    refreshDatabase();

    auto overviewWidget = new ComputerOverviewWidget(this);
    addDeviceWidget(overviewWidget, ":images/overview.svg");
    addDeviceWidget(new CpuWidget(this), ":images/cpu.svg");
    addDeviceWidget(new MotherboardWidget(this), ":images/motherboard.ico");
    addDeviceWidget(new MemoryWidget(this), ":images/memory.svg");
    addDeviceWidget(new DiskWidget(this), ":images/disk.ico");
    addDeviceWidget(new KeyboardWidget(this), ":images/keyboard.svg");
    addDeviceWidget(new MouseWidget(this), ":images/mouse.ico");
    addDeviceWidget(new PowerWidget(this), ":images/power.svg");
    addDeviceWidget(new PrinterWidget(this), ":images/printer.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new DisplayadapterWidget(this), ":images/overview.svg");
    addDeviceWidget(new AudiodeviceWidget(this), ":images/sounddevice.svg");
    addDeviceWidget(new NetworkadapterWidget(this), ":images/networkadapter.svg");
    addDeviceWidget(new BluetoothWidget(this), ":images/bluetooth.svg");
    addDeviceWidget(new MonitorWidget(this), ":images/monitor.svg");
    addDeviceWidget(new CameraWidget(this), ":images/camera.svg");
    addDeviceWidget(new UsbdeviceWidget(this), ":images/usbdevice.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new OtherInputdeviceWidget(this), ":images/otherinputdevice.svg");
    addDeviceWidget(new OtherPciDeviceWidget(this), ":images/otherpcidevice.ico");

    overviewWidget->setOverviewInfos(staticArticles);
    firstAdd_ = false;
    //progress.setValue(100);
    QApplication::restoreOverrideCursor();
    //progress.close();
}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase* w,  const QString& icon)
{
    if(w == nullptr)
    {
        return;
    }

    if(firstAdd_ == true)
    {
        leftDeviceView_->addDevice(w->getDeviceName(), icon);
    }

    ArticleStruct overviweInfo;
    if( true == w->getOverViewInfo(overviweInfo) )
    {
        staticArticles.push_back(overviweInfo);
    }

    rightDeviceInfoWidget_->addWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}

void MainWindow::insertDeviceWidget(int index, DeviceInfoWidgetBase* w)
{
    if(firstAdd_ == true)
    {
        leftDeviceView_->addDevice(w->getDeviceName(), ":images/cpu.svg");
    }

    rightDeviceInfoWidget_->insertWidget(index, w);
    rightDeviceInfoWidget_->setCurrentWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}

void MainWindow::refresh()
{
    QString currentDevice = leftDeviceView_->currentDevice();

    QMap<QString, DeviceInfoWidgetBase*> oldWidgetMap;
    std::swap(deviceInfoWidgetMap_, oldWidgetMap);

    addAllDeviceinfoWidget();

    if(  deviceInfoWidgetMap_.contains(currentDevice) )
    {
        rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);;
    }

    foreach(const QString& widgetName, oldWidgetMap.keys())
    {
        rightDeviceInfoWidget_->removeWidget(oldWidgetMap[widgetName]);
        delete oldWidgetMap[widgetName];
    }
}

void MainWindow::refreshDatabase()
{
    DeviceInfoParserInstance.loadCatosrelelease();
    DeviceInfoParserInstance.loadlsb_release();
    //DeviceInfoParserInstance.getOSInfo(osInfo);
    DeviceInfoParserInstance.loadDemicodeDatabase();
    DeviceInfoParserInstance.loadLshwDatabase();
    DeviceInfoParserInstance.loadCatcpuDatabase();
    DeviceInfoParserInstance.loadLscpuDatabase();
    //DeviceInfoParserInstance.loadSmartctlDatabase();
    DeviceInfoParserInstance.loadCatInputDatabase();
    DeviceInfoParserInstance.loadPowerSettings();
    DeviceInfoParserInstance.loadXrandrDatabase();
    DeviceInfoParserInstance.loadLspciDatabase();
    DeviceInfoParserInstance.loadHciconfigDatabase();
    DeviceInfoParserInstance.loadLsusbDatabase();
    DeviceInfoParserInstance.loadHwinfoDatabase();
    DeviceInfoParserInstance.loadCupsDatabase();
}

bool MainWindow::exportTo(const QString& file, const QString& selectFilter)
{
    if(selectFilter == "Text (*.txt)")
    {
        QFile textFile( file );
        if( false == textFile.open(QIODevice::WriteOnly))
        {
            return false;
        }

        for(int i = 0; i < leftDeviceView_->count(); ++i)
        {
            QString device = leftDeviceView_->indexAt(i);
            if( deviceInfoWidgetMap_.contains(device) )
            {
                deviceInfoWidgetMap_[device]->exportToTxt(textFile);
            }
        }

        textFile.close();

        return true;
    }

    if(selectFilter == "Html (*.html)")
    {
        QFile htmlFile( file );
        if( false == htmlFile.open(QIODevice::WriteOnly))
        {
            return false;
        }

        for(int i = 0; i < leftDeviceView_->count(); ++i)
        {
            QString device = leftDeviceView_->indexAt(i);
            if( deviceInfoWidgetMap_.contains(device) )
            {
                deviceInfoWidgetMap_[device]->exportToHtml(htmlFile);
            }
        }

        htmlFile.close();

        return true;
    }

    if(selectFilter == "Doc (*.doc)")
    {
        Docx::Document doc(":/thirdlib/docx/doc_template/template.doc");

        for(int i = 0; i < leftDeviceView_->count(); ++i)
        {
            QString device = leftDeviceView_->indexAt(i);
            if( deviceInfoWidgetMap_.contains(device) )
            {
                deviceInfoWidgetMap_[device]->exportToDoc(doc);
            }
        }

        doc.save(file);
        return true;
    }

    if( selectFilter == "Xls (*.xls)")
    {
        QXlsx::Document xlsx;

        DeviceInfoWidgetBase::resetXlsRowCount();

        for(int i = 0; i < leftDeviceView_->count(); ++i)
        {
            QString device = leftDeviceView_->indexAt(i);
            if( deviceInfoWidgetMap_.contains(device) )
            {
                deviceInfoWidgetMap_[device]->exportToXls(xlsx);
            }
        }

        xlsx.saveAs(file);
        return true;
    }


    return false;
}
