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

#ifndef DEVICELISTVIEW_H
#define DEVICELISTVIEW_H

#include <QObject>
#include <QWidget>

#include <DListView>
#include <QStandardItemModel>
#include <DStandardItem>

using namespace Dtk::Widget;


/**
 * @brief The DeviceListviewDelegate class
 * listView代理类
 */
class DeviceListviewDelegate : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DeviceListviewDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void paintSeparator(QPainter *painter, const QStyleOptionViewItem &option) const;
};


/**
 * @brief The DeviceListView class
 * listView的控件
 */
class DeviceListView : public DListView
{
    Q_OBJECT
public:
    explicit DeviceListView(QWidget *parent = nullptr);
    ~DeviceListView()override;

    /**
     * @brief:添加item
     * @param[in] name : item的文本
     * @param[in] icon : item的图标文件
     */
    void addItem(const QString &name, const QString &iconFile);

    /**
     * @brief:判断当前是否启用
     * @return : 返回是否启用
     */
    bool curItemEnable();

    /**
     * @brief:设置当前是否启用
     * @param[enable] : 是否启用
     */
    void setCurItemEnable(bool enable);

    /**
     * @brief setCurItem : 根据str设置当前的item
     * @param str
     */
    void setCurItem(const QString &str);

    /**
     * @brief getConcatenateStrings
     * @return
     */
    QString getConcatenateStrings(const QModelIndex &index);

    /**
     * @brief clearItem : 清楚列表
     */
    void clearItem();

protected:
    /**@brief:事件重写*/
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    /**
     * @brief keyPressEvent:相应键盘按键事件
     * @param keyEvent:按键：上，下键
     */
    void keyPressEvent(QKeyEvent *keyEvent) override;

private:
    QStandardItemModel        *mp_ItemModel;                   // Model View 模式里面的 Model
};

#endif // DEVICELISTVIEW_H
