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
    staticArticles.clear();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    refreshDatabase();

    auto overviewWidget = new ComputerOverviewWidget(this);
    addDeviceWidget(overviewWidget);

    addDeviceWidget(new CpuWidget(this));
    addDeviceWidget(new MotherboardWidget(this));
    addDeviceWidget(new MemoryWidget(this));
    addDeviceWidget(new DiskWidget(this));
    addDeviceWidget(new KeyboardWidget(this));
    addDeviceWidget(new MouseWidget(this));
    addDeviceWidget(new PowerWidget(this));
    addDeviceWidget(new PrinterWidget(this));

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new DisplayadapterWidget(this));
    addDeviceWidget(new AudiodeviceWidget(this));
    addDeviceWidget(new NetworkadapterWidget(this));
    addDeviceWidget(new BluetoothWidget(this));
    addDeviceWidget(new MonitorWidget(this));
    addDeviceWidget(new CameraWidget(this));
    addDeviceWidget(new UsbdeviceWidget(this));

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new OtherInputdeviceWidget(this));
    addDeviceWidget(new OtherPciDeviceWidget(this));

    overviewWidget->setOverviewInfos(staticArticles);

    firstAdd_ = false;
    QApplication::restoreOverrideCursor();
}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase* w)
{
    if(w == nullptr)
    {
        return;
    }

    if(firstAdd_ == true)
    {
        leftDeviceView_->addDevice(w->getDeviceName(), ":images/cpu.svg");
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
    //QString osInfo;
    DeviceInfoParserInstance.loadCatosrelelease();
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
