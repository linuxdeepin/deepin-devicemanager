// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QObject>
#include <QThreadPool>
#include <QList>
#include <QVector>

/**
 * @brief The Cmd struct
 */
struct Cmd {
    Cmd(): cmd(""), file(""), canNotReplace(false), waitingTime(-1)
    {}

    QString cmd;         //<! the cmd
    QString file;        //<! the file
    bool canNotReplace;  //<! mark can replace or not
    int waitingTime;     //<! waiting time
};

/**
 * @brief The ThreadPool class
 */
class ThreadPool : public QThreadPool
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);

    /**
     * @brief generateDeviceFile : load device info
     */
    void loadDeviceInfo();

    /**
     * @brief updateDeviceFile
     */
    void updateDeviceInfo();

    /**
     * @brief updateHotplugInfo 热插拔时的局部刷新
     *
     * USB 插拔很频繁，没有必要每次都把整机信息重新采集一遍：lshw、upower、
     * lscpu、lspci、lpstat、bluetoothctl、hwinfo --framebuffer 等命令与插拔
     * 无关，却会把 CPU 占满数百毫秒到数秒，正在播放的音频因此丢帧。
     * 未列入 m_ListHotplug 的命令不会执行，其缓存保持上一次的值，不会丢失数据。
     */
    void updateHotplugInfo();

private:
    /**
     * @brief updateInfo 并发执行一组命令并等待结束
     * @param cmds 命令列表
     */
    void updateInfo(const QList<Cmd> &cmds);

    /**
     * @brief initCmd init all cmd
     */
    void initCmd();

private:
    QList<Cmd>        m_ListCmd;             // all cmd
    QList<Cmd>        m_ListUpdate;          // update cmd
    QList<Cmd>        m_ListHotplug;         // hotplug(usb) cmd
};

#endif // THREADPOOL_H
