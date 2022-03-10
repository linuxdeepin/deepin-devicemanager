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

#include "monitorwidget.h"
#include "../deviceinfoparser.h"
#include "../edidparser.h"
#include "math.h"
#include "../DeviceManager/DeviceManager.h"
#include "../DeviceManager/DeviceMonitor.h"
#include <QDate>
#include <DApplication>
#include "Logger.h"
#include <assert.h>
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

MonitorWidget::MonitorWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("Monitor"))
{
    initWidgetEx();
}

void MonitorWidget::initWidgetEx()
{
    // 获取鼠标设备
    const QList<DeviceMonitor> &devices = DeviceManager::instance()->getMonitorDevices();
    int deviceNum = devices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No monitor found"));
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

void MonitorWidget::initTable(const QList<DeviceMonitor> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"),  tr("Vendor") };

    foreach (const DeviceMonitor &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void MonitorWidget::initDetails(const QList<DeviceMonitor> &devices)
{
    int size = devices.size();
    foreach (const DeviceMonitor &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}

void MonitorWidget::addDeviceDetail(const DeviceMonitor &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Product Date"), device.productionWeek(), attributes);
    addDeviceAttribute(tr("Type"), device.model(), attributes);
    addDeviceAttribute(tr("Display Input"), device.displayInput(), attributes);
    addDeviceAttribute(tr("Interface Type"), device.interface(), attributes);
    addDeviceAttribute(tr("Serial Number"), device.serialNumber(), attributes);
    addDeviceAttribute(tr("Size"), device.screenSize(), attributes);
    addDeviceAttribute(tr("Display Ratio"), device.aspectRatio(), attributes);
    addDeviceAttribute(tr("Primary Monitor"), device.mainScreen(), attributes);
    addDeviceAttribute(tr("Current Resolution"), device.currentResolution(), attributes);
    addDeviceAttribute(tr("Support Resolution"), device.supportResolutions(), attributes);


    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.name() : "", attributes);
}

void MonitorWidget::setOverView(const QList<DeviceMonitor> &devices)
{
    foreach (const DeviceMonitor &device, devices) {
        overviewInfo_.value += QString("%1(%2)/").arg(device.name()).arg(device.screenSize());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}
