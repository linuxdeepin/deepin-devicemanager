// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorusb.h"
#include "controlinterface.h"
#include "mainjob.h"
#include "DDLog.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QFile>
#include <QDateTime>

using namespace DDLog;

MonitorUsb::MonitorUsb()
    : m_Udev(nullptr)
    , mp_Timer(new QTimer(this))
    , m_UsbChangeTime()
    , m_UsbChanged(false)
    , m_workingFlag(true)
{
    qCDebug(appLog) << "Initializing USB monitor";
    m_Udev = udev_new();
    if (!m_Udev) {
        qCWarning(appLog) << "Failed to create udev context";
    }
    // 创建一个新的monitor
    mon = udev_monitor_new_from_netlink(m_Udev, "udev");
    // 增加一个udev事件过滤器
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(mon, "bluetooth", nullptr);
    // 启动监控
    udev_monitor_enable_receiving(mon);
    // 获取该监控的文件描述符，fd就代表了这个监控
    fd = udev_monitor_get_fd(mon);

    // 定时器发送消息
    connect(mp_Timer, &QTimer::timeout, this, &MonitorUsb::slotTimeout);
    mp_Timer->start(1000);
    qCDebug(appLog) << "Started USB monitor timer";
}

void MonitorUsb::monitor()
{
    qCDebug(appLog) << "Starting USB monitor loop";
    char buf[10];
    fd_set fds;
    struct timeval tv;
    while (true) {
        if (!m_workingFlag) {
            qCDebug(appLog) << "Monitor stopped by working flag";
            break;
        }
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

        // 判断是否有事件产生
        if (!ret)
            continue;
        if (! FD_ISSET(fd, &fds))
            continue;

        // 获取产生事件的设备映射
        struct udev_device *dev = udev_monitor_receive_device(mon);
        if (!dev) {
            qCDebug(appLog) << "Received null device from udev";
            continue;
        }

        // 监测蓝牙设备
        if (0 == strcmp(udev_device_get_devtype(dev), "link") && m_workingFlag) {
            qCDebug(appLog) << "Bluetooth device change detected";
            emit usbChanged();
            continue;
        }

        // 获取事件并判断是否是插拔
        unsigned long long curNum = udev_device_get_devnum(dev);
        if (0 == curNum) {
            qCDebug(appLog) << "Ignoring device with num 0";
            udev_device_unref(dev);
            continue;
        }

        // 只有add和remove事件才会更新缓存信息
        strcpy(buf, udev_device_get_action(dev));
        if ((0 == strcmp("add", buf) || 0 == strcmp("remove", buf)) && m_workingFlag) {
            qCDebug(appLog) << "USB device " << buf << " detected";
            QProcess process;
            process.start("hwinfo --usb");
            process.waitForFinished(-1);
            QString info = process.readAllStandardOutput();
            if (0 == strcmp("add", buf)) {
                qCDebug(appLog) << "Processing USB add event";
                ControlInterface::getInstance()->disableOutDevice(info);
            }
            ControlInterface::getInstance()->updateWakeup(info);
            m_UsbChanged = true;
            m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();
            qCDebug(appLog) << "Updated USB device info";
        }

        udev_device_unref(dev);
    }
    qCDebug(appLog) << "Exited USB monitor loop";
}

void MonitorUsb::setWorkingFlag(bool flag)
{
    qCDebug(appLog) << "Setting working flag to:" << flag;
    m_workingFlag = flag;
}

void MonitorUsb::slotTimeout()
{
    if (!m_UsbChanged || !m_workingFlag)
        return;
    if (QDateTime::currentMSecsSinceEpoch() - m_UsbChangeTime < 1000)
        return;
    qCDebug(appLog) << "Emitting USB changed signal after debounce";
    m_UsbChanged = false;
    emit usbChanged();
}
