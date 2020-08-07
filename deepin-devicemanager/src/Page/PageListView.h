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

#ifndef LISTVIEWWIDGET_H
#define LISTVIEWWIDGET_H

#include <DWidget>
#include <QAction>
#include <QMenu>
#include <QList>

class DeviceListView;

using namespace Dtk::Widget;
class PageListView : public DWidget
{
    Q_OBJECT
public:
    PageListView(DWidget *parent = nullptr);
    ~PageListView() override;

    void updateListItems(const QList<QPair<QString, QString> > &lst);

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

signals:
    /**
     * @brief itemClicked:点击item发出信号
     * @param txt:item显示的字符串
     */
    void itemClicked(const QString &txt);

private slots:
    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     * @param point:鼠标位置
     */
    void slotShowMenu(const QPoint &point);

    /**
     * @brief slotActionEnable:右键启用按钮槽函数
     */
    void slotActionEnable();

    /**
     * @brief slotActionDisable:右键禁用按钮槽函数
     */
    void slotActionDisable();

    /**
     * @brief slotListViewItemClicked:ListView Item 点击槽函数
     * @param index:点击Item的索引
     */
    void slotListViewItemClicked(const QModelIndex &index);

private:
    DeviceListView            *mp_ListView;
    QAction                   *mp_Enable;
    QAction                   *mp_Disable;
    QMenu                     *mp_Menu;
    QString                   m_CurType;        // 当前显示的设备类型
};

#endif // LISTVIEWWIDGET_H
