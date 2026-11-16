// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADINFOTHREAD_H
#define LOADINFOTHREAD_H

#include <QObject>
#include <QThread>

#include <atomic>

#include "GetInfoPool.h"
#include "GenerateDevicePool.h"

class LoadInfoThread : public QThread
{
    Q_OBJECT
public:
    LoadInfoThread();
    ~LoadInfoThread() override;

    /**
     * @brief setFramework：设置架构
     * @param arch:架构
     */
    void setFramework(const QString &arch);

signals:
    void finished(const QString &message);

protected:
    void run() override;

private slots:
    /**
     * @brief slotFinishedReadFilePool
     * @param info
     */
    void slotFinishedReadFilePool(const QString &info);

private:
    /**
     * @brief waitServerRefreshFinished 等待服务端全量刷新结束
     */
    void waitServerRefreshFinished();

private:
    GetInfoPool mp_ReadFilePool;
    GenerateDevicePool mp_GenerateDevicePool;
    std::atomic<bool> m_Running { false };                 //<! 标识是否正在运行
    std::atomic<bool> m_FinishedReadFilePool { false };    //<! 标识生成读文件的线程池是否结束
};

#endif // LOADINFOTHREAD_H
