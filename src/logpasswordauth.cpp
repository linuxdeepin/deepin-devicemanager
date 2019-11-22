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

#include "logpasswordauth.h"
#include <DApplication>
#include <QDebug>
#include <DPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "dialog_constants.h"
#include <QAction>
#include <QProcess>
#include "DMessageBox"

DWIDGET_USE_NAMESPACE

LogPasswordAuth::LogPasswordAuth(DWidget *parent)
    : DAbstractDialog(parent)
{
    setWindowFlag(Qt::WindowCloseButtonHint, false);
    this->setFixedSize(380, 140);

    initUI();
    initConnections();
}

LogPasswordAuth::~LogPasswordAuth()
{
    if(m_timer)
    {
        delete m_timer;
    }
}

void LogPasswordAuth::initUI()
{
    //setAutoFillBackground(true);
    // TopLayout
    topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(DIALOG::ICON_LAYOUT_LEFT_MARGIN,
                                  DIALOG::ICON_LAYOUT_TOP_MARGIN,
                                  DIALOG::ICON_LAYOUT_RIGHT_MARGIN,
                                  DIALOG::ICON_LAYOUT_BOTTOM_MARGIN);
    //topLayout->setSpacing(DIALOG::ICON_LAYOUT_SPACING);

    // TopLayout--Icon
    iconLabel = new DLabel;
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QPixmap pix(":images/security.ico");
    iconLabel->setPixmap(pix);
    topLayout->addWidget(iconLabel);

    titleLabel = new DLabel(DApplication::translate("Main", "读取设备信息需要授权"));
    titleLabel->setObjectName("TitleLabel");
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    QFont font = titleLabel->font();
    font.setPixelSize(20);
    titleLabel->setFont(font);

    messageLabel = new DLabel;
    messageLabel->hide();
    messageLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    font = messageLabel->font();
    font.setPixelSize(20);
    messageLabel->setFont(font);

    QVBoxLayout *textLayout = new QVBoxLayout;
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(5);
    textLayout->addStretch();
    textLayout->addWidget(titleLabel, 0, Qt::AlignLeft);
    textLayout->addWidget(messageLabel, 0, Qt::AlignLeft);
    textLayout->addStretch();

    // TopLayout--ContentLayout
    contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addLayout(textLayout);

    m_edt = new DPasswordEdit(this);
    //m_edt->setAutoFillBackground(true);
    //m_edt->setFixedHeight(28);
    //contentLayout->setSpacing(10);
    contentLayout->addWidget(m_edt);

    connect( m_edt, &DPasswordEdit::textEdited, this, &LogPasswordAuth::onTimer );
    connect( m_edt, &DPasswordEdit::focusChanged, this, &LogPasswordAuth::onTimer );
    topLayout->addLayout(contentLayout);


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // MainLayout--TopLayout
    mainLayout->addLayout(topLayout);

    buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(DIALOG::BUTTON_LAYOUT_LEFT_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_TOP_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_RIGHT_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_BOTTOM_MARGIN);

    cancelButton_ = new DPushButton(DApplication::translate("Main", "Cancel"), this);
    cancelButton_->setAttribute(Qt::WA_NoMousePropagation);
    cancelButton_->setFixedHeight(40);
    cancelButton_->setObjectName("ActionButton");
    font = cancelButton_->font();
    font.setPixelSize(20);
    cancelButton_->setFont(font);

    connect(cancelButton_, &DPushButton::clicked, this, &LogPasswordAuth::cancelButtonClicked);

    confimButton_ = new DPushButton(DApplication::translate("Main", "Authenticate"), this);
    confimButton_->setAttribute(Qt::WA_NoMousePropagation);
    confimButton_->setFixedHeight(40);
    font = confimButton_->font();
    font.setPixelSize(20);
    confimButton_->setFont(font);
    connect(confimButton_, &DPushButton::clicked, this, &LogPasswordAuth::confimButtonClicked);

    QAction *button_action = new QAction(this);

    button_action->setShortcuts(QKeySequence::InsertParagraphSeparator);
    button_action->setAutoRepeat(false);
    QObject::connect(button_action, &QAction::triggered, this, &LogPasswordAuth::defaultButtonTriggered );

    buttonLayout->addWidget(cancelButton_);
    buttonLayout->addWidget(confimButton_);

    mainLayout->addLayout(buttonLayout);

    this->setLayout(mainLayout);
    this->addAction(button_action);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setFocus();

    //m_edt->adjustSize();
    m_edt->setEchoButtonIsVisible(true);
}

void LogPasswordAuth::initConnections()
{
    //connect(this, &DDialog::buttonClicked, this, &LogPasswordAuth::slotButtonClicked);
}

QString LogPasswordAuth::getPasswd()
{
    return m_edt->text();
}

void LogPasswordAuth::clearPasswd()
{
    m_edt->clear();
}

void LogPasswordAuth::hideMessage()
{
    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::black);
    titleLabel->setPalette(pa);
    titleLabel->setText(DApplication::translate("Main", "读取设备信息需要授权"));
}

void LogPasswordAuth::showMessage(const QString& msg)
{
//    if(m_timer == nullptr)
//    {
//        m_timer = new QTimer;
//        connect(m_timer, &QTimer::timeout, this, &LogPasswordAuth::onTimer);
//    }

//    m_timer->stop();
//    m_timer->start(3000);

    QPalette pa;
    pa.setColor(QPalette::WindowText,Qt::red);
    titleLabel->setPalette(pa);
    titleLabel->setText(msg);
}

void LogPasswordAuth::cancelButtonClicked()
{
    done(-1);
    close();
}

void LogPasswordAuth::confimButtonClicked()
{
    if(m_edt->text().isEmpty())
    {
        showMessage(  DApplication::translate("Main", "Please Enter Password!") );
        //DMessageBox::warning(this, DApplication::translate("Main", "Error"), DApplication::translate("Main", "Please Enter Password!"));
        return;
    }

    QStringList arg;
    arg << "-c" << "echo " + getPasswd() + " | sudo -S id -un";
    QProcess process;
    process.start("/bin/bash",arg);
    bool res = process.waitForFinished();

    if( res == false || process.readAllStandardOutput().trimmed() != "root" )
    {
        clearPasswd();
        showMessage(  DApplication::translate("Main", "Password Error!") );
        //QMessageBox::warning(this, DApplication::translate("Main", "Error"), DApplication::translate("Main", "Password Error!"));
        return;
    }

    done(0);
    close();
}

void LogPasswordAuth::defaultButtonTriggered()
{
    QAbstractButton *button = qobject_cast<QAbstractButton*>(focusWidget());

    if (button)
        button->click();
    else if (confimButton_)
        confimButton_->click();
}

void LogPasswordAuth::onTimer()
{
    if(m_timer)
        delete m_timer;

    m_timer = nullptr;

    hideMessage();
}
