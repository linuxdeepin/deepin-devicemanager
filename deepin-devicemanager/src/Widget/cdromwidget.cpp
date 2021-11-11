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

#include "cdromwidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceCdrom.h"
#include "../DeviceManager/DeviceManager.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CDRomWidget::CDRomWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("CD-ROM"))
{
    initWidgetEx();
}

void CDRomWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceCdrom> &devices = DeviceManager::instance()->getCdromDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No CD-ROM found"));
        return;
    }

    // 初始化表格
    if (deviceNum > 1) {
        initTable(devices);
    }

    // 初始化详细信息
    initDetails(devices);

    // 设置overview
    setOverView(devices);
}
void CDRomWidget::initTable(const QList<DeviceCdrom> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor") };

    foreach (const DeviceCdrom &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void CDRomWidget::initDetails(const QList<DeviceCdrom> &devices)
{
    int size = devices.size();
    foreach (const DeviceCdrom &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void CDRomWidget::addDeviceDetail(const DeviceCdrom &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Model"), device.type(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Maximum Power"), device.maxPower(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes );
}

void CDRomWidget::setOverView(const QList<DeviceCdrom> &devices)
{
    foreach (const DeviceCdrom &device, devices) {
        overviewInfo_.value += QString("%1 (%2)/").arg(device.name()).arg(device.type());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
