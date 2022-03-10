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

#include "camerawidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceImage.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

CameraWidget::CameraWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Camera"))
{
    initWidgetEx();
}

void CameraWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceImage> &devices = DeviceManager::instance()->getImageDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No camera found"));
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

void CameraWidget::initTable(const QList<DeviceImage> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor") };

    foreach (const DeviceImage &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void CameraWidget::initDetails(const QList<DeviceImage> &devices)
{
    int size = devices.size();
    foreach (const DeviceImage &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}

void CameraWidget::addDeviceDetail(const DeviceImage &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.capabilities(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Maximum Power"), device.maxinumPower(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes);
}

void CameraWidget::setOverView(const QList<DeviceImage> &devices)
{
    foreach (const DeviceImage &device, devices) {
        overviewInfo_.value += QString("%1 (%2)/").arg(device.name()).arg(device.model());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
