// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
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
    // waitingTime 不能默认 -1（QProcess 永久等待）：单条命令卡住会把整批刷新拖死。
    // 实测常规命令均在毫秒级，5s 已足够宽松；慢命令在 initCmd() 里单独放大。
    Cmd(): cmd(""), file(""), canNotReplace(false), waitingTime(5000)
    {}

    QString cmd;         //<! the cmd
    QString file;        //<! the file
    bool canNotReplace;  //<! mark can replace or not
    int waitingTime;     //<! 单条命令的等待上限(ms)，必须为正数
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

private:
    /**
     * @brief updateInfo 并发执行一组命令
     *
     * 只负责派发任务，不等待完成；超时由调用方通过 waitForDone(ms) 决定，
     * 避免库和调用方各设一个超时、叠加后超出对外承诺的上限。
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
};

#endif // THREADPOOL_H
