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

#include "otherpcidevice.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceOtherPCI.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

OtherPciDeviceWidget::OtherPciDeviceWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Other PCI Devices"))
{
    initWidgetEx();
}

void OtherPciDeviceWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceOtherPCI> &devices = DeviceManager::instance()->getOtherPCIDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No other PCI devices found"));
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
void OtherPciDeviceWidget::initTable(const QList<DeviceOtherPCI> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor")};

    foreach (const DeviceOtherPCI &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void OtherPciDeviceWidget::initDetails(const QList<DeviceOtherPCI> &devices)
{
    int size = devices.size();
    foreach (const DeviceOtherPCI &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void OtherPciDeviceWidget::addDeviceDetail(const DeviceOtherPCI &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.version(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Width"), device.width(), attributes);
    addDeviceAttribute(tr("Clock"), device.clock(), attributes);
    addDeviceAttribute(tr("Latency"), device.latency(), attributes);
    addDeviceAttribute(tr("IRQ"), device.irq(), attributes);
    addDeviceAttribute(tr("Memory Address"), device.memory(), attributes);
    addDeviceAttribute(tr("Input/Output"), device.inputOutput(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.model() : "", attributes );
}

void OtherPciDeviceWidget::setOverView(const QList<DeviceOtherPCI> &devices)
{
    foreach (const DeviceOtherPCI &device, devices) {
        overviewInfo_.value += QString("%1/").arg(device.name());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}


