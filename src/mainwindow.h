/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     AaronZhang <ya.zhang@archermind.com>
 *
 * Maintainer: AaronZhang <ya.zhang@archermind.com>
 * Maintainer: Yaobin <yao.bin@archermind.com>
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
#define MAINWIDNOW_H

#include <DMainWindow>
#include "dwidgetstype.h"
#include <QSplashScreen>
#include "DSpinner"
#include "DLabel"

class DeviceListView;
class DeviceInfoWidgetBase;

class MainWindow : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadDeviceWidget();
    void refreshDeviceWidget();

    void addAllDeviceinfoWidget();

    void addDeviceWidget(DeviceInfoWidgetBase* w, const QString& icon);
    void insertDeviceWidget(int index, DeviceInfoWidgetBase* w);

    bool isRefreshing(){ return refreshing_; }
    void refresh();
    void refreshDatabase();

    bool exportTo(/*const QString& file, const QString& selectFilter*/);
    void showDisplayShortcutsHelpDialog();
    void windowMaximizing();

    void currentDeviceChanged(const QString& device);

signals:
    void startLoadingWidgetSignal();

public slots:
    void initLoadingWidget();

private:
    void showSplashMessage(const QString& message);

protected:
    void keyPressEvent(QKeyEvent *keyEvent) override;

    void resizeEvent(QResizeEvent *event) override;

    void setSizeLimits();
    void saveSettings();
    void loadSettings();
private:
    DeviceListView* leftDeviceView_ = nullptr;
    Dtk::Widget::DStackedWidget* rightDeviceInfoWidget_ = nullptr;

    QMap<QString, DeviceInfoWidgetBase*> deviceInfoWidgetMap_;//widgetname　- widget
    bool firstAdd_ = true;
    QString currentDevice_;

    Dtk::Widget::DMenu* contextMenu_ = nullptr;
    Dtk::Widget::DLabel* loadLabel_ = nullptr;
    Dtk::Widget::DWidget* mainWidget_ = nullptr;
    Dtk::Widget::DWidget* loadingWidget_ = nullptr;

    bool refreshing_ = false;
    QSize m_sizeForQSetting;
};

#endif
