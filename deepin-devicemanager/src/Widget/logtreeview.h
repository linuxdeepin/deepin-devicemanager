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

#ifndef LOGTREEVIEW_H
#define LOGTREEVIEW_H

#include <DTreeView>
#include <QKeyEvent>
#include <QStandardItem>
#include "logviewheaderview.h"
#include "logviewitemdelegate.h"

class LogTreeView : public Dtk::Widget::DTreeView
{
public:
    LogTreeView(QWidget *parent = nullptr);

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
    void setItem(int row, int column, QStandardItem *item);

    /**
     * @brief setColumnAverage : 设置表头等宽
     */
    void setColumnAverage();

    /**
     * @brief currentRowEnable : 获取当前行是否是选中状态
     * @return : 返回状态
     */
    bool currentRowEnable();

    /**
     * @brief currentRow
     * @return
     */
    int currentRow();

    /**
     * @brief clear : 清空数据
     */
    void clear();

protected:
    void initUI();
    void paintEvent(QPaintEvent *event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options,
                 const QModelIndex &index) const override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QStandardItemModel         *mp_Model;
    LogViewItemDelegate        *mp_ItemDelegate;
    LogViewHeaderView          *mp_HeaderView;

};

#endif  // LOGTREEVIEW_H
