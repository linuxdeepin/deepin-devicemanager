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

#ifndef GETDRIVERNAMEDIALOG_H
#define GETDRIVERNAMEDIALOG_H

#include "DriverListView.h"

#include <DWidget>

#include <QStandardItemModel>

DWIDGET_BEGIN_NAMESPACE
class DLabel;
DWIDGET_END_NAMESPACE
class GetDriverNameModel;
class WaitingWidget;

DWIDGET_USE_NAMESPACE

class GetDriverNameWidget : public DWidget
{
    Q_OBJECT
public:
    explicit GetDriverNameWidget(QWidget *parent = nullptr);
    ~GetDriverNameWidget();

    /**
     * @brief loadAllDrivers 通过给的路径，查找路径下的所有驱动文件
     * @param includeSub 是否查找目录下的子目录
     * @param path 给定的目录
     */
    void loadAllDrivers(bool includeSub, const QString &path);

    /**
     * @brief selectName 用户选中的驱动名称
     * @return
     */
    QString selectName();

    /**
     * @brief updateTipLabelText 更新tipLabel
     * @param text
     */
    void updateTipLabelText(const QString &text);

    /**
     * @brief stopLoadingDrivers
     */
    void stopLoadingDrivers();

private:
    /**
     * @brief init 初始化界面
     */
    void init();

    /**
     * @brief initConnections 绑定信号槽
     */
    void initConnections();

    /**
     * @brief reloadDriversListPages 根据有无驱动加载界面
     */
    void reloadDriversListPages();

public slots:

    /**
     * @brief slotSelectedDriver 选中驱动属性设置
     * @param row 当前选中行
     */
    void slotSelectedDriver(const QModelIndex &index);

    /**
     * @brief slotFinishLoadDrivers
     */
    void slotFinishLoadDrivers();

signals:

    /**
     * @brief signalItemClicked
     */
    void signalItemClicked();

    /**
     * @brief signalDriversCount
     */
    void signalDriversCount();

    /**
     * @brief startLoadDrivers
     * @param model
     * @param includeSub
     * @param path
     */
    void startLoadDrivers(QStandardItemModel* model, bool includeSub, const QString &path);

private:
    DStackedWidget             *mp_StackWidget = nullptr;
    WaitingWidget              *mp_WaitingWidget = nullptr;
    DriverListView             *mp_ListView = nullptr;
    Dtk::Widget::DLabel        *mp_tipLabel;
    QStandardItemModel         *mp_model;
    GetDriverNameModel         *mp_GetModel;
    QThread                    *mp_Thread = nullptr;
    int                        mp_selectedRow = -1;  //当前选中行
};

#endif // GETDRIVERNAMEDIALOG_H
