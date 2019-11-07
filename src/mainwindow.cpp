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
#include <DWidgetUtil>
#include <QDir>
#include "DFileDialog"
#include <QDateTime>
#include "DTitlebar"
#include "DSpinner"
#include "cdromwidget.h"
#include <thread>

DWIDGET_USE_NAMESPACE

QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    setMinimumSize(1070, 790);

    DeviceInfoParserInstance.getRootPassword();

    initLoadingWidget();
    setCentralWidget(loadingWidget_);

    connect( &DeviceInfoParserInstance, &DeviceInfoParser::loadFinished, this, &MainWindow::showSplashMessage);

    refreshDatabase();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initLoadingWidget()
{
    //setAutoFillBackground(false);

    loadingWidget_ = new DWidget(this);

    //loadingWidget_->setAutoFillBackground(true);

    DPalette pa = DApplicationHelper::instance()->palette(this);
    QColor base_color = palette().base().color();

    pa.setColor(QPalette::Background, base_color);
    //pa.setBrush(DPalette::ItemBackground, base_color);

    //setPalette(pa);

    DApplicationHelper::instance()->setPalette(loadingWidget_, pa);

    //mainWidget->setAutoFillBackground(true);
    //mainWidget_->setMinimumWidth(640);
    //mainWidget_->setMinimumHeight(640);

    QVBoxLayout* vly = new QVBoxLayout;

    vly->setMargin(20);

    vly->addStretch(2);
    auto spinner_ = new DSpinner(loadingWidget_);
    spinner_->setFixedSize(48, 48);

    QHBoxLayout* hly1 = new QHBoxLayout;
    hly1->addWidget(spinner_, 1);

    QHBoxLayout* hly2 = new QHBoxLayout;
    hly2->addStretch();

    loadLabel_ = new DLabel("Loading...", this);
    hly2->addWidget(loadLabel_);
    hly2->addStretch();

    vly->addLayout(hly1);
    vly->addLayout(hly2);

    vly->addStretch(3);

    spinner_->start();
    loadingWidget_->setLayout(vly);
}

void MainWindow::loadDeviceWidget()
{
    //setAutoFillBackground(false);
    mainWidget_ = new DWidget(this);
    //mainWidget->setAutoFillBackground(true);
    //mainWidget_->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    //ly->setContentsMargins(10, 0, 10, 0);
    ly->setMargin(0);
    ly->setSpacing(0);
    //ly->setSizeConstraint(QLayout::SetMinAndMaxSize);
    //setFocus(Qt::FocusReason::NoFocusReason);

    DApplication::processEvents();


    leftDeviceView_ = new DeviceListView(this);

    leftDeviceView_->setMaximumWidth(200);
    leftDeviceView_->setMinimumWidth(170);

    DApplication::processEvents();

    ly->addWidget(leftDeviceView_);

    DApplication::processEvents();

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget_);

    DApplication::processEvents();

    addAllDeviceinfoWidget();

    connect(leftDeviceView_, &DeviceListView::pressed, [this](const QModelIndex& index)
                {
                    QString device = index.data().toString();
                    if( false == deviceInfoWidgetMap_.contains(device) )
                    {
                        return;
                    }

                    deviceInfoWidgetMap_[device]->deviceListClicked();
                    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[index.data().toString()]);
                    leftDeviceView_->setCurrentDevice(device);
                    //titlebar()->setTitle(device);
            }
    );

//    connect(leftDeviceView_, &DeviceListView::clicked, [this](const QModelIndex& index)
//                {
//                    QString device = index.data().toString();
//                    if( false == deviceInfoWidgetMap_.contains(device) )
//                    {
//                        return;
//                    }

//                    deviceInfoWidgetMap_[device]->deviceListClicked();
//                    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[index.data().toString()]);
//                    leftDeviceView_->setCurrentDevice(device);
//            }
//    );

    ly->addWidget(rightDeviceInfoWidget_);

    DApplication::processEvents();

    mainWidget_->setLayout(ly);

    DApplication::processEvents();

    setCentralWidget(mainWidget_);

    if(loadingWidget_)
    {
        loadingWidget_ = nullptr;
        loadLabel_ = nullptr;
    }

    return;
}

void MainWindow::refreshDeviceWidget()
{
    QString currentDevice = leftDeviceView_->currentDevice();

    QMap<QString, DeviceInfoWidgetBase*> oldWidgetMap;
    std::swap(deviceInfoWidgetMap_, oldWidgetMap);

    addAllDeviceinfoWidget();

    if(  deviceInfoWidgetMap_.contains(currentDevice) )
    {
        rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);
    }

    foreach(const QString& widgetName, oldWidgetMap.keys())
    {
        rightDeviceInfoWidget_->removeWidget(oldWidgetMap[widgetName]);
        delete oldWidgetMap[widgetName];
    }

    if(loadingWidget_)
    {
        rightDeviceInfoWidget_->removeWidget(loadingWidget_);
        delete loadingWidget_;
        loadingWidget_ = nullptr;
        loadLabel_ = nullptr;
    }

}

void MainWindow::addAllDeviceinfoWidget()
{
    staticArticles.clear();

    auto overviewWidget = new ComputerOverviewWidget(this);
    addDeviceWidget(overviewWidget, "overview.svg");
    addDeviceWidget(new CpuWidget(this), "cpu.svg");
    addDeviceWidget(new MotherboardWidget(this), "motherboard.svg");
    addDeviceWidget(new MemoryWidget(this), "memory.svg");
    addDeviceWidget(new DiskWidget(this), "storage.svg");
    addDeviceWidget(new DisplayadapterWidget(this), "displayadapter.svg");
    addDeviceWidget(new MonitorWidget(this), "monitor.svg");
    addDeviceWidget(new NetworkadapterWidget(this), "networkadapter.svg");
    addDeviceWidget(new AudiodeviceWidget(this), "audiodevice.svg");
    addDeviceWidget(new BluetoothWidget(this), "bluetooth.svg");
    addDeviceWidget(new PowerWidget(this), "battery.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new KeyboardWidget(this), "keyboard.svg");
    addDeviceWidget(new MouseWidget(this), "mouse.svg");
    addDeviceWidget(new PrinterWidget(this), "printer.svg");
    addDeviceWidget(new CameraWidget(this), "camera.svg");
    addDeviceWidget(new UsbdeviceWidget(this), "usbdevice.svg");
    addDeviceWidget(new CDRomWidget(this), "cdrom.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new OtherInputdeviceWidget(this), "otherinputdevice.svg");
    addDeviceWidget(new OtherPciDeviceWidget(this), "otherpcidevice.svg");

    overviewWidget->setOverviewInfos(staticArticles);

    if(firstAdd_ == true)
    {
        leftDeviceView_->setSelectNull();
    }

    firstAdd_ = false;

    QApplication::restoreOverrideCursor();

    //leftDeviceView_->setEnabled(true);
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

    DApplication::processEvents();
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
    leftDeviceView_->setEnabled(false);
    //leftDeviceView_->setEnabled(false);
    refreshing_ = true;

    initLoadingWidget();

//    loadingWidget_->setAutoFillBackground(true);

//    auto modifyTheme = [this](){
//        DPalette pa = DApplicationHelper::instance()->palette(this);
//        QColor base_color = loadingWidget_->palette().base().color();

//        pa.setColor(QPalette::Background, base_color);
//        pa.setBrush(DPalette::ItemBackground, base_color);

//        loadingWidget_->setPalette(pa);

//        //DApplicationHelper::instance()->setPalette(this, pa);
//    };

//    modifyTheme();

//    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, modifyTheme);

    rightDeviceInfoWidget_->addWidget(loadingWidget_);
    rightDeviceInfoWidget_->setCurrentWidget(loadingWidget_);

    refreshDatabase();
}

void MainWindow::refreshDatabase()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    std::thread thread(
            []()
            {
                DeviceInfoParserInstance.refreshDabase();
            }
    );

    thread.detach();
}
//void MainWindow::refreshDatabase()
//{
//    showSplashMessage("Loading Operating System Info...");
//    DeviceInfoParserInstance.loadCatosrelelease();
//    DeviceInfoParserInstance.loadlsb_release();
//    //DeviceInfoParserInstance.getOSInfo(osInfo);

//    showSplashMessage("Loading SMBBios Info...");
//    DeviceInfoParserInstance.loadDemicodeDatabase();

//    showSplashMessage("Loading List Hardware Info");
//    DeviceInfoParserInstance.loadLshwDatabase();

//    showSplashMessage("Loading CPU Info...");
//    DeviceInfoParserInstance.loadCatcpuDatabase();
//    DeviceInfoParserInstance.loadLscpuDatabase();
//    //DeviceInfoParserInstance.loadSmartctlDatabase();

//    showSplashMessage("Loading Input Device Info...");
//    DeviceInfoParserInstance.loadCatInputDatabase();

//    showSplashMessage("Loading Power Settings...");
//    DeviceInfoParserInstance.loadPowerSettings();
//    DeviceInfoParserInstance.loadUpowerDatabase();

//    showSplashMessage("Loading Displayer Info...");
//    DeviceInfoParserInstance.loadXrandrDatabase();
//    DeviceInfoParserInstance.loadHwinfoDatabase();

//    showSplashMessage("Loading PCI Device Info...");
//    DeviceInfoParserInstance.loadLspciDatabase();

//    showSplashMessage("Loading Bluetooth Device Info...");
//    DeviceInfoParserInstance.loadHciconfigDatabase();

//    showSplashMessage("Loading USB Device Info...");
//    DeviceInfoParserInstance.loadLsusbDatabase();

//    showSplashMessage("Loading Ptinter Info...");
//    DeviceInfoParserInstance.loadCupsDatabase();
//}

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

void MainWindow::showSplashMessage(const QString& message)
{
    if( message == "finish" )
    {
        if(firstAdd_ == true)
        {
            loadDeviceWidget();
        }
        else
        {
            refreshDeviceWidget();
        }

        refreshing_ = false;
        leftDeviceView_->setEnabled(true);
        return;
    }

    if( loadLabel_ )
    {
        loadLabel_->setText(DApplication::translate("Main", message.toStdString().data()));
    }
}


