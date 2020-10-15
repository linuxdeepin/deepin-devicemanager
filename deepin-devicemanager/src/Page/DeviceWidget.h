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

/**
 * @brief The DeviceWidget class
 * 界面类，设备信息的主界面类
 */
class DeviceWidget : public DWidget
{
    Q_OBJECT
public:
    explicit DeviceWidget(QWidget *parent = nullptr);
    ~DeviceWidget();

    /**
     * @brief updateListView:更新ListView
     * @param lst:设备类别
     */
    void updateListView(const QList<QPair<QString, QString> > &lst);

    /**
     * @brief updateDevice:更新设备
     * @param itemStr:设备类别
     * @param lst:该设备类别的所有设备指针lst
     */
    void updateDevice(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateOverview:更新概况界面
     * @param itemStr:概况
     * @param map:概况信息map
     */
    void updateOverview(const QString &itemStr, const QMap<QString, QString> &map);

    /**
     * @brief currentIndex:当前设备类型
     * @return 设备类型
     */
    QString currentIndex() const;
signals:

    /**
     * @brief itemClicked:item点击信号
     * @param itemStr:item内容
     */
    void itemClicked(const QString &itemStr);

    /**
     * @brief refreshInfo:刷新信息
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息
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
     * @brief slotUpdateUI:更新UI界面
     */
    void slotUpdateUI();

protected:
    /**
     * @brief: 事件的重写
     */
    virtual void resizeEvent(QResizeEvent *event)override;

private:
    /**@brief:初始化界面布局*/
    void initWidgets();

private:
    PageListView              *mp_ListView;          //<! 左边的list
    PageInfoWidget            *mp_PageInfo;          //<! 右边的详细内容
    QString                   m_CurItemStr;          //<! 当前Item内容
};

#endif // DETAILWIDGET_H
