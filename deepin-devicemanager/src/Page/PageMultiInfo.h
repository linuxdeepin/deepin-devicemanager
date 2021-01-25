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

#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QObject>
#include <DWidget>
#include <DLabel>

#include "PageInfo.h"


using namespace Dtk::Widget;

class PageTableHeader;
class PageDetail;
class DeviceBaseInfo;

/**
 * @brief The PageMultiInfo class
 * 多个设备信息的展示类
 */
class PageMultiInfo : public PageInfo
{
    Q_OBJECT
public:
    explicit PageMultiInfo(QWidget *parent = nullptr);
    ~PageMultiInfo() override;

    /**
     * @brief updateInfo:更新信息
     * @param lst:某类设备信息列表
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief setLabel:设置类型Label
     * @param itemstr:类型字符串
     */
    void setLabel(const QString &itemstr) override;

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

    /**
     * @brief enableDevice
     * @param row
     * @param enable
     */
    void enableDevice(int row, bool enable);

    /**
     * @brief updateUI : update UI
     */
    void updateUI();

private slots:
    /**
     * @brief slotItemClicked:点击表格Item
     * @param row:点击行
     */
    void slotItemClicked(int row);

    /**
     * @brief slotRefreshInfo:刷新槽函数
     */
    void slotRefreshInfo();

    /**
     * @brief slotExportInfo:单导出槽函数
     */
    void slotExportInfo();

    /**
     * @brief slotEnableDevice
     * @param row
     * @param enable
     */
    void slotEnableDevice(int row, bool enable);

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

private:
    DLabel                    *mp_Label;
    PageTableHeader           *mp_Table;       //<! 上面的表格
    PageDetail                *mp_Detail;      //<! 下面的详细内容
};

#endif // DEVICEPAGE_H
