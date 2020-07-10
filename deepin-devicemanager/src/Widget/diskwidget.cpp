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

#include "diskwidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceStorage.h"
#include <DApplication>
//#include <QDebug>

DWIDGET_USE_NAMESPACE

DiskWidget::DiskWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Storage"))
{
    initWidgetEX();
}

void DiskWidget::initWidgetEX()
{
    // 获取存储设备
    const QList<DeviceStorage> &devices = DeviceManager::instance()->getStorageDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No disk found"));
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

void DiskWidget::initTable(const QList<DeviceStorage> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Model"),  tr("Vendor"), tr("Media Type"), tr("Size") };

    foreach (const DeviceStorage &device, devices) {
        QStringList tab = {
            device.model(),
            device.vendor(),
            device.mediaType(),
            device.size()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void DiskWidget::initDetails(const QList<DeviceStorage> &devices)
{
    int size = devices.size();
    foreach (const DeviceStorage &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}

void DiskWidget::addDeviceDetail(const DeviceStorage &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Media Type"), device.mediaType(), attributes);
    addDeviceAttribute(tr("Size"), device.size(), attributes);
    addDeviceAttribute(tr("Rotation Rate"), device.rotationRate(), attributes);
    addDeviceAttribute(tr("Interface"), device.interface(), attributes);
    addDeviceAttribute(tr("Serial Number"), device.serialNumber(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes);
    addDeviceAttribute(tr("Description"), device.description(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);
    addDeviceAttribute(tr("Firmware Version"), device.firmwareVersion(), attributes);
    addDeviceAttribute(tr("Power On Hours"), device.powerOnHours(), attributes);
    addDeviceAttribute(tr("Power Cycle Count"), device.powerCycleCount(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.model() : "", attributes);
}

void DiskWidget::setOverView(const QList<DeviceStorage> &devices)
{
    foreach (const DeviceStorage &device, devices) {
        overviewInfo_.value += QString("%1 (%2)/").arg(device.model()).arg(device.size());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
