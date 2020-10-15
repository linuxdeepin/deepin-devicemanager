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

#ifndef HEADERTABLEVIEW_H
#define HEADERTABLEVIEW_H

#include <QObject>
#include <DTableView>
#include <QStandardItemModel>
#include <DStandardItem>
#include <DHeaderView>

class LogTreeView;

using namespace Dtk::Widget;

/**
 * @brief The TableWidget class
 * UI : 自定义tablewidget
 */
class TableWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TableWidget(QWidget *parent = nullptr);

    /**
     * @brief setHeaderLabels : 设置table的表头
     * @param lst ：表头的内容
     */
    void setHeaderLabels(const QStringList &lst);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, DStandardItem *item);

    /**
     * @brief setColumnAverage
     */
    void setColumnAverage();

    /**
     * @brief updateCurItemEnable
     * @param row
     * @param enable
     */
    void updateCurItemEnable(int row, bool enable);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setRowNum: 设置表格行数
     * @param row : 总行数
     */
    void setRowNum(int row);

signals:
    void itemClicked(int row);
    void refreshInfo();
    void exportInfo();
    void enableDevice(int row, bool enable);
protected:
    void paintEvent(QPaintEvent *e) override;

private slots:

    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     */
    void slotShowMenu(const QPoint &);

    /**
     * @brief slotActionRefresh:右键刷新按钮槽函数
     */
    void slotActionRefresh();

    /**
     * @brief slotActionExport:右键导出按钮槽函数
     */
    void slotActionExport();

    /**
     * @brief slotActionEnable:判断是否启用
     */
    void slotActionEnable();

    /**
     * @brief slotItemClicked:ListView Item 点击槽函数
     * @param index:点击Item的索引
     */
    void slotItemClicked(const QModelIndex &index);

private:
    void initWidget();

private:
    LogTreeView      *mp_Table;
    QAction          *mp_Enable;      //<! 启用禁用
    QAction          *mp_Refresh;     //<! 右键刷新
    QAction          *mp_Export;      //<! 右键导出
    QMenu            *mp_Menu;        //<! 右键菜单
    bool                       m_Enable;
};






#endif // HEADERTABLEVIEW_H
