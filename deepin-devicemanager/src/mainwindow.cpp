/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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
#include "Widget/devicelistview.h"
#include <QHBoxLayout>
#include "DStackedWidget"

#include "Widget/computeroverviewwidget.h"
#include "Widget/motherboardwidget.h"
#include "Widget/cpuwidget.h"
#include "Widget/memorywidget.h"
#include "Widget/diskwidget.h"
#include "Widget/displayadapterwidget.h"
#include "Widget/monitorwidget.h"
#include "Widget/audiodevicewidget.h"
#include "Widget/networkadapterwidget.h"
#include "Widget/bluetoothwidget.h"
#include "Widget/camerawidget.h"
#include "Widget/keyboardwidget.h"
#include "Widget/mousewidget.h"
#include "Widget/usbdevicewidget.h"
#include "Widget/otherdevicewidget.h"
#include "Widget/powerwidget.h"
#include "DeviceManager/DeviceManager.h"
#include <QStandardItemModel>
#include "Widget/otherpcidevice.h"
#include "Widget/printerwidget.h"
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
#include "Widget/cdromwidget.h"
#include <thread>
#include "commondefine.h"
#include "QStatusBar"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QScreen>
#include <QSettings>
#include <QKeyEvent>
#include "ThreadPool/ThreadPool.h"
#include <QDebug>
#include "LoadInfo/DeviceFactory.h"

DWIDGET_USE_NAMESPACE

static QList<ArticleStruct> staticArticles;

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent),
    m_sizeForQSetting(mainWindowMinWidth_, mainWindowMinHeight_),
    mp_ThreadPool(new ThreadPool(this))
{
    if (false == DeviceInfoParser::Instance().getRootPassword()) {
        exit(-1);
    }

    // 获取计算机架构信息,x86 arm mips
    QString arch = getArchString();
    mp_ThreadPool->setFramework(arch);
    DeviceFactory::setGeneratorKey(arch);

    DeviceManager::instance();
    refreshDatabase();
    setSizeLimits();
    loadSettings();

    initLoadingWidget();

    setCentralWidget(loadingWidget_);
    connect( mp_ThreadPool, &ThreadPool::finished, this, &MainWindow::showSplashMessage);

    setAttribute(Qt::WA_AcceptDrops, false);

    statusBar()->setSizeGripEnabled(true);
    statusBar()->hide();

}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::initLoadingWidget()
{
    loadingWidget_ = new DWidget(this);
    DFontSizeManager::instance()->bind( loadingWidget_, DFontSizeManager::T6);

    QVBoxLayout *vly = new QVBoxLayout;
    //vly->setMargin(0);

    vly->addStretch(1);
    auto spinner_ = new DSpinner(loadingWidget_);
    spinner_->setFixedSize(spinnerWidth, spinnerHeight);

    QHBoxLayout *hly1 = new QHBoxLayout;
    hly1->addStretch();
    hly1->addWidget(spinner_);
    hly1->addStretch();

    QHBoxLayout *hly2 = new QHBoxLayout;
    hly2->addStretch();

    loadLabel_ = new DLabel(tr("Loading..."), this);

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
    QHBoxLayout *ly = new QHBoxLayout;
    ly->setMargin(0);
    ly->setSpacing(0);
    DApplication::processEvents();

    leftDeviceView_ = new DeviceListView(mainWidget_);

    leftDeviceView_->setFixedWidth(leftDeviceListViewMinWidth_);

    DApplication::processEvents();

    ly->addWidget(leftDeviceView_, leftDeviceListViewMinWidth_);

    DApplication::processEvents();

    rightDeviceInfoWidget_ = new DStackedWidget(mainWidget_);

    DApplication::processEvents();

    addAllDeviceinfoWidget();

    ly->addWidget(rightDeviceInfoWidget_, mainWindowMinWidth_ - leftDeviceListViewMinWidth_);

    DApplication::processEvents();

    mainWidget_->setLayout(ly);

    DApplication::processEvents();

    setCentralWidget(mainWidget_);

    if (loadingWidget_) {
        loadingWidget_ = nullptr;
        loadLabel_ = nullptr;
    }

    return;
}

void MainWindow::refreshDeviceWidget()
{
    QString currentDevice = leftDeviceView_->currentDevice();

    QMap<QString, DeviceInfoWidgetBase *> oldWidgetMap;
    std::swap(deviceInfoWidgetMap_, oldWidgetMap);

    addAllDeviceinfoWidget();

    if (  deviceInfoWidgetMap_.contains(currentDevice) ) {
        rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[currentDevice]);
    }

    foreach (const QString &widgetName, oldWidgetMap.keys()) {
        rightDeviceInfoWidget_->removeWidget(oldWidgetMap[widgetName]);
        delete oldWidgetMap[widgetName];
    }

    if (loadingWidget_) {
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
    if (firstAdd_ == true) {
        leftDeviceView_->addSeperator();
    }

    addDeviceWidget(keyboardWidget, "keyboard.svg");
    addDeviceWidget(mouseWidget, "mouse.svg");
    addDeviceWidget(new PrinterWidget(mainWidget_), "printer.svg");
    addDeviceWidget(new CameraWidget(mainWidget_), "camera.svg");
    addDeviceWidget(new CDRomWidget(mainWidget_), "cdrom.svg");

    if (firstAdd_ == true) {
        leftDeviceView_->addSeperator();
    }

//    addDeviceWidget(new UsbdeviceWidget(mainWidget_), "usbdevice.svg");
    addDeviceWidget(new OtherDevicesWidget(mainWidget_), "otherdevices.svg");

    overviewWidget->setOverviewInfos(staticArticles);

    if (firstAdd_ == true) {
        leftDeviceView_->setFistSelected();
        firstAdd_ = false;
    }


    DApplication::restoreOverrideCursor();

    //leftDeviceView_->setEnabled(true);
}

void MainWindow::addDeviceWidget(DeviceInfoWidgetBase *w,  const QString &icon)
{
    if (w == nullptr || icon.isNull() || icon.isEmpty())
        return;
    QString iconName = icon;
    if (iconName.endsWith(".svg"))
        iconName.remove(".svg");

    if (firstAdd_ == true) {
        leftDeviceView_->addDevice(w->getDeviceName(), iconName);
    }

    ArticleStruct overviweInfo;
    if ( true == w->getOverViewInfo(overviweInfo) ) {
        staticArticles.push_back(overviweInfo);
    }

    rightDeviceInfoWidget_->addWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;

    DApplication::processEvents();
}


void MainWindow::insertDeviceWidget(int index, DeviceInfoWidgetBase *w)
{
    if (firstAdd_ == true) {
        leftDeviceView_->addDevice(w->getDeviceName(), ":images/cpu.svg");
    }

    rightDeviceInfoWidget_->insertWidget(index, w);
    rightDeviceInfoWidget_->setCurrentWidget(w);
    deviceInfoWidgetMap_[w->getDeviceName()] = w;
}

void MainWindow::refresh()
{
    if (refreshing_ == true) {
        return;
    }

    if (false == DeviceInfoParser::Instance().getRootPassword()) {
        return;
    }

    leftDeviceView_->setEnabled(false);

    refreshing_ = true;

    initLoadingWidget();

    rightDeviceInfoWidget_->addWidget(loadingWidget_);
    rightDeviceInfoWidget_->setCurrentWidget(loadingWidget_);

    refreshDatabase();
}

void MainWindow::refreshDatabase()
{
    DApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (mp_ThreadPool) {
        mp_ThreadPool->loadCmdInfo();
    }
}


bool MainWindow::exportTo(/*const QString& file, const QString& selectFilter*/)
{
    QString selectFilter;

    QString fileName = QDir::homePath() + "/" + QObject::tr("Device Info", "export file's name") + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") .remove(QRegExp("\\s")) + ".txt";
    QDir dir( QDir::homePath() + "/Documents/");
    if (dir.exists()) {
        fileName = QDir::homePath() + "/Documents/" + QObject::tr("Device Info", "export file's name") + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") .remove(QRegExp("\\s")) + ".txt";
    }

    QString file = DFileDialog::getSaveFileName(
                       nullptr
                       , "Export"
                       , "/home/liujun/Documents/Device Info20200701_103307.txt"
                       , "Text (*.txt);; Doc (*.docx);; Xls (*.xls);; Html (*.html)", &selectFilter
                   ); //

    if ( file.isEmpty() == true ) {
        return true;
    }

    QFileInfo fileInfo(file);
    //saveDir = fileInfo.absolutePath() + "/";

    if ( selectFilter == "Text (*.txt)" ) {
        QFile textFile( file );
        if ( false == textFile.open(QIODevice::WriteOnly)) {
            return false;
        }

        for (int i = 0; i < leftDeviceView_->count(); ++i) {
            QString device = leftDeviceView_->indexString(i);
            if ( deviceInfoWidgetMap_.contains(device) ) {
                deviceInfoWidgetMap_[device]->exportToTxt(textFile);
            }
        }

        textFile.close();
        return true;
    }

    if (selectFilter == "Html (*.html)") {
        QFile htmlFile( file );
        if ( false == htmlFile.open(QIODevice::WriteOnly)) {
            return false;
        }

        for (int i = 0; i < leftDeviceView_->count(); ++i) {
            QString device = leftDeviceView_->indexString(i);
            if ( deviceInfoWidgetMap_.contains(device) ) {
                deviceInfoWidgetMap_[device]->exportToHtml(htmlFile);
            }
        }

        htmlFile.close();

        return true;
    }

    if (selectFilter == "Doc (*.docx)") {
        Docx::Document doc(":/template.docx");

        for (int i = 0; i < leftDeviceView_->count(); ++i) {
            QString device = leftDeviceView_->indexString(i);
            if ( deviceInfoWidgetMap_.contains(device) ) {
                deviceInfoWidgetMap_[device]->exportToDoc(doc);
            }
        }

        doc.save(file);
        return true;
    }

    if ( selectFilter == "Xls (*.xls)") {
        QXlsx::Document xlsx;

        DeviceInfoWidgetBase::resetXlsRowCount();

        for (int i = 0; i < leftDeviceView_->count(); ++i) {
            QString device = leftDeviceView_->indexString(i);
            if ( deviceInfoWidgetMap_.contains(device) ) {
                deviceInfoWidgetMap_[device]->exportToXls(xlsx);
            }
        }

        xlsx.saveAs(file);
        return true;
    }

    return false;
}

void MainWindow::showDisplayShortcutsHelpDialog()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    QJsonObject windowJsonGroup;
    windowJsonGroup.insert("groupName", tr("System"));
    QJsonArray windowJsonItems;

    QJsonObject shortcutItem;
    shortcutItem.insert("name", tr("Display shortcuts"));
    shortcutItem.insert("value", "Ctrl+Shift+?");
    windowJsonItems.append(shortcutItem);

    QJsonObject closeItem;
    closeItem.insert("name", tr("Close"));
    closeItem.insert("value", "Alt+F4");
    windowJsonItems.append(closeItem);

    QJsonObject helpItem;
    helpItem.insert("name", tr("Help"));
    helpItem.insert("value", "F1");
    windowJsonItems.append(helpItem);

    QJsonObject copyItem;
    copyItem.insert("name", tr("Copy"));
    copyItem.insert("value", "Ctrl+C");
    windowJsonItems.append(copyItem);

    windowJsonGroup.insert("groupItems", windowJsonItems);
    jsonGroups.append(windowJsonGroup);

    QStringList editorKeymaps;

    QJsonObject editorJsonGroup;
    editorJsonGroup.insert("groupName", tr("Device Manager"));
    QJsonArray editorJsonItems;

    QJsonObject exportItem;
    exportItem.insert("name", tr("Export"));
    exportItem.insert("value", "Ctrl+E");
    editorJsonItems.append(exportItem);

    QJsonObject refreshItem;
    refreshItem.insert("name", tr("Refresh"));
    refreshItem.insert("value", "F5");
    editorJsonItems.append(refreshItem);

    editorJsonGroup.insert("groupItems", editorJsonItems);
    jsonGroups.append(editorJsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QProcess *shortcutViewProcess = new QProcess();
    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

void MainWindow::windowMaximizing()
{
    if (isMaximized()) {
        showNormal();
    }  else {
        //setWindowState(Qt::WindowMaximized);
        showMaximized();
    }
}

void MainWindow::currentDeviceChanged(const QString &device)
{
    if ( false == deviceInfoWidgetMap_.contains(device) ) {
        return;
    }

    rightDeviceInfoWidget_->setCurrentWidget(deviceInfoWidgetMap_[device]);
    leftDeviceView_->setCurrentDevice(device);
}

void MainWindow::showSplashMessage(const QString &message)
{
    static bool begin = true;
    static qint64 b = 0;
    static qint64 c = 0;
    if (begin) {
        b = QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    if ( message == "finish" ) {
        c = QDateTime::currentDateTime().toMSecsSinceEpoch();
        begin = true;
        qDebug() << "************************&&*************************" << (c - b) / 1000.0;
        if (firstAdd_ == true) {
            loadDeviceWidget();
        } else {
            refreshDeviceWidget();
        }

        refreshing_ = false;
        leftDeviceView_->setEnabled(true);
        return;
    }

    if ( loadLabel_ ) {
        begin = false;
        loadLabel_->setText(message);
    }
}

void MainWindow::saveSettings()
{
    QSettings setting(qApp->organizationName(), qApp->applicationName());
    setting.beginGroup("mainwindow");
    setting.setValue("size", m_sizeForQSetting);
    setting.endGroup();
}

void MainWindow::loadSettings()
{
    QSettings setting(qApp->organizationName(), qApp->applicationName());
    QSize t_size;
    setting.beginGroup("mainwindow");
    if (setting.contains("size")) {
        t_size = setting.value("size").toSize();
        if (t_size.isValid()) {
            this->resize(t_size);
        }
    }
    setting.endGroup();
}

QString MainWindow::getArchString()
{
    QProcess process;
    process.start("uname -m");
    process.waitForFinished(1000);
    QString struction = process.readAllStandardOutput().trimmed();
    process.exitCode();

#ifdef TEST_DATA_FROM_FILE
    QFile inputDeviceFile(DEVICEINFO_PATH + "/" + "uname_m.txt");
    bool res = inputDeviceFile.open(QIODevice::ReadOnly);
    if (res) {
        struction = inputDeviceFile.readAll().trimmed();
    } else {
        struction = "x86_64";
    }
    inputDeviceFile.close();
#endif

    if (struction == "aarch64") {
        QString hw = DeviceInfoParser::Instance().loadGeneratorKey();
        if (!hw.isEmpty()) {
            struction = hw;
        }
    }

    return struction;
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_E ) {
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier)) {
                exportTo();
                return;
            }
        }
    }
    if (e->key() == Qt::Key_F5 ) {
        refresh();
        return;
    }
    if (e->key() == Qt::Key_Question) {
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier)) {
                showDisplayShortcutsHelpDialog();
                return;
            }
        }
    }
    if (e->key() == Qt::Key_F) {
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            if ( modifiers.testFlag(Qt::ControlModifier) && modifiers.testFlag(Qt::AltModifier) ) {
                windowMaximizing();
                return;
            }
        }
    }
    return DMainWindow::keyPressEvent(e);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (this->windowState() == Qt::WindowState::WindowNoState) {
        m_sizeForQSetting = this->size();
    }
    DMainWindow::resizeEvent(event);
}

void MainWindow::setSizeLimits()
{
    QSize normal(mainWindowMinWidth_, mainWindowMinHeight_);

    QList<QScreen *> lst = QGuiApplication::screens();
    if (lst.size() > 0) {
        QSize rect = lst.at(0)->size();
        if ( rect.width() * 2 / 3 < normal.width() && rect.height() * 2 / 3 < normal.height() ) {
            normal.setWidth(rect.width() * 2 / 3);
            normal.setHeight(rect.height() * 2 / 3);
        }
    }
    setMinimumSize(840, 360);
    resize(normal);
}
