// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTTHREAD_H
#define DETECTTHREAD_H

#include <QThread>
#include <QDateTime>

class MonitorUsb;

/**
 * @brief The DetectThread class
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
     * @brief usbChanged
     */
    void usbChanged();

private slots:
    /**
     * @brief slotUsbChanged usb发生变化时的曹函数处理
     */
    void slotUsbChanged();

private:
    /**
     * @brief waitKernelUsbReady 等待内核把 USB 设备信息填充完成
     *
     * 原实现通过反复执行 `hwinfo --usb --keyboard --mouse`（单次 300~400ms）
     * 并比对输出来判断"插拔是否处理完"，最多循环 10 秒，即最多 spawn 11 个
     * hwinfo 进程。在"USB 反复上报事件、但设备列表实际没变"的场景下会跑满
     * 10 秒，持续占满一个 CPU 核心，正在播放的音频因此丢帧（表现为"滋滋"声）。
     *
     * 这里改为纯 sysfs 探测：usb_device 都已有 idVendor、且 usb_interface 都已
     * 绑定驱动，说明内核已处理完毕。全程只读文件，不产生任何子进程。
     *
     * @param maxWaitMs 最长等待毫秒数
     * @return true 表示已经就绪
     */
    bool waitKernelUsbReady(int maxWaitMs);

private:
    MonitorUsb *mp_MonitorUsb; //<! udev检测任务
};

#endif // DETECTTHREAD_H
