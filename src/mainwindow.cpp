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

DWIDGET_USE_NAMESPACE

QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent, QSplashScreen* ss) :
    DMainWindow(parent), splash_(ss)
{
    //setAutoFillBackground(false);
    DWidget* mainWidget = new DWidget(this);
    //mainWidget->setAutoFillBackground(true);
    mainWidget->setMaximumHeight(640);
    QHBoxLayout* ly = new QHBoxLayout;
    ly->setMargin(0);
    ly->setSpacing(0);
    ly->setSizeConstraint(QLayout::SetMinAndMaxSize);
    setFocus(Qt::FocusReason::NoFocusReason);

    leftDeviceView_ = new DeviceListView(this);
    leftDeviceView_->setMaximumWidth(220);
    leftDeviceView_->setMinimumWidth(175);

    ly->addWidget(leftDeviceView_);

    this->setFocusProxy(leftDeviceView_);

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget);
//    rightDeviceInfoWidget_->setAutoFillBackground(true);
//    rightDeviceInfoWidget_->setBackgroundRole(DPalette::Base);
//    rightDeviceInfoWidget_->setForegroundRole(DPalette::Base);

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

//    titlebar()->setAutoFillBackground(false);
//    titlebar()->setBackgroundRole(DPalette::Base);
    //titlebar()->setForegroundRole(DPalette::Base);

    //DPalette pa = DApplicationHelper::instance()->palette(this);


    //pa.setColor(QPalette::Background, base_color);
    //pa.setBrush(DPalette::ItemBackground, titlebar()->palette().base());

    //DApplicationHelper::instance()->setPalette(this, pa);

    //titlebar()->setBackgroundRole(DPalette::Base);
}

MainWindow::~MainWindow()
{

}

void MainWindow::addAllDeviceinfoWidget()
{
    DeviceInfoParserInstance.getRootPassword();

    if( firstAdd_ == true && splash_ )
    {
        splash_->show();
        QApplication::processEvents();
    }

    staticArticles.clear();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    refreshDatabase();

    auto overviewWidget = new ComputerOverviewWidget(this);
    addDeviceWidget(overviewWidget, "概况.svg");

//    if(firstAdd_ == true)
//    {
//        leftDeviceView_->addSeperator();
//    }

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

//    if( firstAdd_ == true && splash_ != nullptr )
//    {
//        splash_->finish(this);
//        //Dtk::Widget::moveToCenter(this);
//        //delete splash_;
//        splash_ = nullptr;
//    }

    firstAdd_ = false;
    QApplication::restoreOverrideCursor();
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
    //move(100, 100);
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
}

void MainWindow::refreshDatabase()
{
    showSplashMessage("Loading Operating System Info...");
    DeviceInfoParserInstance.loadCatosrelelease();
    DeviceInfoParserInstance.loadlsb_release();
    //DeviceInfoParserInstance.getOSInfo(osInfo);

    showSplashMessage("Loading SMBBios Info...");
    DeviceInfoParserInstance.loadDemicodeDatabase();

    showSplashMessage("Loading List Hardware Info");
    DeviceInfoParserInstance.loadLshwDatabase();

    showSplashMessage("Loading CPU Info...");
    DeviceInfoParserInstance.loadCatcpuDatabase();
    DeviceInfoParserInstance.loadLscpuDatabase();
    //DeviceInfoParserInstance.loadSmartctlDatabase();

    showSplashMessage("Loading Input Device Info...");
    DeviceInfoParserInstance.loadCatInputDatabase();

    showSplashMessage("Loading Power Settings...");
    DeviceInfoParserInstance.loadPowerSettings();
    DeviceInfoParserInstance.loadUpowerDatabase();

    showSplashMessage("Loading Displayer Info...");
    DeviceInfoParserInstance.loadXrandrDatabase();
    DeviceInfoParserInstance.loadHwinfoDatabase();

    showSplashMessage("Loading PCI Device Info...");
    DeviceInfoParserInstance.loadLspciDatabase();

    showSplashMessage("Loading Bluetooth Device Info...");
    DeviceInfoParserInstance.loadHciconfigDatabase();

    showSplashMessage("Loading USB Device Info...");
    DeviceInfoParserInstance.loadLsusbDatabase();

    showSplashMessage("Loading Ptinter Info...");
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

void MainWindow::showSplashMessage(const QString& message)
{
    if(firstAdd_ == false || splash_ == nullptr)
    {
        return;
    }

    splash_->showMessage( DApplication::translate("Main", message.toStdString().data()));
    QApplication::processEvents();
}


