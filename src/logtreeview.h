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

private:
    void initUI();
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    int sizeHintForColumn(int column) const override;

private:
    //QStyleOptionViewItem viewOptions() const override;
    //void initStyleOption(QStyleOptionFrame *option) const;

public:
    LogViewItemDelegate *m_itemDelegate;
    LogViewHeaderView *m_headerDelegate;

public:
    QStandardItemModel *m_pModel;
};

#endif  // LOGTREEVIEW_H
