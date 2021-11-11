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

#include "computeroverviewwidget.h"
#include "../deviceinfoparser.h"
#include "QRegExp"
#include "math.h"
#include "../DeviceManager/DeviceComputer.h"
#include "../DeviceManager/DeviceManager.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

ComputerOverviewWidget::ComputerOverviewWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Overview"))
{

}

void ComputerOverviewWidget::setOverviewInfos( const QList<ArticleStruct> &others )
{
    QList<ArticleStruct> articles;
    DeviceComputer device;
    QList<DeviceComputer> devices = DeviceManager::instance()->getComputerDevices();
    if (devices.size() > 0) {
        device = devices[0];
    }

    ArticleStruct model(tr("Device"));
    model.value += device.vendor() + QString(" ");
    model.value += device.name() + QString(" ");
    model.value += device.type() + QString(" ");
    articles.push_back(model);

    // 添加操作系统信息
    ArticleStruct os(tr("Operating System"));
    os.value = device.osDescription() + " " + device.os();
    if ( device.homeUrl().isEmpty() == false) {
        os.externalLinks = true;
    }
    articles.push_back(os);

    articles.append(others);
    addInfo("", articles);
}
