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

#ifndef LISTVIEWWIDGET_H
#define LISTVIEWWIDGET_H

#include <DWidget>
#include <QAction>
#include <QMenu>
#include <QList>

class DeviceListView;

using namespace Dtk::Widget;

/**
 * @brief The PageListView class
 * 设备列表的展示类
 */

class PageListView : public DWidget
{
    Q_OBJECT
public:
    explicit PageListView(DWidget *parent = nullptr);
    ~PageListView() override;

    void updateListItems(const QList<QPair<QString, QString> > &lst);

    /**
     * @brief currentIndex 当前Item类别
     * @return  类别
     */
    QString currentIndex();

    /**
     * @brief currentType:当前展示设备
     * @return 设备
     */
    QString currentType();

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

signals:
    /**
     * @brief itemClicked:点击item发出信号
     * @param txt:item显示的字符串
     */
    void itemClicked(const QString &txt);

    void refreshActionTrigger();

    void exportActionTrigger();

private slots:
    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     * @param point:鼠标位置
     */
    void slotShowMenu(const QPoint &point);

    /**
     * @brief slotActionEnable:右键启用按钮槽函数
     */
    void slotActionRefresh();

    /**
     * @brief slotActionDisable:右键禁用按钮槽函数
     */
    void slotActionExport();

    /**
     * @brief slotListViewItemClicked:ListView Item 点击槽函数
     * @param index:点击Item的索引
     */
    void slotListViewItemClicked(const QModelIndex &index);

private:
    DeviceListView            *mp_ListView;
    QAction                   *mp_Refresh;
    QAction                   *mp_Export;
    QMenu                     *mp_Menu;
    QString                   m_CurType;        // 当前显示的设备类型
};

#endif // LISTVIEWWIDGET_H
