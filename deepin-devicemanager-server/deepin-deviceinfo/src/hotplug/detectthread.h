// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>

class MonitorUsb;

/**
 * @brief The DetectThread class
 *
 * 只负责运行 udev 监控线程并转发 usbChanged 通知。
 */
class DetectThread : public QThread
{
    Q_OBJECT
public:
    explicit DetectThread(QObject *parent);

    /**
     * @brief run
     */
    void run() override;

    /**
     * @brief setWorkingFlag 设置工作状态
     */
    void setWorkingFlag(bool flag);

signals:
    /**
     * @brief usbChanged usb/蓝牙设备发生变化
     */
    void usbChanged();

private:
    MonitorUsb *mp_MonitorUsb; //<! udev检测任务
};

#endif // DETECTTHREAD_H
