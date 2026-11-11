// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitorusb.h"
#include "controlinterface.h"
#include "mainjob.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QFile>
#include <QDateTime>

// 一批 udev 事件平息多久之后才执行合并的 hwinfo --usb
static constexpr qint64 kEventSettleMs = 500;

MonitorUsb::MonitorUsb()
    : m_workingFlag(true)
    , m_Udev(nullptr)
    , mp_Timer(new QTimer(this))
    , m_UsbChangeTime()
    , m_UsbChanged(false)
    , m_NeedControl(false)
    , m_NeedWakeup(false)
    , m_ControlDone(true)
    , m_LastEventTime(0)
{
    m_Udev = udev_new();
    if (!m_Udev) {
        printf("error!!!\n");
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
}

void MonitorUsb::monitor()
{
    fd_set fds;
    struct timeval tv;
    while (true) {
        if (!m_workingFlag)
            break;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        // 超时时间取短一些，便于在事件平息后及时执行一次合并的 hwinfo --usb
        tv.tv_sec = 0;
        tv.tv_usec = 200 * 1000;
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

        if (ret > 0 && FD_ISSET(fd, &fds)) {
            // 获取产生事件的设备映射
            struct udev_device *dev = udev_monitor_receive_device(mon);
            if (dev) {
                const char *action = udev_device_get_action(dev);

                // 监测蓝牙设备
                const char *devtype = udev_device_get_devtype(dev);
                if (devtype && 0 == strcmp(devtype, "link") && m_workingFlag) {
                    emit usbChanged();
                } else if (m_workingFlag && action
                           && (0 == strcmp("add", action) || 0 == strcmp("remove", action))
                           && 0 != udev_device_get_devnum(dev)) {
                    // 只有 add/remove 事件才需要更新缓存信息
                    if (0 == strcmp("add", action))
                        m_NeedControl = true;
                    m_NeedWakeup = true;
                    m_UsbChanged = true;
                    m_ControlDone = false;
                    m_UsbChangeTime = QDateTime::currentMSecsSinceEpoch();
                    m_LastEventTime = m_UsbChangeTime;
                }

                udev_device_unref(dev);
            }
        }

        // 一次插拔会连续产生多个事件，等这一批平息后再执行一次
        // hwinfo --usb，避免每个事件都 spawn 一个进程（单次约 200ms）
        if (!m_ControlDone && (m_NeedControl || m_NeedWakeup)) {
            if (QDateTime::currentMSecsSinceEpoch() - m_LastEventTime >= kEventSettleMs) {
                applyUsbControl();
                m_ControlDone = true;
            }
        }
    }
}

void MonitorUsb::applyUsbControl()
{
    const bool needControl = m_NeedControl;
    const bool needWakeup = m_NeedWakeup;
    m_NeedControl = false;
    m_NeedWakeup = false;

    QProcess process;
    process.start("hwinfo --usb");
    process.waitForFinished(-1);
    const QString info = process.readAllStandardOutput();

    if (needControl)
        ControlInterface::getInstance()->disableOutDevice(info);
    if (needWakeup)
        ControlInterface::getInstance()->updateWakeup(info);
}

void MonitorUsb::setWorkingFlag(bool flag)
{
    m_workingFlag = flag;
}

void MonitorUsb::slotTimeout()
{
    if (!m_UsbChanged || !m_workingFlag)
        return;
    if (QDateTime::currentMSecsSinceEpoch() - m_UsbChangeTime < 1000)
        return;
    m_UsbChanged = false;
    emit usbChanged();
}
