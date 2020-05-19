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

#include "networkadapterwidget.h"
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceNetwork.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

NetworkadapterWidget::NetworkadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Network Adapter"))
{
    initWidgetEx();
}

void NetworkadapterWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceNetwork> &devices = DeviceManager::instance()->getNetworkDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No network adapter found"));
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
void NetworkadapterWidget::initTable(const QList<DeviceNetwork> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"),  tr("Vendor") };

    foreach (const DeviceNetwork &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void NetworkadapterWidget::initDetails(const QList<DeviceNetwork> &devices)
{
    int size = devices.size();
    foreach (const DeviceNetwork &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void NetworkadapterWidget::addDeviceDetail(const DeviceNetwork &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Type"), device.model(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Driver Version"), device.driverVersion(), attributes);
    addDeviceAttribute(tr("Logical Name"), device.logicalName(), attributes);
    addDeviceAttribute(tr("MAC Address"), device.MACAddress(), attributes);
    addDeviceAttribute(tr("IRQ"), device.irq(), attributes);
    addDeviceAttribute(tr("Memory"), device.memory(), attributes);
    addDeviceAttribute(tr("Width"), device.width(), attributes);
    addDeviceAttribute(tr("Clock"), device.clock(), attributes);
    addDeviceAttribute(tr("Auto Negotiation"), device.autoNegotiation(), attributes);
    addDeviceAttribute(tr("Broadcast"), device.broadcast(), attributes);
    addDeviceAttribute(tr("Duplex"), device.duplex(), attributes);
    addDeviceAttribute(tr("Firmware"), device.firmware(), attributes);
    addDeviceAttribute(tr("IP"), device.ip(), attributes);
    addDeviceAttribute(tr("Latency"), device.latency(), attributes);
    addDeviceAttribute(tr("Link"), device.link(), attributes);
    addDeviceAttribute(tr("Multicast"), device.multicast(), attributes);
    addDeviceAttribute(tr("Port"), device.port(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);
    addDeviceAttribute(tr("Capacity"), device.capacity(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);
    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes );
}

void NetworkadapterWidget::setOverView(const QList<DeviceNetwork> &devices)
{
    foreach (const DeviceNetwork &device, devices) {
        overviewInfo_.value += QString("%1/").arg(device.name());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
