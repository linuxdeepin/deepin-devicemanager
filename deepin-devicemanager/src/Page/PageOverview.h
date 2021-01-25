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

#ifndef PAGEOVERVIEW_H
#define PAGEOVERVIEW_H

#include <QObject>
#include <DWidget>
#include <DLabel>

#include "PageInfo.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class PageTableWidget;
class LongTextLabel;

/**
 * @brief The PageOverview class
 * UI 概况界面的展示类
 */
class PageOverview : public PageInfo
{
    Q_OBJECT
public:
    explicit PageOverview(DWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新表格
     * @param lst : 某类型设备列表
     */
    void updateInfo(const QList<DeviceBaseInfo *> &) override;

    /**
     * @brief updateInfo:刷新概况信息
     * @param map : 概况信息
     */
    void updateInfo(const QMap<QString, QString> &map) override;

    /**
     * @brief setLabel:设置Label
     * @param itemstr:Label信息
     */
    void setLabel(const QString &itemstr) override;

    /**
     * @brief setLabel:设置图片Label，设备Label，操作系统Label
     * @param str1:设备名称
     * @param str2:操作系统信息
     */
    void setLabel(const QString &str1, const QString &str2) override;

    /**
     * @brief isOverview:是否是概况界面
     * @return true: 是概况界面
     */
    bool isOverview() override {return  true;}

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

private slots:
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
     * @brief slotActionCopy
     */
    void slotActionCopy();

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DLabel                  *mp_PicLabel;
    LongTextLabel           *mp_DeviceLabel;
    LongTextLabel           *mp_OSLabel;
    PageTableWidget         *mp_Overview;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QAction                 *mp_Copy;        //<! 拷贝
    QMenu                   *mp_Menu;        //<! 右键菜单
};

#endif // PAGEOVERVIEW_H
