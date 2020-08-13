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

#ifndef DEVICEDETAILPAGE_H
#define DEVICEDETAILPAGE_H

#include <QObject>
#include <DWidget>
#include <QScrollArea>
#include <QHBoxLayout>
#include <DCommandLinkButton>
#include <DHorizontalLine>

class TextBrowser;
class DeviceBaseInfo;
class QVBoxLayout;

using namespace Dtk::Widget;

class DetailButton: public DCommandLinkButton
{
    Q_OBJECT
public:
    explicit DetailButton(const QString &txt);
protected:
    void paintEvent(QPaintEvent *e) override;
};

class DetailSeperator: public DWidget
{
    Q_OBJECT
public:
    explicit DetailSeperator(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;
};

class ScrollAreaWidget: public DWidget
{
    Q_OBJECT
public:
    explicit ScrollAreaWidget(DWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *e) override;

};


class PageDetail : public DWidget
{
    Q_OBJECT
public:
    explicit PageDetail(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param lstInfo :需要显示的设备
     */
    void showDeviceInfo(const QList<DeviceBaseInfo *> &lstInfo);

    /**
     * @brief showInfoOfNum : 点击表头时，详细信息显示到相应的位置
     * @param index : 需要显示到相应的位置
     */
    void showInfoOfNum(int index);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief addWidgets 添加widget到布局
     * @param widget ：需要添加的widget
     */
    void addWidgets(TextBrowser *widget, bool enable);

    /**
     * @brief clearWidget : 清楚widget
     */
    void clearWidget();

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

private slots:

    /**
     * @brief slotBtnClicked:鼠标右键菜单槽函数
     */
    void slotBtnClicked();

    /**
     * @brief slotRefreshInfo:右键菜单刷新槽函数
     */
    void slotRefreshInfo();

    /**
     * @brief slotExportInfo:右键菜单导出槽函数
     */
    void slotExportInfo();
    void slotCopyAllInfo();



private:
    QVBoxLayout      *mp_ScrollAreaLayout;
    QScrollArea      *mp_ScrollArea;

    QList<TextBrowser *>           m_ListTextBrowser;
    QList<QHBoxLayout *>           m_ListHlayout;
    QList<DetailButton *>          m_ListDetailButton;
    QList<DetailSeperator *>       m_ListDetailSeperator;
};

#endif // DEVICEDETAILPAGE_H
