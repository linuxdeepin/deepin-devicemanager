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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>

#include <DMainWindow>
#include <DStackedWidget>

#include "DBusInterface.h"

class WaitingWidget;
class DeviceWidget;
class LoadInfoThread;

using namespace Dtk::Widget;

/**
 * @brief The MainWindow class
 * 界面，主窗口
 */
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    /**
     * @brief refresh:界面刷新
     */
    void refresh();

    /**
     * @brief exportTo:导出设备信息
     * @return true:导出成功，false:导出失败
     */
    bool exportTo();

    /**
     * @brief showDisplayShortcutsHelpDialog:显示快捷键帮主窗口
     */
    void showDisplayShortcutsHelpDialog();

    /**
     * @brief addJsonArrayItem:添加json键值对
     * @param windowJsonItems:保存json键值对数组
     * @param name:json名称
     * @param value:json值
     */
    void addJsonArrayItem(QJsonArray &windowJsonItems, const QString &name, const QString &value);

    /**
     * @brief getJsonDoc:获取json文本信息
     * @return json文本信息
     */
    void getJsonDoc(QJsonDocument &doc);

    /**
     * @brief windowMaximizing:窗口最大化
     */
    void windowMaximizing();

    /**@brief:获取计算机架构字符串*/
    QString getArchString();

    /**
     * @brief loadGeneratorKey:获取计算机dbus信息，区分机器类型
     * @return panguV，klu
     */
    QString loadGeneratorKey();

protected:
    /**
     * @brief: 事件的重写
     */
    virtual void resizeEvent(QResizeEvent *event)override;

    /**
     * @brief keyPressEvent:相应键盘按键事件
     * @param keyEvent:按键：E导出，F大小调整，F5刷新，C复制
     */
    void keyPressEvent(QKeyEvent *keyEvent) override;

    /**
     * @brief event:事件变化
     * @param event事件
     * @return 布尔
     */
    bool event(QEvent *event) override;

private:
    /** @brief initWindow:对窗口进行一系列的初始化操作*/
    void initWindow();

    /** @brief initWindowSize:初始化窗口大小*/
    void initWindowSize();

    /** @brief initWidgets:初始化界面相关的内容*/
    void initWidgets();

    /**
     * @brief refreshDataBase:刷新设备信息
     */
    void refreshDataBase();

private slots:

    /**
     * @brief loadingFinishSlot:加载设备信息结束 槽
     * @param message:提示信息
     */
    void loadingFinishSlot(const QString &message);

    /**
     * @brief slotListItemClicked:ListView item点击槽函数
     * @param itemStr:item显示字符串
     */
    void slotListItemClicked(const QString &itemStr);

    /**
     * @brief slotRefreshInfo:刷新信息槽函数
     */
    void slotRefreshInfo();

    /**
     * @brief slotExportInfo:导出信息槽函数
     */
    void slotExportInfo();

    /**
     * @brief changeUI:UI界面变化,BIOS界面行高
     */
    void changeUI();

signals:
    /**
     * @brief fontChange:系统字体变化
     */
    void fontChange();

private:
    /**@brief:主窗口的stackWidget，主要有两个widget，一个是等待界面，还有一个是信息显示界面*/
    DStackedWidget        *mp_MainStackWidget;
    WaitingWidget         *mp_WaitingWidget;
    DeviceWidget          *mp_DeviceWidget;
    LoadInfoThread        *mp_WorkingThread;
    bool                  m_refreshing = false;                  // 判断界面是否正在刷新
    bool                  m_IsFirstRefresh = true;
};

#endif // MAINWINDOW_H
