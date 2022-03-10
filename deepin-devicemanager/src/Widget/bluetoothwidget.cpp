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

#include "bluetoothwidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceBluetooth.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE


BluetoothWidget::BluetoothWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Bluetooth"))
{
    initWidgetEx();
}

void BluetoothWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceBluetooth> &devices = DeviceManager::instance()->getBluetoothDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No Bluetooth device found"));
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

void BluetoothWidget::initTable(const QList<DeviceBluetooth> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor") };

    foreach (const DeviceBluetooth &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void BluetoothWidget::initDetails(const QList<DeviceBluetooth> &devices)
{
    int size = devices.size();
    foreach (const DeviceBluetooth &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}

void BluetoothWidget::addDeviceDetail(const DeviceBluetooth &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("MAC Address"), device.MAC(), attributes);
    addDeviceAttribute(tr("Logical Name"), device.logicalName(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Driver Version"), device.driverVersion(), attributes);
    addDeviceAttribute(tr("Maximum Power"), device.maximumPower(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes);
}

void BluetoothWidget::setOverView(const QList<DeviceBluetooth> &devices)
{
    foreach (const DeviceBluetooth &device, devices) {
        overviewInfo_.value += QString("%1 /").arg(device.name());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
