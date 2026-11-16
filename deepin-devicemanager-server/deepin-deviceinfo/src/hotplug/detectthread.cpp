// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectthread.h"
#include "monitorusb.h"

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_MonitorUsb(new MonitorUsb())
{
    connect(mp_MonitorUsb, &MonitorUsb::usbChanged, this, &DetectThread::usbChanged, Qt::QueuedConnection);
}

void DetectThread::run()
{
    if (mp_MonitorUsb) {
        mp_MonitorUsb->monitor();
    }
}

void DetectThread::setWorkingFlag(bool flag)
{
    mp_MonitorUsb->setWorkingFlag(flag);
    if (flag && !isRunning()) {
        start();
    }
}
