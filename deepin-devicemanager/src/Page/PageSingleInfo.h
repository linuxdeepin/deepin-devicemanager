/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

using namespace Dtk::Widget;

class PageTableWidget;

/**
 * @brief The PageSingleInfo class
 * UI 单个设备的界面展示类
 */
class PageSingleInfo : public PageInfo
{
    Q_OBJECT
public:
    explicit PageSingleInfo(QWidget *parent = nullptr);
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

    /**
     * @brief slotActionCopy:右键菜单拷贝信息
     */
    void slotActionCopy();

    /**
     * @brief slotActionEnable:右键菜单禁用启用
     */
    void slotActionEnable();

protected:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

    /**
     * @brief expandTable 手动张开表格
     */
    void expandTable();

protected:
    PageTableWidget         *mp_Content;
    DLabel                  *mp_Label;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QAction                 *mp_Copy;        //<! 拷贝
    QAction                 *mp_Enable;      //<! 启用/禁用
    QMenu                   *mp_Menu;        //<! 右键菜单

    DeviceBaseInfo          *mp_Device;      //<! 设备
    bool                    m_SameDevice;


};

#endif // PAGESINGLEINFO_H
