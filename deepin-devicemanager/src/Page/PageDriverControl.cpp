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

#include <QVBoxLayout>
#include <QDBusConnection>

DWIDGET_BEGIN_NAMESPACE
PageDriverControl::PageDriverControl(QString operation, QString deviceName, QString driverName, bool install, QWidget *parent)
    : DDialog(parent)
    , mp_stackWidget(new DStackedWidget)
    , m_Install(install)
    , m_DriverName(driverName)
{
    setFixedSize(480, 335);
    setOnButtonClickedClose(false);
    setWindowTitle(QString("%1-%2").arg(operation).arg(deviceName));
    setWindowFlag(Qt::WindowMinimizeButtonHint, true);
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
    connect(mp_PathDialog, &GetDriverPathWidget::signalNotLocalFolder, this, [=](bool isLocal){
        if (!isLocal)
           getButton(1)->setDisabled(true);
        else
           getButton(1)->setDisabled(false);
    });
    connect(mp_NameDialog, &GetDriverNameWidget::signalDriversCount, this, [=]{getButton(1)->setDisabled(true);});
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

void PageDriverControl::slotProcessEnd(bool sucess)
{
    QString successStr = m_Install ? tr("Update successful") : tr("Uninstallation successful");
    QString failedStr = m_Install ? tr("Update failed") : tr("Uninstallation failed");
    QString status = sucess ? successStr : failedStr;
    QString iconPath = sucess ? "success" : "fail";
    QIcon icon(QIcon::fromTheme(iconPath));
    QPixmap pic = icon.pixmap(80, 80);
    DriverIconWidget *widget = new DriverIconWidget(pic, status, "", this);
    mp_stackWidget->addWidget(widget);
    this->addButton(tr("OK", "button"), true);
    connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotClose);
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    if (sucess)
        DBusInterface::getInstance()->refreshInfo();
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
        if(path.isEmpty() || !file.exists()){
            mp_PathDialog->updateTipLabelText(tr("The selected folder does not exist, please select again"));
            return;
        }
        mp_NameDialog->loadAllDrivers(includeSubdir, path);
        mp_stackWidget->setCurrentIndex(1);
        this->setButtonText(1, tr("Update", "button"));
        this->setButtonText(0, tr("Previous"));
        this->getButton(0)->disconnect();
        connect(this->getButton(0), &QPushButton::clicked, this, &PageDriverControl::slotBackPathPage);
    } else if (1 == curIndex) {
        QString driveName = mp_NameDialog->selectName();
        QFile file(driveName);
        mp_NameDialog->updateTipLabelText("");
        if (driveName.isEmpty() || !file.exists()) {
            mp_NameDialog->updateTipLabelText(tr("The selected file does not exist, please select again"));
            return;
        }
        if (!DBusDriverInterface::getInstance()->isDriverPackage(driveName)) {
            mp_NameDialog->updateTipLabelText(tr("It is not a driver"));
            return;
        }
        removeBtn();
        mp_WaitDialog->setValue(0);
        mp_WaitDialog->setText(tr("Updating"));
        mp_stackWidget->setCurrentIndex(2);
        DBusDriverInterface::getInstance()->installDriver(driveName);
    }
}

void PageDriverControl::uninstallDriverLogical()
{
    // 点击卸载之后进入正在卸载界面
    removeBtn();
    mp_stackWidget->setCurrentIndex(mp_stackWidget->currentIndex() + 1);
    DBusDriverInterface::getInstance()->uninstallDriver(m_DriverName);
}

DWIDGET_END_NAMESPACE
