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

#include "mousewidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMouse.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

MouseWidget::MouseWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Mouse"))
{
    //initWidget();
    initWidgetEX();
}

void MouseWidget::initWidgetEX()
{
    // 获取鼠标设备
    const QList<DeviceMouse> &mouseDevices = DeviceManager::instance()->getMouseDevices();
    int deviceNum = mouseDevices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No mouse found"));
        return;
    }

    // 初始化表格
    if (deviceNum > 1) {
        initTable(mouseDevices);
    }

    // 初始化详细信息
    initDetails(mouseDevices);

    // 设置overview
    setOverView(mouseDevices);
}

void MouseWidget::initTable(const QList<DeviceMouse> &mouseDevices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Type"), tr("Vendor")};

    foreach (const DeviceMouse &deviceMouse, mouseDevices) {
        QStringList tab = {
            deviceMouse.name(),
            deviceMouse.interface(),
            deviceMouse.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);

}

void MouseWidget::initDetails(const QList<DeviceMouse> &mouseDevices)
{
    int size = mouseDevices.size();
    foreach (const DeviceMouse &deviceMouse, mouseDevices) {
        addDeviceDetail(deviceMouse, size == 1 ? false : true);
    }
}

void MouseWidget::addDeviceDetail(const DeviceMouse &mouseDevice, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), mouseDevice.name(), attributes);
    addDeviceAttribute(tr("Vendor"), mouseDevice.vendor(), attributes);
    addDeviceAttribute(tr("Model"), mouseDevice.model(), attributes);
    addDeviceAttribute(tr("Interface"), mouseDevice.interface(), attributes);
    addDeviceAttribute(tr("Bus Info"), mouseDevice.busInfo(), attributes);
    addDeviceAttribute(tr("Version"), mouseDevice.version(), attributes);
    addDeviceAttribute(tr("Capabilities"), mouseDevice.capability(), attributes);
    addDeviceAttribute(tr("Driver"), mouseDevice.driver(), attributes);
    addDeviceAttribute(tr("Maximum Power"), mouseDevice.maxPower(), attributes);
    addDeviceAttribute(tr("Speed"), mouseDevice.speed(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(mouseDevice, attributes);

    // 显示到界面
    addSubInfo(withTitle ? mouseDevice.name() : "", attributes);
}

void MouseWidget::setOverView(const QList<DeviceMouse> &devices)
{
    foreach (const DeviceMouse &device, devices) {
        overviewInfo_.value += QString("%1 (%2)/").arg(device.name()).arg(device.model());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
