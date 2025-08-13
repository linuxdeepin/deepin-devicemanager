// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "MainWindow.h"
#include "WaitingWidget.h"
#include "DeviceWidget.h"
#include "PageDriverManager.h"
#include "MacroDefinition.h"
#include "DeviceManager.h"
#include "DebugTimeManager.h"
#include "commondefine.h"
#include "LoadInfoThread.h"
#include "DeviceFactory.h"
#include "ThreadExecXrandr.h"
#include "LoadCpuInfoThread.h"
#include "CmdTool.h"
#include "commonfunction.h"
#include "DriverScanWidget.h"
#include "DDLog.h"

// Dtk头文件
#include <DFileDialog>
#include <DApplication>
#include <DFontSizeManager>
#include <DButtonBox>
#include <DTitlebar>
#include <DDialog>
#include <QShortcut>
#ifdef DTKCORE_CLASS_DConfigFile
#include <DConfig>
#endif

// Qt库文件
#include <QResizeEvent>
#include <QDateTime>
#include <QLoggingCategory>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QDir>
#include <QVBoxLayout>
#include <QTimer>
#include <QRegularExpression>

DWIDGET_USE_NAMESPACE
using namespace DDLog;
// 主界面需要的一些宏定义
#define INIT_WIDTH  1000    // 窗口的初始化宽度
#define INIT_HEIGHT 720     // 窗口的初始化高度
#define MIN_WIDTH  680      // 窗口的最小宽度
#define MIN_HEIGHT 300      // 窗口的最小高度

static bool startScanningFlag = false;
static int monitorNumber = 1;
static bool checkWaylandMode()
{
    qCDebug(appLog) << "checkWaylandMode start";
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    bool waylandMode = false;
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) { //是否开启wayland
        qCDebug(appLog) << "checkWaylandMode: wayland mode is on";
        waylandMode = true;
    }

    qCDebug(appLog) << "checkWaylandMode end, waylandMode:" << waylandMode;
    return waylandMode;
}

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
    , mp_MainStackWidget(new DStackedWidget(this))
    , mp_WaitingWidget(new WaitingWidget(this))
    , mp_DeviceWidget(new DeviceWidget(this))
    , mp_DriverScanWidget(new DriverScanWidget(this))
    , mp_DriverManager(new PageDriverManager(this))
    , mp_WorkingThread(new LoadInfoThread)
    , mp_ButtonBox(new DButtonBox(this))
{
    qCDebug(appLog) << "MainWindow constructor start";
    // 初始化窗口相关的内容，比如界面布局，控件大小
    initWindow();
    qCDebug(appLog) << "MainWindow constructor end";

    // 加载设备信息
    refreshDataBase();

    // 关联信号槽
    connect(mp_WorkingThread, &LoadInfoThread::finished, this, &MainWindow::slotLoadingFinish);
    connect(mp_DeviceWidget, &DeviceWidget::itemClicked, this, &MainWindow::slotListItemClicked);
    connect(mp_DeviceWidget, &DeviceWidget::refreshInfo, this, &MainWindow::slotRefreshInfo);
    connect(mp_DeviceWidget, &DeviceWidget::exportInfo, this, &MainWindow::slotExportInfo);
    connect(this, &MainWindow::fontChange, this, &MainWindow::slotChangeUI);
    connect(mp_DriverManager, &PageDriverManager::startScanning, this, [ = ]() {
        qCDebug(appLog) << "MainWindow startScanning triggered";
        // 正在刷新,避免重复操作
        if (m_refreshing || mp_WorkingThread->isRunning()) {
            qCDebug(appLog) << "MainWindow is refreshing or working thread is running, set startScanningFlag to true and return";
            mp_ButtonBox->setEnabled(false);
            startScanningFlag = true;
            return;
        }
        // 正在刷新标志
        m_refreshing = true;
        mp_ButtonBox->setEnabled(false);
        // 加载设备信息
        refreshDataBase();
        startScanningFlag = true;
        qCDebug(appLog) << "MainWindow startScanning finished";
    });
    connect(mp_DriverManager, &PageDriverManager::scanFinished, this, [ = ](ScanResult sr) {
        qCDebug(appLog) << "MainWindow scanFinished triggered, result:" << sr;
        mp_ButtonBox->setEnabled(true);

        if (SR_Failed == sr) {
            qCDebug(appLog) << "MainWindow scan failed";
            mp_DriverScanWidget->setScanFailedUI();
        } else if (SR_SUCESS == sr) {
            qCDebug(appLog) << "MainWindow scan success";
            mp_DriverScanWidget->setProgressFinish();
            mp_MainStackWidget->setCurrentIndex(3);
        } else if (SR_NETWORD_ERR == sr) {
            qCDebug(appLog) << "MainWindow scan network error";
            mp_DriverScanWidget->setNetworkErr();
        }
    });
    connect(mp_DriverManager, &PageDriverManager::scanInfo, this, [ = ](const QString &info, int progress) {
        qCDebug(appLog) << "MainWindow scanInfo triggered, info:" << info << "progress:" << progress;
        mp_DriverScanWidget->refreshProgress(info, progress);
    });
    connect(mp_DriverScanWidget, &DriverScanWidget::redetected, mp_DriverManager, &PageDriverManager::startScanning);
    //get monitorNumber
    ThreadExecXrandr txgpu(true, !checkWaylandMode());
    txgpu.start();
    txgpu.wait();
    monitorNumber = txgpu.getMonitorNumber();
    connect(mp_WorkingThread, &LoadInfoThread::finishedReadFilePool, this, [ = ]() {
        qCDebug(appLog) << "MainWindow finishedReadFilePool triggered";
        refreshDataBaseLater();
    });
}
void MainWindow::refreshDataBaseLater()
{
    qCDebug(appLog) << "MainWindow::refreshDataBaseLater start";
    DBusInterface::getInstance()->refreshInfo();
    QTimer::singleShot(2000, this, [ = ]() {  //test the x86 the fast desktop PC need 2s
        qCDebug(appLog) << "MainWindow::refreshDataBaseLater timeout, refresh database";
        // 加载设备信息
        refreshDataBase();
    });
    qCDebug(appLog) << "MainWindow::refreshDataBaseLater end";
}

MainWindow::~MainWindow()
{
    qCDebug(appLog) << "MainWindow destructor start";
    // 释放指针
    if (mp_WorkingThread && mp_WorkingThread->isRunning()) {
        qCWarning(appLog) << "Terminating running working thread";
        mp_WorkingThread->terminate();
    }
    while (mp_WorkingThread && mp_WorkingThread->isRunning()) {}
    if (mp_WaitingWidget) {
        qCDebug(appLog) << "MainWindow destructor delete waiting widget";
        delete mp_WaitingWidget;
        mp_WaitingWidget = nullptr;
    }
    if (mp_DeviceWidget) {
        qCDebug(appLog) << "MainWindow destructor delete device widget";
        delete mp_DeviceWidget;
        mp_DeviceWidget = nullptr;
    }
    //    DELETE_PTR(mp_DriverManager)
    if (mp_MainStackWidget) {
        qCDebug(appLog) << "MainWindow destructor delete main stack widget";
        delete mp_MainStackWidget;
        mp_MainStackWidget = nullptr;
    }
    if (mp_WorkingThread) {
        qCDebug(appLog) << "MainWindow destructor delete working thread";
        delete mp_WorkingThread;
        mp_WorkingThread = nullptr;
    }
    qCDebug(appLog) << "MainWindow destructor end";
}

void MainWindow::refresh()
{
    qCDebug(appLog) << "MainWindow::refresh start";
    //电池状态刷新
    refreshBatteryStatus();

    // 正在刷新,避免重复操作
    if (m_refreshing || startScanningFlag || mp_DriverManager->isScanning() || mp_WorkingThread->isRunning()) {
        qCDebug(appLog) << "Refresh skipped - operation already in progress";
        return;
    }

    if (mp_ButtonBox->checkedId() == 1) {
        qCDebug(appLog) << "MainWindow::refresh driver manager page, set startScanningFlag to true";
        startScanningFlag = true;
    }
    // 正在刷新标志
    m_refreshing = true;
    mp_ButtonBox->setEnabled(false);

    mp_WaitingWidget->start();
    mp_MainStackWidget->setCurrentIndex(0);
    mp_ButtonBox->buttonList().at(0)->click();
    mp_DeviceWidget->clear();

    // 加载设备信息
    refreshDataBase();
    qCDebug(appLog) << "MainWindow::refresh end";
}

void MainWindow::refreshBatteryStatus()
{
    qCDebug(appLog) << "Refreshing battery status";
    QDBusConnection bus = QDBusConnection::systemBus();

    //创建Dbus接口
    QDBusInterface interfaceService("org.freedesktop.UPower", "/org/freedesktop/UPower", "org.freedesktop.UPower", bus);
    if (interfaceService.isValid()) {
        qCDebug(appLog) << "UPower interface valid";
         QProcess process;
         QString command = "gdbus call --system --dest org.freedesktop.UPower --object-path /org/freedesktop/UPower --method org.freedesktop.UPower.EnumerateDevices";
         process.start(command);
         process.waitForFinished();

         QByteArray output = process.readAllStandardOutput();
         QString outputStr = QString::fromLocal8Bit(output);
         QStringList outputList = outputStr.split(",");

         foreach (const QString& str, outputList) {
            // qCDebug(appLog) << "Processing battery device path:" << str;
             if (str.contains("BAT")) {
                // qCDebug(appLog) << "Found battery device";
                 QStringList pathStr = str.split("'");
                 if (pathStr.size() >= 2) {
                    // qCDebug(appLog) << "Valid battery path found:" << pathStr.at(1);
                     QDBusInterface interfaceBattery("org.freedesktop.UPower", pathStr.at(1), "org.freedesktop.UPower.Device", bus);
                     if (interfaceBattery.isValid()) {
                        // qCDebug(appLog) << "Battery interface valid, calling Refresh";
                         QDBusMessage reply = interfaceBattery.call("Refresh");
                         if (reply.type() != QDBusMessage::ReplyMessage)
                             qWarning() << "call Refresh failure:" << reply.errorMessage();
                     }
                 }
             }
         }
    } else {
       qCWarning(appLog) << "UPower interface invalid - cannot refresh battery status";
    }
    qCDebug(appLog) << "refreshBatteryStatus end";
}

bool MainWindow::exportTo()
{
    qCDebug(appLog) << "MainWindow::exportTo start";
    QString selectFilter;

    // 导出信息文件保存路径
    static QString saveDir = []() {
        // qCDebug(appLog) << "MainWindow::exportTo get save dir";
        QString dirStr = "./";
        QDir dir(QDir::homePath() + "/Desktop/");
        if (dir.exists()) {
            // qCDebug(appLog) << "MainWindow::exportTo save dir exists";
            dirStr = QDir::homePath() + "/Desktop/";
        }
        return dirStr;
    }
    ();

    // 导出信息文件名称
    QString file = DFileDialog::getSaveFileName(
                       this,
                       "Export", saveDir + tr("Device Info", "export file's name") + \
                       QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss").remove(QRegularExpression("\\s")) + ".txt", \
                       "Text (*.txt);; Doc (*.docx);; Xls (*.xls);; Html (*.html)", &selectFilter);  //

    if (file.isEmpty()) {
        qCDebug(appLog) << "MainWindow::exportTo file is empty, return";
        return true;
    }

    QFileInfo fileInfo(file);

    // 文件类型txt
    if (selectFilter == "Text (*.txt)")
        return DeviceManager::instance()->exportToTxt(file);

    // 文件类型html
    if (selectFilter == "Html (*.html)")
        return DeviceManager::instance()->exportToHtml(file);

    // 文件类型docx
    if (selectFilter == "Doc (*.docx)")
        return DeviceManager::instance()->exportToDoc(file);

    // 文件类型xls
    if (selectFilter == "Xls (*.xls)")
        return DeviceManager::instance()->exportToXlsx(file);

    qCDebug(appLog) << "MainWindow::exportTo return false!";
    return false;
}



void MainWindow::showDisplayShortcutsHelpDialog()
{
    qCDebug(appLog) << "MainWindow::showDisplayShortcutsHelpDialog start";
    QJsonDocument doc;

    //获取快捷键json文本
    getJsonDoc(doc);

    // 快捷键窗口位置
    QRect rect = this->window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);

    // 快捷键窗口显示进程
    QProcess *shortcutViewProcess = new QProcess();
    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    // 启动子进程
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

void MainWindow::addJsonArrayItem(QJsonArray &windowJsonItems, const QString &name, const QString &value)
{
    qCDebug(appLog) << "MainWindow::addJsonArrayItem name:" << name << "value:" << value;
    // 添加json数组对
    QJsonObject jsonObject;
    jsonObject.insert("name", name);
    jsonObject.insert("value", value);
    windowJsonItems.append(jsonObject);
}

void MainWindow::getJsonDoc(QJsonDocument &doc)
{
    qCDebug(appLog) << "MainWindow::getJsonDoc start";
    QJsonArray jsonGroups;

    // 窗口快捷键组
    QJsonArray windowJsonItems;

    addJsonArrayItem(windowJsonItems, tr("Display shortcuts"), "Ctrl+Shift+?");
    addJsonArrayItem(windowJsonItems, tr("Close"), "Alt+F4");
    addJsonArrayItem(windowJsonItems, tr("Help"), "F1");
    addJsonArrayItem(windowJsonItems, tr("Copy"), "Ctrl+C");

    // 窗口快捷键添加到 系统分类
    QJsonObject windowJsonGroup;
    windowJsonGroup.insert("groupName", tr("System"));
    windowJsonGroup.insert("groupItems", windowJsonItems);
    jsonGroups.append(windowJsonGroup);

    // 编辑快捷键组
    QJsonArray editorJsonItems;

    addJsonArrayItem(editorJsonItems, tr("Export"), "Ctrl+E");
    addJsonArrayItem(editorJsonItems, tr("Refresh"), "F5");

    // 编辑快捷键添加到 设备管理器分类
    QJsonObject editorJsonGroup;
    editorJsonGroup.insert("groupName", tr("Device Manager"));
    editorJsonGroup.insert("groupItems", editorJsonItems);
    jsonGroups.append(editorJsonGroup);

    // 添加快捷键组到对象
    QJsonObject shortcutObj;
    shortcutObj.insert("shortcut", jsonGroups);

    doc.setObject(shortcutObj);
}

void MainWindow::windowMaximizing()
{
    qCDebug(appLog) << "MainWindow::windowMaximizing start";
    if (!window()->windowState().testFlag(Qt::WindowMaximized)) {
        qCDebug(appLog) << "MainWindow::windowMaximizing not maximized";
        window()->setWindowState(windowState() | Qt::WindowMaximized);
    } else {
        qCDebug(appLog) << "MainWindow::windowMaximizing maximized";
        window()->setWindowState(windowState() & ~Qt::WindowMaximized);
    }
}

void MainWindow::swichStackWidget()
{
    qCDebug(appLog) << "MainWindow::swichStackWidget start";
    if (0 == mp_MainStackWidget->currentIndex())
        mp_MainStackWidget->setCurrentIndex(1);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    qCDebug(appLog) << "MainWindow::resizeEvent start";
    DMainWindow::resizeEvent(event);
}

void MainWindow::initWindow()
{
    qCDebug(appLog) << "MainWindow::initWindow start";
    //1. 第一步初始化窗口大小
    initWindowSize();

    //2. 添加标题栏按钮
    initWindowTitle();

    //3. 初始化界面布局
    initWidgets();

    //4. 初始化快捷键
    QShortcut *viewshortcut = new QShortcut(this);
    viewshortcut->setAutoRepeat(false);
    QKeySequence viewSequence(QLatin1String("F5"));
    viewshortcut->setKey(viewSequence);
    connect(viewshortcut, &QShortcut::activated, this, [ = ]() {
        refresh();
    });
}

void MainWindow::initWindowSize()
{
    qCDebug(appLog) << "MainWindow::initWindowSize start";
    // 设置窗口的最小尺寸
    QSize minSize(MIN_WIDTH, MIN_HEIGHT);
    setMinimumSize(minSize);

    // 设置窗口的大小
    QSize initSize(INIT_WIDTH, INIT_HEIGHT);
    resize(initSize);
}

void MainWindow::initWindowTitle()
{
    qCDebug(appLog) << "MainWindow::initWindowTitle start";
    QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");
    titlebar()->setIcon(appIcon);
    // 设置 DButtonBox 里面的 button
    mp_ButtonBox->setFixedWidth(242);
    mp_ButtonBox->setButtonList({new DButtonBoxButton(tr("Hardware")), new DButtonBoxButton(tr("Drivers"))}, true);
    mp_ButtonBox->setId(mp_ButtonBox->buttonList().at(0), 0);
    mp_ButtonBox->setId(mp_ButtonBox->buttonList().at(1), 1);
    mp_ButtonBox->buttonList().at(0)->click();

    connect(mp_ButtonBox, &DButtonBox::buttonClicked, this, [this](QAbstractButton * button) {
        if (mp_ButtonBox->id(button) == 0) {
            if (0 == mp_MainStackWidget->currentIndex())
                return ;
            else
                mp_MainStackWidget->setCurrentIndex(1);
        } else {
            if (mp_DriverManager->isFirstScan()) {
                mp_ButtonBox->setEnabled(false);
                mp_DriverManager->scanDriverInfo();
                mp_MainStackWidget->setCurrentIndex(2);
                mp_DriverScanWidget->setScanningUI("", 0);
            } else {
                mp_MainStackWidget->setCurrentIndex(3);
            }
        }
    });
    titlebar()->addWidget(mp_ButtonBox);
#ifdef DTKCORE_CLASS_DConfigFile
    //需要查询是否支持特殊机型静音恢复，例如hw机型
    DConfig *dconfig = DConfig::create("org.deepin.devicemanager","org.deepin.devicemanager");
    //需要判断Dconfig文件是否合法
    if(dconfig && dconfig->isValid() && dconfig->keyList().contains("specialComType")){
        Common::specialComType = dconfig->value("specialComType").toInt();
    }
    qCInfo(appLog) << "Common::specialComType value is:" << Common::specialComType;
#endif
    // 特殊处理
    if (!Common::boardVendorType().isEmpty())
        mp_ButtonBox->hide();
#ifdef DISABLE_DRIVER
    mp_ButtonBox->hide();
#endif
    // Hide hardware info and driver management buttons from title bar
    mp_ButtonBox->hide();
}

void MainWindow::initWidgets()
{
    qCDebug(appLog) << "MainWindow::initWidgets start";
    // 设置窗口的主控件
    setCentralWidget(mp_MainStackWidget);
    setContentsMargins(0, 0, 0, 0);

    // 添加加载等待界面
    mp_MainStackWidget->addWidget(mp_WaitingWidget);
    mp_WaitingWidget->start();

    // 添加信息显示界面
    mp_MainStackWidget->addWidget(mp_DeviceWidget);

    mp_MainStackWidget->addWidget(mp_DriverScanWidget);

    // 初始化驱动相关界面
    mp_MainStackWidget->addWidget(mp_DriverManager);
}

void MainWindow::refreshDataBase()
{
    qCDebug(appLog) << "Refreshing device database";
    if (mp_WorkingThread) {
        /* 一定要与 restoreOverrideCursor 成对使用*/
        if(!m_statusCursorIsWait) {
            DApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
            m_statusCursorIsWait = true;
        }
        mp_WorkingThread->start();
    }
}

void MainWindow::slotSetPage(QString page)
{
    qCDebug(appLog) << "MainWindow::slotSetPage page:" << page;
    if ("driver" == page) {
        qCDebug(appLog) << "MainWindow::slotSetPage driver";
        if (m_IsFirstRefresh) {
            qCDebug(appLog) << "MainWindow::slotSetPage driver first refresh";
            m_ShowDriverPage = true;
        } else {
            qCDebug(appLog) << "MainWindow::slotSetPage driver not first refresh";
            mp_ButtonBox->buttonList().at(1)->click();
        }
    }
}

void MainWindow::slotLoadingFinish(const QString &message)
{
    qCDebug(appLog) << "Loading finished with message:" << message;
    static bool begin = true;

    if (begin)
        begin = false;

    // finish 表示所有设备信息加载完成
    if (message == "finish") {
        qCDebug(appLog) << "MainWindow::slotLoadingFinish finish";
        begin = true;

        /* 一定要有与 setOverrideCursor 成对使用 否则指针一直显示圆圈*/
        if( m_statusCursorIsWait) {
            qCDebug(appLog) << "MainWindow::slotLoadingFinish restore cursor";
            m_statusCursorIsWait = false;
            DApplication::restoreOverrideCursor();
        }

        // 信息显示界面
        // 获取设备类型列表
        DeviceManager::instance()->setDeviceListClass();
        const QList<QPair<QString, QString>> types = DeviceManager::instance()->getDeviceTypes();

        // 获取设备驱动列表
        DeviceManager::instance()->getDeviceDriverPool();

        // 更新左侧ListView
        mp_DeviceWidget->updateListView(types);

        // 设置当前页面设备信息页
        if (mp_ButtonBox->checkedId() != 1)
            mp_MainStackWidget->setCurrentWidget(mp_DeviceWidget);

        QList<DeviceBaseInfo *> lst;
        bool ret = DeviceManager::instance()->getDeviceList(mp_DeviceWidget->currentIndex(), lst);

        if (ret && lst.size() > 0) {//当设备大小为0时，显示概况信息
            mp_DeviceWidget->updateDevice(mp_DeviceWidget->currentIndex(), lst);
        } else {
            QMap<QString, QString> overviewMap = DeviceManager::instance()->getDeviceOverview();
            mp_DeviceWidget->updateOverview(overviewMap);
        }

        if (!startScanningFlag) {
            mp_ButtonBox->setEnabled(true);
        }
        // 刷新结束
        m_refreshing = false;

        if (m_IsFirstRefresh)
            m_IsFirstRefresh = false;

        // 是否切换到驱动界面
        if (m_ShowDriverPage) {
            m_ShowDriverPage = false;
            mp_ButtonBox->buttonList().at(1)->click();
        }
    }
    if (startScanningFlag) {
        qCDebug(appLog) << "MainWindow::slotLoadingFinish start scanning";
        mp_MainStackWidget->setCurrentIndex(2);
        mp_DriverManager->scanDriverInfo();
        mp_DriverScanWidget->setScanningUI("", 0);
        startScanningFlag = false;
    }
}

void MainWindow::slotListItemClicked(const QString &itemStr)
{
    qCDebug(appLog) << "MainWindow::slotListItemClicked itemStr:" << itemStr;
    // xrandr would be execed later
    if (tr("Monitor") == itemStr || tr("Overview") == itemStr) { //点击显示设备，执行线程加载信息
        ThreadExecXrandr tx(false, !checkWaylandMode());
        tx.start();
        tx.wait();
    } else if (tr("Display Adapter") == itemStr) { //点击显示适配器，执行线程加载信息
        ThreadExecXrandr tx(true, !checkWaylandMode());
        tx.start();
        tx.wait();
    } else if (tr("CPU") == itemStr) { //点击处理器，执行加载处理器信息线程
        LoadCpuInfoThread lct;
        lct.start();
        lct.wait();
    } else if (tr("Network Adapter") == itemStr) { //点击网络适配器，更新网络连接的信息
        CmdTool tool;
        QStringList networkDriver = DeviceManager::instance()->networkDriver();
        //判断所有网卡的连接情况
        for (int i = 0; i < networkDriver.size(); i++)
            DeviceManager::instance()->correctNetworkLinkStatus(tool.getCurNetworkLinkStatus(networkDriver.at(i)), networkDriver.at(i));
    } else if (tr("Battery") == itemStr) { //点击电池，重新加载电池显示信息
        CmdTool tool;
        DeviceManager::instance()->correctPowerInfo(tool.getCurPowerInfo());
    }

        ThreadExecXrandr txgpu(true, !checkWaylandMode());
        txgpu.start();
        txgpu.wait();
        if(monitorNumber != txgpu.getMonitorNumber()) {

            QString info;
            DBusInterface::getInstance()->getInfo("is_server_running", info);
            //请求后台更新信息
            if (!info.toInt()) {
                refreshDataBaseLater();
            }
            qCDebug(appLog)<< "Monitor refreshInfo" << __LINE__  << QDateTime::currentDateTime().toString("hh:mm:ss") << info << monitorNumber;
            monitorNumber = txgpu.getMonitorNumber();
        }

    // 数据刷新时不处理界面刷新
    if (m_refreshing || mp_WorkingThread->isRunning()) {
        qCDebug(appLog) << "MainWindow::slotListItemClicked refreshing or working thread running";
        return;
    }

    QList<DeviceBaseInfo *> lst;
    bool ret = DeviceManager::instance()->getDeviceList(itemStr, lst);

    if (ret && lst.size() > 0) {//当设备大小为0时，显示概况信息
        mp_DeviceWidget->updateDevice(itemStr, lst);
    } else {
        QMap<QString, QString> overviewMap = DeviceManager::instance()->getDeviceOverview();
        mp_DeviceWidget->updateOverview(overviewMap);
    }
}

void MainWindow::slotRefreshInfo()
{
    qCDebug(appLog) << "MainWindow::slotRefreshInfo";
    refreshDataBaseLater();
    // 界面刷新
    refresh();
}

void MainWindow::slotExportInfo()
{
    qCDebug(appLog) << "MainWindow::slotExportInfo";
    // 设备信息导出
    exportTo();
}

void MainWindow::slotChangeUI()
{
    qCDebug(appLog) << "MainWindow::slotChangeUI";
    // 设置字体变化标志
    mp_DeviceWidget->setFontChangeFlag();

    // 更新当前设备界面设备
    slotListItemClicked(mp_DeviceWidget->currentIndex());
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    // qCDebug(appLog) << "MainWindow::keyPressEvent key:" << e->key();
    // ctrl+e:导出
    if (e->key() == Qt::Key_E) {
        qCDebug(appLog) << "MainWindow::keyPressEvent ctrl+e";
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            qCDebug(appLog) << "MainWindow::keyPressEvent ctrl+e modifiers:" << modifiers;
            if (modifiers.testFlag(Qt::ControlModifier)) {
                exportTo();
                return;
            }
        }
    }

    // ctrl+shift+command:快捷键提示界面
    if (e->key() == Qt::Key_Question) {
        qCDebug(appLog) << "MainWindow::keyPressEvent ctrl+shift+command";
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier)) {
                showDisplayShortcutsHelpDialog();
                return;
            }
        }
    }

    // ctrl+alt：窗口最大化
    if (e->key() == Qt::Key_F) {
        qCDebug(appLog) << "MainWindow::keyPressEvent ctrl+alt";
        Qt::KeyboardModifiers modifiers = e->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::ControlModifier) && modifiers.testFlag(Qt::AltModifier)) {
                windowMaximizing();
                return;
            }
        }
    }

    return DMainWindow::keyPressEvent(e);
}

bool MainWindow::event(QEvent *event)
{
    // qCDebug(appLog) << "MainWindow::event type:" << event->type();    
    // 字体大小改变
    if (QEvent::ApplicationFontChange == event->type()) {
        qCDebug(appLog) << "MainWindow::event font change";
        emit fontChange();
        DWidget::event(event);
    }

    return DMainWindow::event(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // qCDebug(appLog) << "MainWindow::closeEvent";
    if (mp_DriverManager->isInstalling()) {
        qCDebug(appLog) << "MainWindow::closeEvent is installing";
        // 当前界面正在驱动安装，弹窗提示
        // bug134487
        DDialog dialog(QObject::tr("You are installing a driver, which will be interrupted if you exit.")
                       , QObject::tr("Are you sure you want to exit?"));

        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.addButton(QObject::tr("Exit", "button"), false, DDialog::ButtonWarning);
        dialog.addButton(QObject::tr("Cancel", "button"));
        int ret = dialog.exec();
        if (0 == ret) {
            qCDebug(appLog) << "MainWindow::closeEvent exit";
            return DMainWindow::closeEvent(event);
        } else {
            qCDebug(appLog) << "MainWindow::closeEvent ignore";
            event->ignore();
        }
    } else if (mp_DriverManager->isBackingup()) {
        qCDebug(appLog) << "MainWindow::closeEvent is backing up";
        DDialog dialog(QObject::tr("You are backing up drivers, which will be interrupted if you exit.")
                       , QObject::tr("Are you sure you want to exit?"));

        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.addButton(QObject::tr("Exit", "button"), false, DDialog::ButtonWarning);
        dialog.addButton(QObject::tr("Cancel", "button"));
        int ret = dialog.exec();
        if (0 == ret) {
            qCDebug(appLog) << "MainWindow::closeEvent exit";
            return DMainWindow::closeEvent(event);
        } else {
            qCDebug(appLog) << "MainWindow::closeEvent ignore";
            event->ignore();
        }
    } else if (mp_DriverManager->isRestoring()) {
        qCDebug(appLog) << "MainWindow::closeEvent is restoring";
        DDialog dialog(QObject::tr("You are restoring drivers, which will be interrupted if you exit.")
                       , QObject::tr("Are you sure you want to exit?"));

        dialog.setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
        dialog.addButton(QObject::tr("Exit", "button"), false, DDialog::ButtonWarning);
        dialog.addButton(QObject::tr("Cancel", "button"));
        int ret = dialog.exec();
        if (0 == ret) {
            qCDebug(appLog) << "MainWindow::closeEvent exit";
            return DMainWindow::closeEvent(event);
        } else {
            qCDebug(appLog) << "MainWindow::closeEvent ignore";
            event->ignore();
        }
    } else {
        return DMainWindow::closeEvent(event);
    }
}
