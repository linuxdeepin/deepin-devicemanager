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

#ifndef PAGEINFOWIDGET_H
#define PAGEINFOWIDGET_H

#include <QObject>
#include <DWidget>
//#include <QHBoxLayout>

class DeviceBaseInfo;
class PageInfo;
class PageMultiInfo;
class PageSingleInfo;
class PageOverview;
class PageBoardInfo;

using namespace Dtk::Widget;

/**
 * @brief The PageInfoWidget class
 * UI 设备信息的展示类
 */

class PageInfoWidget : public DWidget
{
    Q_OBJECT
public:
    explicit PageInfoWidget(QWidget *parent = nullptr);

    /**
     * @brief updateTable
     * @param lst ：某类设备信息列表
     */

    /**
     * @brief updateTable: 更新表格
     * @param itemStr:设备类型
     * @param lst:设备类型对应的所有设备列表
     */
    void updateTable(const QString &itemStr, const QList<DeviceBaseInfo *> &lst);

    /**
     * @brief updateTable: 更新表格
     * @param itemStr:概况
     * @param map:概况信息map
     */
    void updateTable(const QString &itemStr, const QMap<QString, QString> &map);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();
    void updateUI();

private slots:

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
    /**
     * @brief initWidgets
     */
    void initWidgets();

private:
    PageInfo       *mp_PageInfo;
    PageSingleInfo *mp_PageSignalInfo;
    PageMultiInfo  *mp_PageMutilInfo;
    PageOverview   *mp_PageOverviewInfo;
    PageBoardInfo  *mp_PageBoardInfo;
};

#endif // PAGEINFOWIDGET_H
