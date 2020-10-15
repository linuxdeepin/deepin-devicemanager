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

#ifndef PAGEINFO_H
#define PAGEINFO_H

#include <QObject>
#include <DWidget>

class DeviceBaseInfo;

using namespace Dtk::Widget;

/**
 * @brief The PageInfo class
 * UI 设备信息展示的基类
 */
class PageInfo : public DWidget
{
    Q_OBJECT
public:
    explicit PageInfo(QWidget *parent = nullptr);

    /**
     * @brief updateTable : 更新表格
     * @param lst : 某类型设备列表
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst) = 0;

    /**
     * @brief updateInfo:刷新概况信息
     * @param map : 概况信息
     */
    virtual void updateInfo(const QMap<QString, QString> &map);

    /**
     * @brief setLabel:设置Label
     * @param itemstr:Label信息
     */
    virtual void setLabel(const QString &itemstr) = 0;

    /**
     * @brief setLabel:设置图片Label，设备Label，操作系统Label
     * @param str1:设备名称
     * @param str2:操作系统信息
     */
    virtual void setLabel(const QString &str1, const QString &str2);

    /**
     * @brief clearContent:清除内容
     */
    virtual void clearContent();

    /**
     * @brief isOverview:是否是概况界面
     * @return false : 不是概况界面
     */
    virtual bool isOverview() {return false;}

    /**
     * @brief isBaseBoard : 是否是主板信息界面
     * @return false : 不是主板信息界面
     */
    virtual bool isBaseBoard() {return false;}

    virtual void setDeviceInfoNum(int num);

    virtual int getDeviceInfoNum();
protected:
    void paintEvent(QPaintEvent *e) override;

private:

    int m_AllInfoNum;

};

#endif // PAGEINFO_H
