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

#ifndef DETAILWIDGET_H
#define DETAILWIDGET_H

#include <QObject>
#include <DWidget>
#include <DSplitter>

class PageListView;
class PageInfoWidget;
class DeviceBaseInfo;

using namespace Dtk::Widget;

class DeviceWidget : public DWidget
{
    Q_OBJECT
public:
    DeviceWidget(QWidget *parent = nullptr);
    ~DeviceWidget();

    /**
     * @brief updateListView:
     * @param lst
     */
    void updateListView(const QList<QPair<QString, QString> > &lst);

    /**
     * @brief updateDevice
     * @param itemStr
     * @param lst
     */
    void updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateOverview
     * @param itemStr
     * @param map
     */
    void updateOverview(const QString &itemStr, const QMap<QString, QString> &map);
signals:

    /**
     * @brief itemClicked
     * @param itemStr
     */
    void itemClicked(const QString &itemStr);

    /**
     * @brief refreshInfo
     */
    void refreshInfo();

    /**
     * @brief exportInfo
     */
    void exportInfo();

private slots:

    /**
     * @brief slotListViewWidgetItemClicked:ListView item点击槽函数
     * @param itemStr:item显示字符串
     */
    void slotListViewWidgetItemClicked(const QString &itemStr);

    /**
     * @brief slotRefreshInfo:刷新信息槽函数
     */
    void slotRefreshInfo();

    /**
     * @brief slotExportInfo:导出信息槽函数
     */
    void slotExportInfo();

    /**
     * @brief slotUpdateUI
     */
    void slotUpdateUI();

private:
    /**@brief:初始化界面布局*/
    void initWidgets();

private:
    PageListView              *mp_ListView;          //<! 左边的list
    PageInfoWidget            *mp_PageInfo;          //<! 右边的详细内容
    QString                   m_CurItemStr;
};

#endif // DETAILWIDGET_H
