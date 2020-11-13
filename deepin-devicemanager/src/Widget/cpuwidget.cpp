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
    getTrNumber();

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
                          arg(m_trNumber[device.cpuCores()]).\
                          arg(tr("Core(s)")).
                          arg(m_trNumber[device.logicalCPUNum()]).
                          arg(tr("Processor"));
}

void CpuWidget::getTrNumber()
{
    m_trNumber.insert(1, QObject::tr("One"));
    m_trNumber.insert(2, QObject::tr("Two"));
    m_trNumber.insert(4, QObject::tr("Four"));
    m_trNumber.insert(6, QObject::tr("Six"));
    m_trNumber.insert(8, QObject::tr("Eight"));

    m_trNumber.insert(10, QObject::tr("Ten"));
    m_trNumber.insert(12, QObject::tr("Twelve"));
    m_trNumber.insert(14, QObject::tr("Fourteen"));
    m_trNumber.insert(16, QObject::tr("Sixteen"));
    m_trNumber.insert(18, QObject::tr("Eighteen"));

    m_trNumber.insert(20, QObject::tr("Twenty"));
    m_trNumber.insert(22, QObject::tr("Twenty-two"));
    m_trNumber.insert(24, QObject::tr("Twenty-four"));
    m_trNumber.insert(26, QObject::tr("Twenty-six"));
    m_trNumber.insert(28, QObject::tr("Twenty-eight"));

    m_trNumber.insert(30, QObject::tr("Thirty"));
    m_trNumber.insert(32, QObject::tr("Thirty-two"));
    m_trNumber.insert(34, QObject::tr("Thirty-four"));
    m_trNumber.insert(36, QObject::tr("Thirty-six"));
    m_trNumber.insert(38, QObject::tr("Thirty-eight"));

    m_trNumber.insert(40, QObject::tr("Forty"));
    m_trNumber.insert(42, QObject::tr("Forty-two"));
    m_trNumber.insert(44, QObject::tr("Forty-four"));
    m_trNumber.insert(46, QObject::tr("Forty-six"));
    m_trNumber.insert(48, QObject::tr("Forty-eight"));

    m_trNumber.insert(50, QObject::tr("Fifty"));
    m_trNumber.insert(52, QObject::tr("Fifty-two"));
    m_trNumber.insert(54, QObject::tr("Fifty-four"));
    m_trNumber.insert(56, QObject::tr("Fifty-six"));
    m_trNumber.insert(58, QObject::tr("Fifty-eight"));

    m_trNumber.insert(60, QObject::tr("Sixty"));
    m_trNumber.insert(62, QObject::tr("Sixty-two"));
    m_trNumber.insert(64, QObject::tr("Sixty-four"));
    m_trNumber.insert(66, QObject::tr("Sixty-six"));
    m_trNumber.insert(68, QObject::tr("Sixty-eight"));

    m_trNumber.insert(70, QObject::tr("Seventy"));
    m_trNumber.insert(72, QObject::tr("Seventy-two"));
    m_trNumber.insert(74, QObject::tr("Seventy-four"));
    m_trNumber.insert(76, QObject::tr("Seventy-six"));
    m_trNumber.insert(78, QObject::tr("Seventy-eight"));

    m_trNumber.insert(80, QObject::tr("Eighty"));
    m_trNumber.insert(82, QObject::tr("Eighty-two"));
    m_trNumber.insert(84, QObject::tr("Eighty-four"));
    m_trNumber.insert(86, QObject::tr("Eighty-six"));
    m_trNumber.insert(88, QObject::tr("Eighty-eight"));

    m_trNumber.insert(90, QObject::tr("Ninety"));
    m_trNumber.insert(92, QObject::tr("Ninety-two"));
    m_trNumber.insert(94, QObject::tr("Ninety-four"));
    m_trNumber.insert(96, QObject::tr("Ninety-six"));
    m_trNumber.insert(98, QObject::tr("Ninety-eight"));

    m_trNumber.insert(100, QObject::tr("One hundred"));
    m_trNumber.insert(102, QObject::tr("One hundred and Two"));
    m_trNumber.insert(104, QObject::tr("One hundred and four"));
    m_trNumber.insert(106, QObject::tr("One hundred and Six"));
    m_trNumber.insert(108, QObject::tr("One hundred and Eight"));

    m_trNumber.insert(110, QObject::tr("One hundred and Ten"));
    m_trNumber.insert(112, QObject::tr("One hundred and Twelve"));
    m_trNumber.insert(114, QObject::tr("One hundred and Fourteen"));
    m_trNumber.insert(116, QObject::tr("One hundred and Sixteen"));
    m_trNumber.insert(118, QObject::tr("One hundred and Eighteen"));

    m_trNumber.insert(120, QObject::tr("One hundred and Twenty"));
    m_trNumber.insert(122, QObject::tr("One hundred and Twenty-two"));
    m_trNumber.insert(124, QObject::tr("One hundred and Twenty-four"));
    m_trNumber.insert(126, QObject::tr("One hundred and Twenty-six"));
    m_trNumber.insert(128, QObject::tr("One hundred and Twenty-eight"));
}
