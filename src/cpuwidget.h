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

class CpuWidget : public DeviceInfoWidgetBase
{
    Q_OBJECT
public:
    explicit CpuWidget(QWidget *parent = nullptr);

    void initWidget() override;
    void addDetailFromLscpuAndCatcpu(const QString& precessor);
    void addDetailFromDmidecode(const QString& precessor);
    void addArticleStruct(const QString& name,const QString& command,const QString& item,const QString& attri,QList<ArticleStruct>& articles,QSet<QString>& existArticles);

    /**@brief:判断能不能从lsCpu获取频率，传入的参数是从lscpu命令中获取的最大最小频率*/
    bool isCanGetSpeedFromLsCpu(double& minMHz,double& maxMHz);
    void getSpeedFromLscpu(double& minHz,double& maxHz);
    void getSpeedFromDmidecode();
    void getTableHeader(bool getSpeedFromLscpu,QStringList& headers);
    void getTableContentFromLscpu(const QString& cpuModel,const QStringList& cpuList,QList<QStringList>& tabList);
    void getTableContentFromDmidecode(const QString& cpuModel,const QStringList& cpuList,QList<QStringList>& tabList);

private:
    QString speed_;
};
const QString getTrNumber(int i);
