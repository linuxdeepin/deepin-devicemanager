// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "driveritem.h"
#include "commontools.h"
#include "BtnLabel.h"
#include "DDLog.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DToolTip>

#include <QHBoxLayout>

using namespace DDLog;

DriverCheckItem::DriverCheckItem(DWidget *parent, bool header)
    : DWidget(parent)
    , mp_cb(new DCheckBox(this))
{
    qCDebug(appLog) << "DriverCheckItem instance created, header:" << header;
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    if (header) {
        hLayout->setContentsMargins(9, 7, 0, 0);
    } else {
        hLayout->setContentsMargins(1, 0, 0, 0);
    }
    hLayout->addWidget(mp_cb);
    this->setLayout(hLayout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);
}

void DriverCheckItem::setChecked(bool checked, bool dis)
{
    qCDebug(appLog) << "Setting checkbox state:" << checked << "disconnect signals:" << dis;

    // 设置之前先断开
    if (dis)
        disconnect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);

    mp_cb->setChecked(checked);

    // 设置之后重新链接
    if (dis)
        connect(mp_cb, &DCheckBox::stateChanged, this, &DriverCheckItem::slotStateChanged);
}

bool DriverCheckItem::checked()
{
    // qCDebug(appLog) << "Getting checkbox state, checked:" << mp_cb->isChecked();
    return mp_cb->isChecked();
}

bool DriverCheckItem::isEnabled()
{
    // qCDebug(appLog) << "Getting checkbox state, enabled:" << mp_cb->isEnabled();
    return mp_cb->isEnabled();
}

void DriverCheckItem::setCbEnable(bool e)
{
    // qCDebug(appLog) << "Setting checkbox enabled state to:" << e;
    mp_cb->setEnabled(e);
}

void DriverCheckItem::paintEvent(QPaintEvent *event)
{
    // qCDebug(appLog) << "DriverCheckItem paint event called.";
    return DWidget::paintEvent(event);
}

void DriverCheckItem::slotStateChanged(int state)
{
    qCDebug(appLog) << "Checkbox state changed to:" << state;
    if (Qt::Unchecked == state) {
        qCDebug(appLog) << "Checkbox unchecked, emitting sigChecked(false)";
        emit sigChecked(false);
    } else {
        qCDebug(appLog) << "Checkbox checked, emitting sigChecked(true)";
        emit sigChecked(true);
    }
}

DriverNameItem::DriverNameItem(DWidget *parent, DriverType dt)
    : DWidget(parent)
    , mp_Icon(new DLabel(this))
    , mp_Type(new DLabel(this))
    , mp_Name(new TipsLabel(this))
    , m_Index(-1)
{
    qCDebug(appLog) << "DriverNameItem instance created, type:" << dt;
    mp_Type->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    mp_Type->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mp_Type->setElideMode(Qt::ElideRight);
    DFontSizeManager::instance()->bind(mp_Type, DFontSizeManager::T6);

    // 设置名称的相关参数：字体大小、颜色、对齐、布局
    mp_Name->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    mp_Name->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    mp_Name->setElideMode(Qt::ElideRight);

    mp_Icon->setPixmap(QIcon(CommonTools::getDriverPixmap(dt)).pixmap(ICON_SIZE_WIDTH, ICON_SIZE_HEIGHT));
    QString ts = DApplication::translate("QObject", CommonTools::getDriverType(dt).toStdString().data());
    mp_Type->setText(ts);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(10, 0, 0, 0);
    hLayout->addWidget(mp_Icon);
    hLayout->addSpacing(20);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addSpacing(3);
    vLayout->setSpacing(0);
    vLayout->addWidget(mp_Type);
    vLayout->addWidget(mp_Name);
    hLayout->addLayout(vLayout);
    hLayout->addSpacing(15);

    this->setLayout(hLayout);
}

void DriverNameItem::setName(const QString &name)
{
    qCDebug(appLog) << "Setting driver name:" << name;

    mp_Name->setText(name);
    mp_Name->setToolTip(name);
}

void DriverNameItem::setIndex(int index)
{
    // qCDebug(appLog) << "Setting driver index to:" << index;
    m_Index = index;
}

int DriverNameItem::index()
{
    // qCDebug(appLog) << "Getting driver index:" << m_Index;
    return m_Index;
}


DriverLabelItem::DriverLabelItem(DWidget *parent,  const QString &txt)
    : DWidget(parent)
    , mp_Txt(new TipsLabel(this))
{
    qCDebug(appLog) << "Creating DriverLabelItem with text:" << txt;
    mp_Txt->setText(txt);
    mp_Txt->setElideMode(Qt::ElideRight);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(1,0,0,0);
    vLayout->addStretch();
    vLayout->addWidget(mp_Txt);
    vLayout->addStretch();
    this->setLayout(vLayout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

DriverStatusItem::DriverStatusItem(DWidget *parent, Status s)
    : DWidget(parent)
    , mp_Spinner(new DSpinner(this))
    , mp_Icon(new DLabel(this))
    , mp_Status(new BtnLabel(this))
{
    qCDebug(appLog) << "DriverStatusItem instance created, initial status:" << s;
    mp_Spinner->setFixedSize(16, 16);
    mp_Spinner->start();
    mp_Status->setElideMode(Qt::ElideRight);

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    this->setContentsMargins(0, 0, 0, 0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(4);
    hLayout->addWidget(mp_Spinner);
    hLayout->addWidget(mp_Icon);
    hLayout->addWidget(mp_Status);
    hLayout->addStretch();
    this->setLayout(hLayout);

    // 初始化图标状态和状态信息
    setStatus(s);
}

void DriverStatusItem::setStatus(Status st)
{
    qCDebug(appLog) << "Setting driver status:" << st;

    showSpinner(ST_DOWNLOADING == st || ST_INSTALL == st || ST_DRIVER_BACKING_UP == st || ST_DRIVER_RESTORING == st);
    mp_Icon->setPixmap(QIcon(CommonTools::getStatusPixmap(st)).pixmap(16, 16));

    // bug132075 安装成功状态此button无法点击
    QString ts = DApplication::translate("QObject", CommonTools::getStausType(st).toStdString().data());

    if (ST_FAILED == st || ST_DRIVER_BACKUP_FAILED == st) {
        qCWarning(appLog) << "Driver status is failed, setting error message";
        QString statusStr = QString("<a style=\"text-decoration:none\" href=\"failed\">") + ts + "</a>";
        mp_Status->setText(statusStr);
    } else {
        mp_Status->setText(ts);
    }

    m_Status = st;
}

Status DriverStatusItem::getStatus()
{
    // qCDebug(appLog) << "Getting driver status:" << m_Status;
    return m_Status;
}

void DriverStatusItem::setErrorMsg(const QString &msg)
{
    // qCDebug(appLog) << "Setting error message:" << msg;
    mp_Status->setDesc(msg);
}

void DriverStatusItem::showSpinner(bool spin)
{
    // qCDebug(appLog) << "Showing spinner:" << spin;
    mp_Icon->setVisible(!spin);
    mp_Spinner->setVisible(spin);
    spin ? mp_Spinner->start() : mp_Spinner->stop();
    mp_Status->setVisible(true);
}

DriverOperationItem::DriverOperationItem(DWidget *parent, Mode mode)
    : DWidget(parent)
    , mp_Btn(new DToolButton(this))
{
    // qCDebug(appLog) << "Creating DriverOperationItem with mode:" << mode;
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addWidget(mp_Btn);
    mp_Btn->setFixedSize(36, 36);
    mp_Btn->setIconSize(QSize(36, 36));
    m_mode = mode;
    setBtnIcon();
    this->setLayout(hLayout);
    // 处理信号逻辑
    connect(mp_Btn, &DIconButton::clicked, this, &DriverOperationItem::clicked);
}

void DriverOperationItem::setBtnEnable(bool enable)
{
    qCDebug(appLog) << "Setting button enable:" << enable;

    mp_Btn->setEnabled(enable);
}

void DriverOperationItem::setBtnIcon()
{
    qCDebug(appLog) << "Setting button icon for mode:" << m_mode;
    switch (m_mode) {
    case Mode::INSTALL:
        qCDebug(appLog) << "Setting icon for install mode";
        mp_Btn->setIcon(QIcon::fromTheme("install"));
        mp_Btn->setToolTip(QObject::tr("Install"));
        break;
    case Mode::UPDATE:
        qCDebug(appLog) << "Setting icon for update mode";
        mp_Btn->setIcon(QIcon::fromTheme("update-btn"));
        mp_Btn->setToolTip(QObject::tr("Update"));
        break;
    case Mode::BACKUP:
        qCDebug(appLog) << "Setting icon for backup mode";
        mp_Btn->setIcon(QIcon(":/icons/deepin/builtin/light/backup.svg"));
        mp_Btn->setIconSize(QSize(16, 16));
        mp_Btn->setToolTip(QObject::tr("Backup"));
        break;
    case Mode::RESTORE:
        qCDebug(appLog) << "Setting icon for restore mode";
        mp_Btn->setIcon(QIcon(":/icons/deepin/builtin/light/restore.svg"));
        mp_Btn->setIconSize(QSize(16, 16));
        mp_Btn->setToolTip(QObject::tr("Restore"));
        break;
    }
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void DriverOperationItem::enterEvent(QEvent *event)
{
    // qCDebug(appLog) << "Mouse entered driver operation item";
    return DWidget::enterEvent(event);
}
#else
void DriverOperationItem::enterEvent(QEnterEvent *event)
{
    // qCDebug(appLog) << "Mouse entered driver operation item";
    return DWidget::enterEvent(event);
}
#endif

void DriverOperationItem::leaveEvent(QEvent *event)
{
    // qCDebug(appLog) << "Mouse left driver operation item";
    return DWidget::leaveEvent(event);
}
