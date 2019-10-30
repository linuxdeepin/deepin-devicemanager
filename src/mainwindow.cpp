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
#include <thread>

DWIDGET_USE_NAMESPACE

QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    setMinimumSize(900, 780);
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
    loadingWidget_ = new DWidget;
    //mainWidget->setAutoFillBackground(true);
    //mainWidget_->setMinimumWidth(640);
    //mainWidget_->setMinimumHeight(640);

    QVBoxLayout* vly = new QVBoxLayout;

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
    ly->setMargin(0);
    ly->setSpacing(0);
    ly->setSizeConstraint(QLayout::SetMinAndMaxSize);
    //setFocus(Qt::FocusReason::NoFocusReason);

    DApplication::processEvents();

    leftDeviceView_ = new DeviceListView(this);
    leftDeviceView_->setMaximumWidth(220);
    leftDeviceView_->setMinimumWidth(175);

    DApplication::processEvents();

    ly->addWidget(leftDeviceView_);

    DApplication::processEvents();

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget_);

    DApplication::processEvents();

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
    addDeviceWidget(overviewWidget, "概况.svg");
    addDeviceWidget(new CpuWidget(this), "处理器.svg");
    addDeviceWidget(new MotherboardWidget(this), "主板.svg");
    addDeviceWidget(new MemoryWidget(this), "内存.svg");
    addDeviceWidget(new DiskWidget(this), "存储设备.svg");
    addDeviceWidget(new DisplayadapterWidget(this), "显示适配器.svg");
    addDeviceWidget(new MonitorWidget(this), "显示设备.svg");
    addDeviceWidget(new NetworkadapterWidget(this), "网络适配器.svg");
    addDeviceWidget(new AudiodeviceWidget(this), "音频适配器.svg");
    addDeviceWidget(new PowerWidget(this), "电池.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new KeyboardWidget(this), "键盘.svg");
    addDeviceWidget(new MouseWidget(this), "鼠标.svg");
    addDeviceWidget(new BluetoothWidget(this), "蓝牙.svg");
    addDeviceWidget(new PrinterWidget(this), "打印机.svg");
    addDeviceWidget(new CameraWidget(this), "图像设备.svg");
    addDeviceWidget(new UsbdeviceWidget(this), "usb设备.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new OtherInputdeviceWidget(this), "其他输入设备.svg");
    addDeviceWidget(new OtherPciDeviceWidget(this), "其他PCI设备.svg");

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
    //leftDeviceView_->setEnabled(false);

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
        return;
    }

    if( loadLabel_ )
    {
        loadLabel_->setText(DApplication::translate("Main", message.toStdString().data()));
    }
}


