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

#include "keyboardwidget.h"
#include "deviceinfoparser.h"
#include "DeviceManager/DeviceManager.h"
#include "DeviceManager/DeviceKeyboard.h"
#include <DApplication>
#include <linux/input.h>

DWIDGET_USE_NAMESPACE

KeyboardWidget::KeyboardWidget(QWidget *parent) : DeviceInfoWidgetBase(parent, tr("Keyboard"))
{
    initWidgetEX();
}


void KeyboardWidget::initWidgetEX()
{
    // 获取存储设备
    const QList<DeviceKeyboard> &devices = DeviceManager::instance()->getKeyboardDevices();
    int deviceNum = devices.size();
    if ( deviceNum < 1 ) {
        setCentralInfo(tr("No keyboard found"));
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
void KeyboardWidget::initTable(const QList<DeviceKeyboard> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Model"),  tr("Vendor") };

    foreach (const DeviceKeyboard &device, devices) {
        QStringList tab = {
            device.model(),
            device.vendor()
        };
        tabList.append(tab);
    }

    addTable( headers, tabList);
}
void KeyboardWidget::initDetails(const QList<DeviceKeyboard> &devices)
{
    int size = devices.size();
    foreach (const DeviceKeyboard &device, devices) {
        addDeviceDetail(device, size == 1 ? false : true);
    }
}
void KeyboardWidget::addDeviceDetail(const DeviceKeyboard &device, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("Interface"), device.interface(), attributes);
    addDeviceAttribute(tr("Version"), device.version(), attributes);
    addDeviceAttribute(tr("Bus Info"), device.busInfo(), attributes);
    addDeviceAttribute(tr("Capabilities"), device.version(), attributes);
    addDeviceAttribute(tr("Driver"), device.driver(), attributes);
    addDeviceAttribute(tr("Maximum Power"), device.maxinumPower(), attributes);
    addDeviceAttribute(tr("Speed"), device.speed(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? device.model() : "", attributes );
}

void KeyboardWidget::setOverView(const QList<DeviceKeyboard> &devices)
{
    foreach (const DeviceKeyboard &device, devices) {
        overviewInfo_.value += QString("%1 (%2)/").arg(device.name()).arg(device.model());
    }
    overviewInfo_.value.replace(QRegExp("/$"), "");
}


