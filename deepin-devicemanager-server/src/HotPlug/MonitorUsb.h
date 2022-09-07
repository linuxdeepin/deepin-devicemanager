// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
    struct udev                       *m_Udev;              //<! udev Environment
    struct udev_monitor               *mon;                 //<! object of mon
    int                               fd;                   //<! fd
    QTimer                            *mp_Timer;            //<! 定时器
    qint64                            m_UsbChangeTime;      //<! 记录当前时间
    bool                              m_UsbChanged;         //<! 记录是否有usb插拔

};

#endif // MONITORUSB_H
