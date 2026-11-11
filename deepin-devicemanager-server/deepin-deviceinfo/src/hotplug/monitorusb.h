// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITORUSB_H
#define MONITORUSB_H
#include <libudev.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <QObject>
#include <QTimer>

class MonitorUsb : public QObject
{
    Q_OBJECT
public:
    MonitorUsb();

    /**
     * @brief monitor
     */
    void monitor();
    /**
     * @brief setWorkingFlag 设置工作状态
     */
    void setWorkingFlag(bool flag);

signals:
    /**
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotTimeout
     */
    void slotTimeout();

private:
    /**
     * @brief applyUsbControl 采集当前 usb 信息并同步禁用/唤醒状态
     *
     * 一次插拔通常会产生多个 udev 事件（device / interface / ...），
     * 每次事件都执行一次 hwinfo --usb（约 200ms）会成倍放大 CPU 开销，
     * 因此合并为"一批事件结束后执行一次"。
     */
    void applyUsbControl();

private:
    bool                              m_workingFlag;        //<! 工作状态
    struct udev                       *m_Udev;              //<! udev Environment
    struct udev_monitor               *mon;                 //<! object of mon
    int                               fd;                   //<! fd
    QTimer                            *mp_Timer;            //<! 定时器
    qint64                            m_UsbChangeTime;      //<! 记录当前时间
    bool                              m_UsbChanged;         //<! 记录是否有usb插拔
    bool                              m_NeedControl;        //<! 本批事件是否需要同步禁用状态
    bool                              m_NeedWakeup;         //<! 本批事件是否需要同步唤醒状态
    bool                              m_ControlDone;        //<! 本批事件是否已经执行过 applyUsbControl
    qint64                            m_LastEventTime;      //<! 本批最后一次事件的时间
};

#endif // MONITORUSB_H
