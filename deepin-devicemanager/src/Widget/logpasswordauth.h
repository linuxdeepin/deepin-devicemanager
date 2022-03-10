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

/*  Deprecated 20191030
 *
#pragma once

#include <dabstractdialog.h>
#include <DDialog>
#include <DLabel>
#include <DLineEdit>
#include <DPushButton>
#include <DWidget>
#include <QTimer>
#include "DIconButton"
#include "DPasswordEdit"


class LogPasswordAuth : public Dtk::Widget::DAbstractDialog
{
public:
    LogPasswordAuth(Dtk::Widget::DWidget *parent = nullptr);
    ~LogPasswordAuth();
    void initUI();
    void initConnections();

    QString getPasswd();
    void clearPasswd();

    void hideMessage();
    void showMessage(const QString& msg);

public slots:
    void cancelButtonClicked();
    void confimButtonClicked();
    void defaultButtonTriggered();
    void onTimer();

private:
    //void setPasswd(QString str);
    //QString m_passwd;

    QHBoxLayout *iconLayout;
    QVBoxLayout *contentLayout;
    QHBoxLayout *buttonLayout;
    QHBoxLayout *topLayout;

    Dtk::Widget::DLabel* iconLabel;
    Dtk::Widget::DLabel* messageLabel;
    Dtk::Widget::DLabel* titleLabel;

    Dtk::Widget::DPushButton* cancelButton_;
    Dtk::Widget::DPushButton* confimButton_;

    Dtk::Widget::DPasswordEdit *m_edt;

    QTimer* m_timer = nullptr;
};

*/
