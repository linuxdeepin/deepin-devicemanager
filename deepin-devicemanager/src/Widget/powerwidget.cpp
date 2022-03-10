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

#include "powerwidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DevicePower.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PowerWidget::PowerWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Battery"))
{
    initWidgetEx();
}

void PowerWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DevicePower> &devices = DeviceManager::instance()->getPowerDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No battery found"));
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

void PowerWidget::initTable(const QList<DevicePower> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor")};

    foreach (const DevicePower &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void PowerWidget::initDetails(const QList<DevicePower> &devices)
{
    int size = devices.size();
    foreach (const DevicePower &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}

void PowerWidget::addDeviceDetail(const DevicePower &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Serial Number"), device.serialNumber(), attributes);
    addDeviceAttribute(tr("Type"), device.type(), attributes);
    addDeviceAttribute(tr("Status"), device.status(), attributes);
    addDeviceAttribute(tr("Capacity"), device.capacity(), attributes);
    addDeviceAttribute(tr("Voltage"), device.voltage(), attributes);
    addDeviceAttribute(tr("Slot"), device.slot(), attributes);
    addDeviceAttribute(tr("Design Capacity"), device.designCapacity(), attributes);
    addDeviceAttribute(tr("Design Voltage"), device.designVoltage(), attributes);
    addDeviceAttribute(tr("Temprature"), device.temprature(), attributes);
    addDeviceAttribute(tr("SBDS Version"), device.SBDSVersion(), attributes);
    addDeviceAttribute(tr("SBDS Serial Number"), device.SBDSSerialNumber(), attributes);
    addDeviceAttribute(tr("SBDS Manufacture Date"), device.SBDSManufactureDate(), attributes);
    addDeviceAttribute(tr("SBDS Chemistry"), device.SBDSChemistry(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes);
}

void PowerWidget::setOverView(const QList<DevicePower> &devices)
{
    foreach (const DevicePower &device, devices) {
        QString value = DApplication::translate("ManulTrack", device.name().trimmed().toStdString().data(), "");
        overviewInfo_.value += QString("%1/").arg(value);
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}

bool PowerWidget::infoIsEmpty()
{
    return true;
}

