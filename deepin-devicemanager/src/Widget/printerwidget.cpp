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

#include "printerwidget.h"
#include "../deviceinfoparser.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DevicePrint.h"
#include <DApplication>

DWIDGET_USE_NAMESPACE

PrinterWidget::PrinterWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Printer"))
{
    initWidgetEx();
}

void PrinterWidget::initWidgetEx()
{
    // 获取打印机设备
    const QList<DevicePrint> &devices = DeviceManager::instance()->getPrintDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No printer found"));
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
void PrinterWidget::initTable(const QList<DevicePrint> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor")};

    foreach (const DevicePrint &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}
void PrinterWidget::initDetails(const QList<DevicePrint> &devices)
{
    int size = devices.size();
    foreach (const DevicePrint &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void PrinterWidget::addDeviceDetail(const DevicePrint &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Serial Number"), device.serialNumber(), attributes);
    addDeviceAttribute(tr("Interface Type"), device.interfaceType(), attributes);
    addDeviceAttribute(tr("Status"), device.status(), attributes);
    addDeviceAttribute(tr("URI"), device.URI(), attributes);
    addDeviceAttribute(tr("Shared"), device.shared(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes);
}

void PrinterWidget::setOverView(const QList<DevicePrint> &devices)
{
    foreach (const DevicePrint &device, devices) {
        overviewInfo_.value += QString("%1/").arg(device.name());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
