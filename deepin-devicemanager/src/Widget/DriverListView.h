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
#include <DRadioButton>

#include <QMouseEvent>

DWIDGET_BEGIN_NAMESPACE

class DriverListView : public DTreeView
{
    Q_OBJECT
public:
    explicit DriverListView(QWidget *parent = nullptr);
    void initUI();

signals:

public slots:

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event);
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const;
    void mouseReleaseEvent(QMouseEvent *event);
};
DWIDGET_END_NAMESPACE
#endif // DRIVERLISTVIEW_H
