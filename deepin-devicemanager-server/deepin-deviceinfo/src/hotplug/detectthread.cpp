// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "detectthread.h"
#include "monitorusb.h"
#include "DDLog.h"

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>

using namespace DDLog;

// 内核把 USB 设备信息填充完成的最长等待时间。
// 纯 sysfs 探测，等待期间几乎不消耗 CPU。
static constexpr int kUsbSettleMaxMs = 1000;
static constexpr int kUsbSettleStepMs = 100;

DetectThread::DetectThread(QObject *parent)
    : QThread(parent)
    , mp_MonitorUsb(new MonitorUsb())
{
    // 连接槽函数
    connect(mp_MonitorUsb, SIGNAL(usbChanged()), this, SLOT(slotUsbChanged()), Qt::QueuedConnection);
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

void DetectThread::slotUsbChanged()
{
    // 当监听到新的usb时，内核需要加载usb信息，而上层应用需要在内核处理之后获取信息。
    // 这里只做纯 sysfs 的就绪探测，不再 spawn hwinfo：
    // 原实现每秒执行一次 `hwinfo --usb --keyboard --mouse`（单次 300~400ms）
    // 直到设备信息变化，最多 10 秒即 11 个进程；在 USB 反复上报事件但设备列表
    // 实际未变时会跑满 10 秒，持续占满一个 CPU 核心，播放中的音频因此丢帧。
    qint64 begin = QDateTime::currentMSecsSinceEpoch();
    const bool ready = waitKernelUsbReady(kUsbSettleMaxMs);
    qCInfo(appLog) << " usb settle time:" << QDateTime::currentMSecsSinceEpoch() - begin
                   << " ready:" << ready;
    emit usbChanged();
}

bool DetectThread::waitKernelUsbReady(int maxWaitMs)
{
    const qint64 begin = QDateTime::currentMSecsSinceEpoch();
    while (true) {
        bool ready = true;

        QDir dir("/sys/bus/usb/devices");
        const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QFileInfo &entry : entries) {
            const QString path = entry.absoluteFilePath();
            if (entry.fileName().contains(':')) {
                // usb_interface：还没有 driver 符号链接说明驱动仍在 probe
                if (!QFileInfo::exists(path + "/driver")) {
                    ready = false;
                    break;
                }
            } else if (QFileInfo::exists(path + "/idVendor")) {
                // usb_device：已枚举
                continue;
            } else {
                ready = false;
                break;
            }
        }

        if (ready)
            return true;
        if (QDateTime::currentMSecsSinceEpoch() - begin >= maxWaitMs)
            return false;

        msleep(kUsbSettleStepMs);
    }
}
