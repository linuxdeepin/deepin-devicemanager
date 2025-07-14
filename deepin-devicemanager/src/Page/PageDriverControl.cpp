// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageDriverControl.h"
#include "GetDriverPathWidget.h"
#include "GetDriverNameWidget.h"
#include "DriverWaitingWidget.h"
#include "DBusDriverInterface.h"
#include "DBusAnythingInterface.h"
#include "DBusInterface.h"
#include "drivericonwidget.h"
#include "HttpDriverInterface.h"
#include "DDLog.h"

#include <DBlurEffectWidget>
#include <DWidget>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DTitlebar>
#include <DIconButton>

#include <QVBoxLayout>
#include <QDBusConnection>
#include <QWindow>

#include <QFile>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <polkit-qt5-1/PolkitQt1/Authority>
#else
#include <polkit-qt6-1/PolkitQt1/Authority>
#endif

#define EUNKNOW         0
#define E_FILE_NOT_EXISTED 100 // file_not_existed 文件不存在，主要比如点击下一步时拔掉优盘
#define E_NOT_DRIVER      101 // not driver 非驱动文件
#define E_NOT_SIGNED      102 // not signed 没有数字签名

using namespace PolkitQt1;
using namespace DDLog;

PageDriverControl::PageDriverControl(QWidget *parent, QString operation, bool install, QString deviceName, QString driverName, QString printerVendor, QString printerModel)
    : DDialog(parent)
    , mp_stackWidget(new DStackedWidget)
    , m_Install(install)
    , m_DriverName(driverName)
    , m_printerVendor(printerVendor)
    , m_printerModel(printerModel)
    , m_deviceName(deviceName)
{
    qCDebug(appLog) << "PageDriverControl constructor start, operation:" << operation
                   << "device:" << deviceName << "driver:" << driverName;
    setObjectName("PageDriverControl");
    setFixedSize(480, 335);
    setOnButtonClickedClose(false);
    setWindowTitle(QString("%1-%2").arg(operation).arg(deviceName));
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setIcon(QIcon::fromTheme("deepin-devicemanager"));
    initErrMsg();

    DBlurEffectWidget *widget = findChild<DBlurEffectWidget *>();
    if (nullptr != widget) {
        qCDebug(appLog) << "PageDriverControl constructor, find DBlurEffectWidget";
        widget->lower();
        widget->setBlurEnabled(false);
    }
    setAttribute(Qt::WA_NoSystemBackground, false);

    DWidget *cenWidget = new DWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(mp_stackWidget);
    cenWidget->setLayout(vLayout);
    if (m_Install) {
        qCDebug(appLog) << "PageDriverControl constructor, init install widget";
        initInstallWidget();
    } else {
        qCDebug(appLog) << "PageDriverControl constructor, init uninstall widget";
        initUninstallWidget();
    }
    addContent(cenWidget);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
}

bool PageDriverControl::isRunning()
{
    qCDebug(appLog) << "PageDriverControl::isRunning start";
    for (auto w : qApp->allWindows()) {
        // qCDebug(appLog) << "PageDriverControl::isRunning check window:" << w->objectName();
        if ("PageDriverControlWindow" == w->objectName()) {
            if (w->isVisible()) {
                // qCDebug(appLog) << "PageDriverControl::isRunning is running";
                return true;
            }
        }
    }
    qCDebug(appLog) << "PageDriverControl::isRunning not running";
    return false;
}

void PageDriverControl::initInstallWidget()
{
    qCDebug(appLog) << "PageDriverControl::initInstallWidget start";
    // 先添加用户选择驱动文件所在目录路径的界面
    mp_PathDialog = new GetDriverPathWidget(this);
    mp_stackWidget->addWidget(mp_PathDialog);
    // 添加用户选择驱动文件名称的界面
    mp_NameDialog = new GetDriverNameWidget(this);
    mp_stackWidget->addWidget(mp_NameDialog);
    // 添加安装等待界面
    mp_WaitDialog = new DriverWaitingWidget(tr("Updating"), this);
    mp_stackWidget->addWidget(mp_WaitDialog);

    mp_stackWidget->setCurrentIndex(0);
    addContent(mp_stackWidget);
    this->addButton(tr("Cancel", "button"), true);
    this->addButton(tr("Next"), true, DDialog::ButtonRecommend);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
    connect(this->getButton(1), &QPushButton::clicked, this, &PageDriverControl::slotBtnNext);
    connect(mp_PathDialog, &GetDriverPathWidget::signalNotLocalFolder, this, [ = ](bool isLocal) {
        qCDebug(appLog) << "PageDriverControl::initInstallWidget signalNotLocalFolder isLocal:" << isLocal;
        if (!isLocal)
            getButton(1)->setDisabled(true);
        else
            getButton(1)->setDisabled(false);
    });
    connect(mp_NameDialog, &GetDriverNameWidget::signalDriversCount, this, [ = ] {getButton(1)->setDisabled(true);});
    connect(mp_NameDialog, &GetDriverNameWidget::signalItemClicked, this, [ = ] {getButton(1)->setDisabled(false);});
}

void PageDriverControl::initUninstallWidget()
{
    qCDebug(appLog) << "PageDriverControl::initUninstallWidget start";
    // 先添加警告界面
    QIcon icon(QIcon::fromTheme("cautious"));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, tr("Warning"), tr("The device will be unavailable after the driver uninstallation"), this);
    mp_stackWidget->addWidget(widget);
    mp_stackWidget->setCurrentIndex(0);
    this->addButton(tr("Cancel", "button"), true);
    this->addButton(tr("Uninstall", "button"), true, DDialog::ButtonWarning);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
    connect(this->getButton(1), &QPushButton::clicked, this, &PageDriverControl::slotBtnNext);

    mp_WaitDialog = new DriverWaitingWidget(tr("Uninstalling"), this);
    mp_stackWidget->addWidget(mp_WaitDialog);
    qCDebug(appLog) << "PageDriverControl::initUninstallWidget end";
}

void PageDriverControl::slotBtnCancel()
{
    qCDebug(appLog) << "PageDriverControl::slotBtnCancel";
    this->close();
}

void PageDriverControl::slotBtnNext()
{
    qCDebug(appLog) << "Next button clicked, operation:" << (m_Install ? "Install" : "Uninstall");
    if (m_Install) {
        qCDebug(appLog) << "PageDriverControl::slotBtnNext install driver";
        installDriverLogical();
    } else {
        qCDebug(appLog) << "PageDriverControl::slotBtnNext uninstall driver";
        uninstallDriverLogical();
    }
}

void PageDriverControl::slotProcessChange(qint32 value, QString detail)
{
    qCDebug(appLog) << "PageDriverControl::slotProcessChange, value:" << value << "detail:" << detail;
    Q_UNUSED(detail)
    mp_WaitDialog->setValue(value);
}

void PageDriverControl::slotProcessEnd(bool sucess, QString errCode)
{
    qCDebug(appLog) << "Process ended, success:" << sucess << "error:" << errCode;
    QString successStr = m_Install ? tr("Update successful") : tr("Uninstallation successful");
    QString failedStr = m_Install ? tr("Update failed") : tr("Uninstallation failed");
    QString status = sucess ? successStr : failedStr;
    QString iconPath = sucess ? "success" : "fail";
    QIcon icon(QIcon::fromTheme(iconPath));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, status, sucess ? "" : errMsg(errCode), this);
    mp_stackWidget->addWidget(widget);
    this->addButton(tr("OK", "button"), true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotClose);
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
    setProperty("DriverProcessStatus", "Done"); //卸载或者加载程序已完成
    enableCloseBtn(true);
    qCDebug(appLog) << "PageDriverControl::slotProcessEnd end";
}

void PageDriverControl::slotClose()
{
    qCDebug(appLog) << "PageDriverControl::slotClose";
    emit refreshInfo();
    this->close();
}

void PageDriverControl::slotBackPathPage()
{
    qCDebug(appLog) << "PageDriverControl::slotBackPathPage";
    mp_NameDialog->stopLoadingDrivers();
    mp_stackWidget->setCurrentIndex(0);
    this->setButtonText(1, tr("Next", "button"));
    this->setButtonText(0, tr("Cancel", "button"));
    this->getButton(0)->disconnect();
    getButton(1)->setEnabled(true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
}

void PageDriverControl::removeBtn()
{
    qCDebug(appLog) << "PageDriverControl::removeBtn";
    clearButtons();
}

void PageDriverControl::installDriverLogical()
{
    qCDebug(appLog) << "PageDriverControl::installDriverLogical start";
    int curIndex = mp_stackWidget->currentIndex();
    if (0 == curIndex) {
        qCDebug(appLog) << "PageDriverControl::installDriverLogical at path page";
        QString path = mp_PathDialog->path();
        QFile file(path);
        bool includeSubdir = mp_PathDialog->includeSubdir();
        mp_PathDialog->updateTipLabelText("");
        if (path.isEmpty() || !file.exists()) {
            qCWarning(appLog) << "PageDriverControl::installDriverLogical path is empty or not exists";
            mp_PathDialog->updateTipLabelText(tr("The selected folder does not exist, please select again"));
            return;
        }
        mp_stackWidget->setCurrentIndex(1);
        mp_NameDialog->loadAllDrivers(includeSubdir, path);
        this->setButtonText(1, tr("Update", "button"));
        this->setButtonText(0, tr("Previous", "button"));
        getButton(1)->setDisabled(true);          //默认置灰
        this->getButton(0)->disconnect();
        connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBackPathPage);
    } else if (1 == curIndex) {
        qCDebug(appLog) << "PageDriverControl::installDriverLogical at name page";
        // 驱动安装之前需要先提权
        Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                                 UnixProcessSubject(getpid()),
                                                                                 Authority::AllowUserInteraction);
        if (result != Authority::Yes) {
            qCWarning(appLog) << "PageDriverControl::installDriverLogical no authorization";
            return;
        }

        QString driveName = mp_NameDialog->selectName();
        //先判断是否是驱动文件，如果不是，再判断是否存在。
        //因为后台isDriverPackage返回false的情况有2种：1.文件不存在 2.不是驱动文件
        mp_NameDialog->updateTipLabelText("");
        if (!installErrorTips(driveName)) {
            qCWarning(appLog) << "PageDriverControl::installDriverLogical install error tips";
            return;
        }
        removeBtn();
        mp_WaitDialog->setValue(0);
        mp_WaitDialog->setText(tr("Updating"));
        mp_stackWidget->setCurrentIndex(2);
        DBusDriverInterface::getInstance()->installDriver(driveName);
        setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
        enableCloseBtn(false);
    }
    qCDebug(appLog) << "PageDriverControl::installDriverLogical end";
}

bool PageDriverControl::installErrorTips(const QString &driveName)
{
    qCDebug(appLog) << "Validating driver package:" << driveName;
    QFile file(driveName);
    if (!DBusDriverInterface::getInstance()->isDebValid(driveName)) {
        qCWarning(appLog) << "Invalid/broken package:" << driveName;
        mp_NameDialog->updateTipLabelText(tr("Broken package"));
        return false;
    }
    if (!DBusDriverInterface::getInstance()->isArchMatched(driveName)) {
        qCWarning(appLog) << "Architecture mismatch for package:" << driveName;
        mp_NameDialog->updateTipLabelText(tr("Unmatched package architecture"));
        return false;
    }

    if (driveName.isEmpty() || !file.exists()) {
        qCWarning(appLog) << "Driver file not found:" << driveName;
        mp_NameDialog->updateTipLabelText(tr("The selected file does not exist, please select again"));
        return false;
    }
    qCDebug(appLog) << "Driver package validation passed:" << driveName;
    return true;
}

void PageDriverControl::uninstallDriverLogical()
{
    qCDebug(appLog) << "PageDriverControl::uninstallDriverLogical start";
    // 点击卸载之后进入正在卸载界面
    removeBtn();
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    if (m_printerVendor.count() > 0) {
        DBusDriverInterface::getInstance()->uninstallPrinter(m_printerVendor, m_printerModel);
    } else {
        DBusDriverInterface::getInstance()->uninstallDriver(m_DriverName);
    }
    setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
    enableCloseBtn(false);
    qCDebug(appLog) << "PageDriverControl::uninstallDriverLogical end";
}

void PageDriverControl::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(appLog) << "PageDriverControl::keyPressEvent key:" << event->key();
    // Esc、Alt+F4：如果后台更新时，直接返回。如果后台更新结束，触发close事件。如果是开始状态，则不处理
    bool blIsClose = false;
    if (event->key() == Qt::Key_F4) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::AltModifier)) {
                qCDebug(appLog) << "keyPressEvent: Alt+F4 pressed";
                blIsClose = true;
            }
        }
    }
    if (Qt::Key_Escape == event->key()) {
        qCDebug(appLog) << "keyPressEvent: Escape key pressed";
        blIsClose = true;
    }
    if (!blIsClose) {
        qCDebug(appLog) << "keyPressEvent: not a close event, key:" << event->key();
        return;
    }
    if ("Doing" == property("DriverProcessStatus").toString()) {
        qCDebug(appLog) << "keyPressEvent: driver processing, ignore close event";
        return;
    } else if ("Done" == property("DriverProcessStatus").toString()) {
        qCDebug(appLog) << "keyPressEvent: driver process done, closing";
        slotClose();//卸载或者更新后，关闭时刷新
    } else {
        qCDebug(appLog) << "keyPressEvent: closing window";
        this->close();
    }
}
void PageDriverControl::closeEvent(QCloseEvent *event)
{
    qCDebug(appLog) << "closeEvent triggered";
    // 如果后台更新时，直接返回，否则不处理
    if ("Doing" == property("DriverProcessStatus").toString()) {
        qCDebug(appLog) << "closeEvent: driver processing, ignore close event";
        event->ignore();
        return;
    } else if ("Done" == property("DriverProcessStatus").toString()) {
        qCDebug(appLog) << "closeEvent: driver process done, closing";
        slotClose();//卸载或者更新后，关闭时刷新
    }
}

void PageDriverControl::enableCloseBtn(bool enable)
{
    qCDebug(appLog) << "PageDriverControl::enableCloseBtn enable:" << enable;
    // 获取titlebar
    DTitlebar *titlebar = findChild<DTitlebar *>();
    if (!titlebar) {
        qCWarning(appLog) << "PageDriverControl::enableCloseBtn titlebar not found";
        return;
    }

    // 获取安装按钮
    DIconButton *closeBtn = titlebar->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if (!closeBtn) {
        qCWarning(appLog) << "PageDriverControl::enableCloseBtn close button not found";
        return;
    }

    // 禁用按钮
    closeBtn->setAttribute(Qt::WA_TransparentForMouseEvents, !enable);
    closeBtn->setEnabled(enable);
}

void PageDriverControl::initErrMsg()
{
    qCDebug(appLog) << "PageDriverControl::initErrMsg";
    // 初始化错误消息
    m_MapErrMsg.insert(EUNKNOW,tr("Unknown error"));
    m_MapErrMsg.insert(ENOENT,tr("The driver module was not found")); // 2	未发现该驱动模块 /* No such file or directory */
    m_MapErrMsg.insert(ENOEXEC,tr("Invalid module format"));          // 8   模块格式无效 当该驱动已经编译到内核实，安装相同驱动的.ko文件会出现这个错误
    m_MapErrMsg.insert(EAGAIN,tr("The driver module has dependencies"));  // 11	驱动模块被依赖
    m_MapErrMsg.insert(E_FILE_NOT_EXISTED,tr("The selected file does not exist, please select again"));
    m_MapErrMsg.insert(E_NOT_DRIVER,tr("It is not a driver"));
    m_MapErrMsg.insert(E_NOT_SIGNED,tr("Unable to install - no digital signature"));
//    m_MapErrMsg.insert(EBADF,tr(""));                // EBADF		9	/* Bad file number */
//    m_MapErrMsg.insert(EEXIST,tr(""));               // EEXIST		17	/* File exists */
//    m_MapErrMsg.insert(ENODEV,tr(""));               // ENODEV		19	/* No such device */
//    m_MapErrMsg.insert(EROFS,tr(""));               // EROFS		30	/* Read-only file system */
}

const QString &PageDriverControl::errMsg(const QString &errCode)
{
    qCDebug(appLog) << "PageDriverControl::errMsg errCode:" << errCode;
    // 将错误码转换为错误信息
    if (m_MapErrMsg.find(errCode.toInt()) != m_MapErrMsg.end()) {
        qCDebug(appLog) << "PageDriverControl::errMsg errCode:" << errCode.toInt();
        return m_MapErrMsg[errCode.toInt()];
    } else {
        qCWarning(appLog) << "PageDriverControl::errMsg errCode UNKNOW";
        return m_MapErrMsg[EUNKNOW];
    }
}
