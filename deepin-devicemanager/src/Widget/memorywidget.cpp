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

#include "memorywidget.h"
#include "../deviceinfoparser.h"
#include "DTableWidget"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMemory.h"
#include <QVBoxLayout>
#include <DApplication>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE

MemoryWidget::MemoryWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Memory"))
{
    initWidgetEX();
}

void MemoryWidget::initWidgetEX()
{
    // 获取鼠标设备
    const QList<DeviceMemory> &devices = DeviceManager::instance()->getMemoryDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No memory found"));
        return;
    }

    // 初始化表格
    initTable(devices);

    // 初始化详细信息
    initDetails(devices);

    // 设置overview
    setOverView(devices);
}
void MemoryWidget::initTable(const QList<DeviceMemory> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor"), tr("Type"), tr("Speed"),  tr("Size")};

    foreach (const DeviceMemory &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor(),
            device.type(),
            device.speed(),
            device.size()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void MemoryWidget::initDetails(const QList<DeviceMemory> &devices)
{
//    int size = devices.size();
    foreach (const DeviceMemory &device, devices) {
        addDeviceDetail(device, true);
    }
}
void MemoryWidget::addDeviceDetail(const DeviceMemory &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Size"), device.size(), attributes);
    addDeviceAttribute(tr("Type"), device.type(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);
    addDeviceAttribute(tr("Total Width"), device.totalBandwidth(), attributes);
    addDeviceAttribute(tr("Data Width"), device.dataBandwidth(), attributes);
    addDeviceAttribute(tr("Locator"), device.locator(), attributes);
    addDeviceAttribute(tr("Serial Number"), device.serialNumber(), attributes);
    addDeviceAttribute(tr("Configured Speed"), device.configuredSpeed(), attributes);
    addDeviceAttribute(tr("Minimum Voltage"), device.minimumVoltage(), attributes);
    addDeviceAttribute(tr("Maximum Voltage"), device.maximumVoltage(), attributes);
    addDeviceAttribute(tr("Configured Voltage"), device.configuredVoltage(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.vendor() + " " + device.name() : "", attributes );
}

void MemoryWidget::setOverView(const QList<DeviceMemory> &devices)
{
    foreach (const DeviceMemory &device, devices) {
        overviewInfo_.value += QString("%1(%2 %3 %4)/").arg(device.size()).arg(device.name() != "" ? device.name() : device.vendor()).arg(device.type()).arg(device.speed());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
