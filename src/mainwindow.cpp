/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "otherdevicewidget.h"
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
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent)
{
    setMinimumSize(mainWindowMinWidth_, mainWindowMinHeight_);

    DeviceInfoParserInstance.getRootPassword();

    initLoadingWidget();

    setCentralWidget(loadingWidget_);

    connect( &DeviceInfoParserInstance, &DeviceInfoParser::loadFinished, this, &MainWindow::showSplashMessage);

    refreshDatabase();

    setAttribute(Qt::WA_AcceptDrops, false);
}

MainWindow::~MainWindow()
{

}

void MainWindow::initLoadingWidget()
{
    loadingWidget_ = new DWidget(this);

//    DPalette pa = DApplicationHelper::instance()->palette(this);
//    QColor base_color = palette().base().color();

//    pa.setColor(QPalette::Background, base_color);
//    //pa.setBrush(DPalette::ItemBackground, base_color);

//    //setPalette(pa);

//    DApplicationHelper::instance()->setPalette(loadingWidget_, pa);

    //mainWidget->setAutoFillBackground(true);
    //mainWidget_->setMinimumWidth(640);
    //mainWidget_->setMinimumHeight(640);

    QVBoxLayout* vly = new QVBoxLayout;

    vly->addStretch(1);
    auto spinner_ = new DSpinner(loadingWidget_);
    spinner_->setFixedSize(spinnerWidth, spinnerHeight);

    QHBoxLayout* hly1 = new QHBoxLayout;
    hly1->addStretch();
    hly1->addWidget(spinner_);
    hly1->addStretch();

    QHBoxLayout* hly2 = new QHBoxLayout;
    hly2->addStretch();

    loadLabel_ = new DLabel("Loading...", this);
    QFont font = loadLabel_->font();
    font.setPixelSize(14);
    loadLabel_->setFont(font);
    hly2->addWidget(loadLabel_);
    hly2->addStretch();

    vly->addLayout(hly1);
    vly->addSpacing(5);
    vly->addLayout(hly2);

    vly->addStretch(1);

    spinner_->start();
    loadingWidget_->setLayout(vly);
}

void MainWindow::loadDeviceWidget()
{
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


    leftDeviceView_ = new DeviceListView(mainWidget_);

    //leftDeviceView_->setMaximumWidth(200);
    leftDeviceView_->setFixedWidth(leftDeviceListViewMinWidth_);

    DApplication::processEvents();

    ly->addWidget(leftDeviceView_, leftDeviceListViewMinWidth_);

    DApplication::processEvents();

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget_);


    auto flags = rightDeviceInfoWidget_->windowFlags();
    //setAttribute(Qt::)

    DApplication::processEvents();

    addAllDeviceinfoWidget();

//    connect(leftDeviceView_, &DeviceListView::pressed, [this](const QModelIndex& index)
//                {
//                    QString device = index.data().toString();
//                    currentDeviceChanged(device);
//            }
//    );

    ly->addWidget(rightDeviceInfoWidget_, mainWindowMinWidth_ - leftDeviceListViewMinWidth_);

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

    auto overviewWidget = new ComputerOverviewWidget(mainWidget_);
    addDeviceWidget(overviewWidget, "overview.svg");
    addDeviceWidget(new CpuWidget(mainWidget_), "cpu.svg");
    addDeviceWidget(new MotherboardWidget(mainWidget_), "motherboard.svg");
    addDeviceWidget(new MemoryWidget(mainWidget_), "memory.svg");
    addDeviceWidget(new DiskWidget(mainWidget_), "storage.svg");
    addDeviceWidget(new DisplayadapterWidget(mainWidget_), "displayadapter.svg");
    addDeviceWidget(new MonitorWidget(mainWidget_), "monitor.svg");
    addDeviceWidget(new NetworkadapterWidget(mainWidget_), "networkadapter.svg");
    addDeviceWidget(new AudiodeviceWidget(mainWidget_), "audiodevice.svg");

    auto keyboardWidget = new KeyboardWidget(mainWidget_);
    auto mouseWidget = new MouseWidget(mainWidget_);        //提前占用蓝牙键盘鼠标

    addDeviceWidget(new BluetoothWidget(mainWidget_), "bluetooth.svg");
    addDeviceWidget(new OtherPciDeviceWidget(mainWidget_), "otherpcidevices.svg");
    addDeviceWidget(new PowerWidget(mainWidget_), "battery.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(keyboardWidget, "keyboard.svg");
    addDeviceWidget(mouseWidget, "mouse.svg");
    addDeviceWidget(new PrinterWidget(mainWidget_), "printer.svg");
    addDeviceWidget(new CameraWidget(mainWidget_), "camera.svg");
    addDeviceWidget(new CDRomWidget(mainWidget_), "cdrom.svg");

    if(firstAdd_ == true)
    {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(new UsbdeviceWidget(mainWidget_), "usbdevice.svg");
    addDeviceWidget(new OtherDevicesWidget(mainWidget_), "otherdevices.svg");

    overviewWidget->setOverviewInfos(staticArticles);

    if(firstAdd_ == true)
    {
        leftDeviceView_->setFistSelected();
        firstAdd_ = false;
    }


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
            QString device = leftDeviceView_->indexString(i);
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
            QString device = leftDeviceView_->indexString(i);
            if( deviceInfoWidgetMap_.contains(device) )
            {
                deviceInfoWidgetMap_[device]->exportToHtml(htmlFile);
            }
        }

        htmlFile.close();

        return true;
    }

    if(selectFilter == "Doc (*.docx)")
    {
        Docx::Document doc(":/thirdlib/docx/doc_template/template.docx");

        for(int i = 0; i < leftDeviceView_->count(); ++i)
        {
            QString device = leftDeviceView_->indexString(i);
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
            QString device = leftDeviceView_->indexString(i);
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

void MainWindow::currentDeviceChanged(const QString& device)
{
    if( false == deviceInfoWidgetMap_.contains(device) )
    {
        return;
    }

    deviceInfoWidgetMap_[device]->deviceListClicked();
    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[device]);
    leftDeviceView_->setCurrentDevice(device);
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


