/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
* Maintainer: Jun.Liu <liujuna@uniontech.com>
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
    Cmd()
    {
        cmd = "";
        file = "";
        canNotReplace = false;
        waitingTime = -1;
    }

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
    ThreadPool(QObject *parent = nullptr);

    /**
     * @brief generateDeviceFile : load device info
     */
    void generateDeviceFile();

private:
    /**
     * @brief initCmd init all cmd
     */
    void initCmd();

private:
    QList<Cmd>        m_ListCmd;             // all cmd
};

#endif // THREADPOOL_H
