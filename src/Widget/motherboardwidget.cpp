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

#include "motherboardwidget.h"
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceBios.h"
#include "DeviceManager/DeviceManager.h"
#include <DApplication>
#include "commondefine.h"

DWIDGET_USE_NAMESPACE

MotherboardWidget::MotherboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Motherboard"))
{
    initWidgetEx();
}

void MotherboardWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceBios> &devices = DeviceManager::instance()->getBiosDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No motherboard found"));
        return;
    }

    // 初始化详细信息
    initDetails(devices);

    // 设置overview
    setOverView(devices);
}
void MotherboardWidget::initTable(const QList<DeviceBios> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"),  tr("Vendor") };

    foreach (const DeviceBios &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void MotherboardWidget::initDetails(const QList<DeviceBios> &devices)
{
    foreach (const DeviceBios &device, devices) {
        addDeviceDetail(device, true);
    }
}
void MotherboardWidget::addDeviceDetail(const DeviceBios &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Chipset"), device.chipsetFamily(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes );
}

void MotherboardWidget::setOverView(const QList<DeviceBios> &devices)
{
    foreach (const DeviceBios &device, devices) {
        if (device.isBoard()) {
            overviewInfo_.value += QString("%1/").arg(device.productName());
        }
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}

