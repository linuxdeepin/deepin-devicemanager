/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     LZ <zhou.lu@archermind.com>
 *
 * Maintainer: LZ <zhou.lu@archermind.com>
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

#ifndef PAGESINGLEINFO_H
#define PAGESINGLEINFO_H

#include <QObject>
#include <QWidget>

#include <DLabel>

#include "PageInfo.h"
#include "DetailTreeView.h"

DWIDGET_USE_NAMESPACE
using namespace Dtk::Widget;

class PageSingleInfo : public PageInfo
{
    Q_OBJECT
public:
    PageSingleInfo(QWidget *parent = nullptr);
    ~PageSingleInfo() override;

    /**
     * @brief updateInfo:更新信息
     * @param lst:某类设备信息列表
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief setLabel:设置类型Label
     * @param itemstr:类型字符串
     */
    void setLabel(const QString &itemstr) override;

    /**
     * @brief loadDeviceInfo:加载设备信息
     * @param lst:某设备设备的信息列表
     */
    virtual void loadDeviceInfo(const QList<QPair<QString, QString>> &lst);

    /**
     * @brief clearContent:清除表格内容
     */
    void clearContent() override;

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

protected slots:
    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     */
    void slotShowMenu(const QPoint &);

    /**
     * @brief slotActionRefresh:右键菜单刷新槽函数
     */
    void slotActionRefresh();

    /**
     * @brief slotActionExport:右键菜单导出槽函数
     */
    void slotActionExport();

protected:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

protected:
    DetailTreeView          *mp_Content;
    DLabel                  *mp_Label;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // PAGESINGLEINFO_H
