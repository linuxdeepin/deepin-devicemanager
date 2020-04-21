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

#pragma once

#include "deviceinfowidgetbase.h"
#include <QSize>
class MonitorWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit MonitorWidget(QWidget *parent = nullptr);

    bool loadXrandrDisplaySize();
    std::string getCmdResult(const std::string &strCmd);

    void initWidget() override;
    void initHwMonitor();
    void initWidgetEX();
    void addDeviceFromHwinfo(const QString& monitor,QString& deviceName,QSize& size,QStringList& tabItem);
    void addDeviceFromXrandr(const QString& monitor);
    void initWidgetFromBoth(const QStringList& hwinfoList,const QStringList& xrandrList);
    void getInfoFromXrandr(const QSize& size,const QStringList& xrandrList);
    void addOverviewInfo(const QString& info);

    QString parseMonitorSize(const QString& sizeDescription, double& inch,QSize& retSize);
    QString parseMonitorSize(const QString& sizeDescription,QSize& retSize);
    QString parseMonitorSize(const QString& sizeDescription,double& inch);
    QString parseMonitorSize(const QString& sizeDescription);
    QString parseDisplayRatio(const QString& resulotion);
    QString paraseDisplaySize(const double& width,const double& height);

    QString getMonitorSizeFromEDID(int index);
    ArticleStruct getRefreshRateScreen() const;
    void setRefreshRateScreen(const ArticleStruct &value);

private:
    void parseCurResolution(ArticleStruct &curResolution,ArticleStruct &resolutionList);



    QList<QStringList> tabList;
    QList<ArticleStruct> articles;
    QSet<QString> existArticles;
    //当前分辨率
    ArticleStruct currentResolution;
    //支持分辨率
    ArticleStruct resolutionList;
    //显示刷新率
    ArticleStruct refreshRateScreen;
    //显示比例字段
    ArticleStruct displayRatio;
};



