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

#include "cpuwidget.h"
#include "../deviceinfoparser.h"
#include"../DeviceManager/DeviceCpu.h"
#include"../DeviceManager/DeviceManager.h"
#include  <QObject>
#include <DApplication>
#include <QSet>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE

CpuWidget::CpuWidget(QWidget *parent)
    : DeviceInfoWidgetBase(parent, tr("CPU"))
{
    initWidgetEX();
}

void CpuWidget::initWidgetEX()
{
    // 获取鼠标设备
    const QList<DeviceCpu> &cpuDevices = DeviceManager::instance()->getCPUDevices();
    int deviceNum = cpuDevices.size();
    if (deviceNum < 1) {
        setCentralInfo(tr("No CPU found"));
        return;
    }

    // 初始化表格
    if (deviceNum > 1) {
        initTable(cpuDevices);
    }

    // 初始化详细信息
    initDetails(cpuDevices);

    // 设置overview
    setOverView(cpuDevices);
}

void CpuWidget::initTable(const QList<DeviceCpu> &devices)
{
    QList<QStringList> tabList;
    QStringList headers = { tr("Name"), tr("Vendor"), devices[0].frequencyIsRange() ? tr("Speed") : tr("Max Speed"), tr("Architecture")};

    foreach (const DeviceCpu &device, devices) {
        QStringList tab = {
            device.name(),
            device.vendor(),
            device.frequency(),
            device.architecture()
        };
        tabList.append(tab);
    }

    addTable(headers, tabList);
}

void CpuWidget::initDetails(const QList<DeviceCpu> &devices)
{
    int index = 0;
    int size = devices.size();
    foreach (const DeviceCpu &device, devices) {
        addDeviceDetail(device, index, size == 1 ? false : true);
        index++;
    }
}

void CpuWidget::addDeviceDetail(const DeviceCpu &device, int index, bool withTitle)
{
    QList<ArticleStruct> attributes;

    // 添加基本信息
    addDeviceAttribute(tr("Name"), device.name(), attributes);
    addDeviceAttribute(tr("Vendor"), device.vendor(), attributes);
    addDeviceAttribute(tr("CPU ID"), device.physicalID(), attributes);
    addDeviceAttribute(tr("Core ID"), device.coreID(), attributes);
    addDeviceAttribute(tr("Threads"), device.threadNum(), attributes);
    addDeviceAttribute(tr("Current Speed"), device.curFrequency(), attributes);
    addDeviceAttribute(tr("BogoMIPS"), device.bogoMIPS(), attributes);
    addDeviceAttribute(tr("Architecture"), device.architecture(), attributes);
    addDeviceAttribute(tr("CPU Family"), device.familly(), attributes);
    addDeviceAttribute(tr("Model"), device.model(), attributes);
    addDeviceAttribute(tr("Stepping"), device.step(), attributes);
    addDeviceAttribute(tr("L1d Cache"), device.cacheL1Data(), attributes);
    addDeviceAttribute(tr("L1i Cache"), device.cacheL1Data(), attributes);
    addDeviceAttribute(tr("L2 Cache"), device.cacheL2(), attributes);
    addDeviceAttribute(tr("L3 Cache"), device.cacheL3(), attributes);
    addDeviceAttribute(tr("Extensions"), device.extensions(), attributes);
    addDeviceAttribute(tr("Flags"), device.flags(), attributes);
    addDeviceAttribute(tr("Virtualization"), device.hardwareVirtual(), attributes);

    // 添加其他信息
    addOtherDeviceAttribute(device, attributes);

    // 显示到界面
    addSubInfo(withTitle ? QString("%1 %2").arg(tr("Processor")).arg(index) : "", attributes);
}

void CpuWidget::setOverView(const QList<DeviceCpu> &devices)
{
    // 获取cpu overview
    const DeviceCpu &device = devices[0];

    overviewInfo_.value = QString("%1 (%2%3 / %4%5)").\
                          arg(device.name()).\
                          arg(getTrNumber(device.cpuCores().toInt())).\
                          arg(tr("Core(s)")).
                          arg(getTrNumber(device.logicalCPUNum().toInt())).
                          arg(tr("Processor"));
}

const QString getTrNumber(int i)
{
    static const  QStringList trNumbers = {
        QObject::tr("One"),
        QObject::tr("One"),
        QObject::tr("Two"),
        QObject::tr("Four"),
        QObject::tr("Four"),
        QObject::tr("Six"),
        QObject::tr("Six"),
        QObject::tr("Eight"),
        QObject::tr("Eight"),
        //
        QObject::tr("Ten"),
        QObject::tr("Ten"),
        QObject::tr("Twelve"),
        QObject::tr("Twelve"),
        QObject::tr("Fourteen"),
        QObject::tr("Fourteen"),
        QObject::tr("Sixteen"),
        QObject::tr("Sixteen"),
        QObject::tr("Eighteen"),
        QObject::tr("Eighteen"),
        //                                         ,
        QObject::tr("Twenty"),
        QObject::tr("Twenty"),
        QObject::tr("Twenty-two"),
        QObject::tr("Twenty-two"),
        QObject::tr("Twenty-four"),
        QObject::tr("Twenty-four"),
        QObject::tr("Twenty-six"),
        QObject::tr("Twenty-six"),
        QObject::tr("Twenty-eight"),
        QObject::tr("Twenty-eight"),
        QObject::tr("Thirty"),
        QObject::tr("Thirty"),
        //                                         ,
        QObject::tr("Thirty-two"),
        QObject::tr("Thirty-two"),
        QObject::tr("Thirty-four"),
        QObject::tr("Thirty-four"),
        QObject::tr("Thirty-six"),
        QObject::tr("Thirty-six"),
        QObject::tr("Thirty-eight"),
        QObject::tr("Thirty-eight"),
        QObject::tr("Forty"),
        QObject::tr("Forty"),
        //                                         ,
        QObject::tr("Forty-two"),
        QObject::tr("Forty-two"),
        QObject::tr("Forty-four"),
        QObject::tr("Forty-four"),
        QObject::tr("Forty-six"),
        QObject::tr("Forty-six"),
        QObject::tr("Forty-eight"),
        QObject::tr("Forty-eight"),
        QObject::tr("Fifty"),
        QObject::tr("Fifty"),
        //                                         ,
        QObject::tr("Fifty-two"),
        QObject::tr("Fifty-two"),
        QObject::tr("Fifty-four"),
        QObject::tr("Fifty-four"),
        QObject::tr("Fifty-six"),
        QObject::tr("Fifty-six"),
        QObject::tr("Fifty-eight"),
        QObject::tr("Fifty-eight"),
        QObject::tr("Sixty"),
        QObject::tr("Sixty"),
        //                                         ,
        QObject::tr("Sixty-two"),
        QObject::tr("Sixty-two"),
        QObject::tr("Sixty-four"),
        QObject::tr("Sixty-four"),
        QObject::tr("One hundred and Twenty-eight"),
    };
    if (i < trNumbers.size()) {
        return trNumbers.at(i);
    }
    return trNumbers.first();
}
