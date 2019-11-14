/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
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

#pragma once

#include <QObject>
#include <QString>
#include "DListView"
#include <DStyledItemDelegate>
#include "DWidget"

class QListViewItem;
class QAction;

class DeviceListviewDelegate : public Dtk::Widget::DStyledItemDelegate
{
    Q_OBJECT
public:
    DeviceListviewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
};

class DeviceListView : public Dtk::Widget::DListView
{
    Q_OBJECT
public:
    DeviceListView(Dtk::Widget::DWidget* parent = nullptr);
    void addDevice(const QString& deviceName, const QString& iconFile);
    void addSeperator();

    QString currentDevice();
    void setCurrentDevice(const QString& device);

    void setFistSelected();

    QString indexString(int index);

public slots:
    bool onExportToFile();
    void OnlvOpRightBtn( const QPoint&);
    void changeThemeIcon();

private:
    void initContextMenu();
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

private:
    QStandardItemModel* navModel_ = nullptr;
    QString currentDevice_;
    Dtk::Widget::DMenu* contextMenu_ = nullptr;
    QAction* refreshAction_ = nullptr;
};
