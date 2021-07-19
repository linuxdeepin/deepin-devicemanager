/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
* Maintainer: xxx.xx <xxxxxxx@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef LOADINFOTHREAD_H
#define LOADINFOTHREAD_H

#include <QObject>
#include <QThread>
#include "GetInfoPool.h"
#include "GenerateDevicePool.h"

class ZmqOrder;

class LoadInfoThread : public QThread
{
    Q_OBJECT
public:
    LoadInfoThread();
    ~LoadInfoThread();

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
    GetInfoPool mp_ReadFilePool;
    GenerateDevicePool mp_GenerateDevicePool;
    bool            m_Running;                      //<!  标识是否正在运行
    bool            m_FinishedReadFilePool;         //<!  标识生成读文件的线程池是否结束
    ZmqOrder       *mp_ZmqOrder;
};

#endif // LOADINFOTHREAD_H
