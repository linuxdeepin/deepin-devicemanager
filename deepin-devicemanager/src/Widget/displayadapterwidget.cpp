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

#include "displayadapterwidget.h"
#include "../deviceinfoparser.h"
#include"../DeviceManager/DeviceGpu.h"
#include"../DeviceManager/DeviceManager.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

DisplayadapterWidget::DisplayadapterWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Display Adapter"))
{
    initWidgetEX();
}
void DisplayadapterWidget::initWidgetEX()
{
    // 获取鼠标设备
    const QList<DeviceGpu> &devices = DeviceManager::instance()->getGPUDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No GPU found"));
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
void DisplayadapterWidget::initTable(const QList<DeviceGpu> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor")};

    foreach (const DeviceGpu &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void DisplayadapterWidget::initDetails(const QList<DeviceGpu> &devices)
{
    int size = devices.size();
    foreach (const DeviceGpu &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void DisplayadapterWidget::addDeviceDetail(const DeviceGpu &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes, true);
    addDeviceAttribute(tr("Model"), device.model(), attributes, true);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes, true);
    addDeviceAttribute(tr("Version"), device.version(), attributes, true);
    addDeviceAttribute(tr("Graphics Memory"), device.graphicsMemory(), attributes, true);
    addDeviceAttribute(tr("Width"), device.busWidth(), attributes, true);
    addDeviceAttribute(tr("IRQ"), device.IRQ(), attributes, true);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes, true);
    addDeviceAttribute(tr("Display Output"), device.displayOutput(), attributes, true);
    addDeviceAttribute(tr("VGA"), device.VGA(), attributes, true);
    addDeviceAttribute(tr("HDMI"), device.HDMI(), attributes, true);
    addDeviceAttribute(tr("eDP"), device.eDP(), attributes, true);
    addDeviceAttribute(tr("DP"), device.displayPort(), attributes, true);
    addDeviceAttribute(tr("Clock"), device.clock(), attributes, true);
    addDeviceAttribute(tr("Description"), device.description(), attributes, true);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes, true);
    addDeviceAttribute(tr("Current Resolution"), device.currentResolution(), attributes, true);
    addDeviceAttribute(tr("Minimum Resolution"), device.minimumResolution(), attributes, true);
    addDeviceAttribute(tr("Maximum Resolution"), device.maximumResolution(), attributes, true);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes, true);
    addDeviceAttribute(tr("IO Port"), device.ioPort(), attributes, true);
    addDeviceAttribute(tr("Memory"), device.memory(), attributes, true);
    addDeviceAttribute(tr("Physical ID"), device.physID(), attributes, true);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes, true);

    // 显示到界面
    addSubInfo(withTitle ? device.model() : "", attributes );
}

void DisplayadapterWidget::setOverView(const QList<DeviceGpu> &devices)
{
    foreach (const DeviceGpu &device, devices) {
        overviewInfo_.value += QString("%1/").arg(device.name());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}

