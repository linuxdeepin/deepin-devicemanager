/*
* Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
*
* Author:      zhangkai <zhangkai@uniontech.com>
* Maintainer:  zhangkai <zhangkai@uniontech.com>
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DRIVERLISTVIEW_H
#define DRIVERLISTVIEW_H

#include <DTreeView>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

class DriverListView : public DTreeView
{
    Q_OBJECT
public:
    explicit DriverListView(QWidget *parent = nullptr);
    void initUI();
    /**
     * @brief keyPressEvent 重写按钮事件
     */
    void keyPressEvent(QKeyEvent *event) override;
public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
};

#endif // DRIVERLISTVIEW_H
