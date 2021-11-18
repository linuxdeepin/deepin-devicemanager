/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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
#include "PageDriverControl.h"
#include "GetDriverPathWidget.h"
#include "GetDriverNameWidget.h"
#include "DriverWaitingWidget.h"
#include "DBusDriverInterface.h"
#include "DBusInterface.h"
#include "drivericonwidget.h"

#include <DBlurEffectWidget>
#include <DWidget>
#include <DApplicationHelper>
#include <DPalette>
#include <DTitlebar>
#include <DIconButton>

#include <QVBoxLayout>
#include <QDBusConnection>
#include <QWindow>
#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace PolkitQt1;

PageDriverControl::PageDriverControl(QWidget *parent, QString operation, bool install, QString deviceName, QString driverName, QString printerVendor, QString printerModel)
    : DDialog(parent)
    , mp_stackWidget(new DStackedWidget)
    , m_Install(install)
    , m_DriverName(driverName)
    , m_printerVendor(printerVendor)
    , m_printerModel(printerModel)
    , m_deviceName(deviceName)
{
    setObjectName("PageDriverControl");
    setFixedSize(480, 335);
    setOnButtonClickedClose(false);
    setWindowTitle(QString("%1-%2").arg(operation).arg(deviceName));
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setIcon(QIcon::fromTheme("deepin-devicemanager"));

    DBlurEffectWidget *widget = findChild<DBlurEffectWidget *>();
    widget->lower();
    widget->setBlurEnabled(false);
    setAttribute(Qt::WA_NoSystemBackground, false);

    DWidget *cenWidget = new DWidget;
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addWidget(mp_stackWidget);
    cenWidget->setLayout(vLayout);
    if (m_Install) {
        initInstallWidget();
    } else {
        initUninstallWidget();
    }
    addContent(cenWidget);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    connect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
}

bool PageDriverControl::isRunning()
{
    for (auto w : qApp->allWindows()) {
        if ("PageDriverControlWindow" == w->objectName()) {
            if (w->isVisible())
                return true;
        }
    }
    return false;
}

void PageDriverControl::initInstallWidget()
{
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
}

void PageDriverControl::slotBtnCancel()
{
    this->close();
}

void PageDriverControl::slotBtnNext()
{
    if (m_Install) {
        installDriverLogical();
    } else {
        uninstallDriverLogical();
    }
}

void PageDriverControl::slotProcessChange(qint32 value, QString detail)
{
    Q_UNUSED(detail)
    mp_WaitDialog->setValue(value);
}

void PageDriverControl::slotProcessEnd(bool sucess, QString errMsg)
{
    qInfo() << errMsg;
    QString successStr = m_Install ? tr("Update successful") : tr("Uninstallation successful");
    QString failedStr = m_Install ? tr("Update failed") : tr("Uninstallation failed");
    QString status = sucess ? successStr : failedStr;
    QString iconPath = sucess ? "success" : "fail";
    QIcon icon(QIcon::fromTheme(iconPath));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, status, errMsg, this);
    mp_stackWidget->addWidget(widget);
    this->addButton(tr("OK", "button"), true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotClose);
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processChange, this, &PageDriverControl::slotProcessChange);
    disconnect(DBusDriverInterface::getInstance(), &DBusDriverInterface::processEnd, this, &PageDriverControl::slotProcessEnd);
    setProperty("DriverProcessStatus" , "Done");//卸载或者加载程序已完成
    enableCloseBtn(true);
}

void PageDriverControl::slotClose()
{
    emit refreshInfo();
    this->close();
}

void PageDriverControl::slotBackPathPage()
{
    mp_stackWidget->setCurrentIndex(0);
    getButton(1)->setDisabled(false);
    this->setButtonText(1, tr("Next", "button"));
    this->setButtonText(0, tr("Cancel", "button"));
    this->getButton(0)->disconnect();
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBtnCancel);
}

void PageDriverControl::removeBtn()
{
    clearButtons();
}

void PageDriverControl::installDriverLogical()
{
    int curIndex = mp_stackWidget->currentIndex();
    if (0 == curIndex) {
        QString path = mp_PathDialog->path();
        QFile file(path);
        bool includeSubdir = mp_PathDialog->includeSubdir();
        mp_PathDialog->updateTipLabelText("");
        if (path.isEmpty() || !file.exists()) {
            mp_PathDialog->updateTipLabelText(tr("The selected folder does not exist, please select again"));
            return;
        }
        mp_NameDialog->loadAllDrivers(includeSubdir, path);
        mp_stackWidget->setCurrentIndex(1);
        this->setButtonText(1, tr("Update", "button"));
        this->setButtonText(0, tr("Previous", "button"));
        getButton(1)->setDisabled(true);          //默认置灰
        this->getButton(0)->disconnect();
        connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBackPathPage);
    } else if (1 == curIndex) {
        // 驱动安装之前需要先提权
        Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                 UnixProcessSubject(getpid()),
                                                 Authority::AllowUserInteraction);
        if (result != Authority::Yes) {
            return;
        }

        QString driveName = mp_NameDialog->selectName();
        //先判断是否是驱动文件，如果不是，再判断是否存在。
        //因为后台isDriverPackage返回false的情况有2种：1.文件不存在 2.不是驱动文件
        mp_NameDialog->updateTipLabelText("");
        if (!installErrorTips(driveName))
            return;
        removeBtn();
        mp_WaitDialog->setValue(0);
        mp_WaitDialog->setText(tr("Updating"));
        mp_stackWidget->setCurrentIndex(2);
        DBusDriverInterface::getInstance()->installDriver(driveName);
        setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
        enableCloseBtn(false);
    }
}

bool PageDriverControl::installErrorTips(const QString &driveName)
{
    QFile file(driveName);
    if(!DBusDriverInterface::getInstance()->isDebValid(driveName)) {
        mp_NameDialog->updateTipLabelText(tr("Broken package"));
        return false;
    }
    if (!DBusDriverInterface::getInstance()->isArchMatched(driveName)) {
        mp_NameDialog->updateTipLabelText(tr("Unmatched package architecture"));
        return false;
    }

    if (!DBusDriverInterface::getInstance()->isDriverPackage(driveName)) {
        if (driveName.isEmpty() || !file.exists()) {
            mp_NameDialog->updateTipLabelText(tr("The selected file does not exist, please select again"));
            return false;
        }
        mp_NameDialog->updateTipLabelText(tr("It is not a driver"));
        return false;
    }
    if (driveName.isEmpty() || !file.exists()) {
        mp_NameDialog->updateTipLabelText(tr("The selected file does not exist, please select again"));
        return false;
    }
    return true;
}

void PageDriverControl::uninstallDriverLogical()
{
    // 点击卸载之后进入正在卸载界面
    removeBtn();
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    if(m_printerVendor.count() > 0) {
        DBusDriverInterface::getInstance()->uninstallPrinter(m_printerVendor, m_printerModel);
    } else {
        DBusDriverInterface::getInstance()->uninstallDriver(m_DriverName);
    }
    setProperty("DriverProcessStatus", "Doing");//卸载或者加载程序进行中
    enableCloseBtn(false);
}

void PageDriverControl::keyPressEvent(QKeyEvent *event)
{
    // Esc、Alt+F4：如果后台更新时，直接返回。如果后台更新结束，触发close事件。如果是开始状态，则不处理
    bool blIsClose = false;
    if (event->key() == Qt::Key_F4) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers != Qt::NoModifier) {
            if (modifiers.testFlag(Qt::AltModifier)) {
                blIsClose = true;
            }
        }
    }
    if (Qt::Key_Escape == event->key()) {
        blIsClose = true;
    }
    if(!blIsClose)
    {
        return;
    }
    if("Doing" == property("DriverProcessStatus").toString()){
        return;
    }
    else if("Done" == property("DriverProcessStatus").toString()){
        slotClose();//卸载或者更新后，关闭时刷新
    }else{
        this->close();
    }
}
void PageDriverControl::closeEvent(QCloseEvent *event)
{
    // 如果后台更新时，直接返回，否则不处理
    if("Doing" == property("DriverProcessStatus").toString()){
        event->ignore();
        return;
    }else if("Done" == property("DriverProcessStatus").toString()){
        slotClose();//卸载或者更新后，关闭时刷新
    }
}

void PageDriverControl::enableCloseBtn(bool enable)
{
    // 获取titlebar
    DTitlebar *titlebar = findChild<DTitlebar *>();
    if(!titlebar){
        return;
    }

    // 获取安装按钮
    DIconButton *closeBtn = titlebar->findChild<DIconButton *>("DTitlebarDWindowCloseButton");
    if(!closeBtn){
        return;
    }

    // 禁用按钮
    closeBtn->setAttribute(Qt::WA_TransparentForMouseEvents,!enable);
    if(enable){
        closeBtn->setIcon(style()->standardIcon(style()->SP_TitleBarCloseButton));//还原
    }else {
        QIcon icon = style()->standardIcon(style()->SP_TitleBarCloseButton);
        QPixmap pm(icon.pixmap(closeBtn->size()));
        QPixmap t(pm);
        t.fill(Qt::transparent);
        QPainter p(&t);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.drawPixmap(0,0,pm);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(t.rect(),QColor(0,0,0,102));//255*2/5=102
        p.end();
        pm=t;
        closeBtn->setIcon(pm);
    }
}
