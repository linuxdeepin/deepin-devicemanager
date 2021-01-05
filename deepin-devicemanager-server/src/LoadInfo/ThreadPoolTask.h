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

#ifndef THREADPOOLTASK_H
#define THREADPOOLTASK_H

#include <QObject>
#include <QRunnable>
#include <QFile>

//#define PATH "/home/liujun/device-info/"
#define PATH "/tmp/device-info/"  // 设备文件存放的目录

/**
 * @brief The ThreadPoolTask class
 */
class ThreadPoolTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ThreadPoolTask(QString cmd, QString file, bool replace, int waiting, QObject *parent = nullptr);
    ~ThreadPoolTask() override;

signals:
    /**
     * @brief finished : finish task
     */
    void finished();

protected:
    void run() override;

private:
    /**
     * @brief runCmd : running a command
     * @param cmd : the command
     */
    void runCmd(const QString &cmd);
    void runCmd(const QString &cmd, QString &info);

    /**
     * @brief runCmdToCache
     * @param cmd
     */
    void runCmdToCache(const QString &cmd);

    /**
     * @brief loadSmartCtlInfoToCache
     * @param info
     */
    void loadSmartCtlInfoToCache(const QString &info);

    /**
     * @brief loadSgSmartCtlInfoToCache
     * @param info
     */
    void loadSgSmartCtlInfoToCache(const QString &info);

    /**
     * @brief loadLspciVSInfoToCache
     * @param info
     */
    void loadLspciVSInfoToCache(const QString &info);

    /**
     * @brief runCmdToTxt
     * @param cmd
     */
    void runCmdToFile(const QString &cmd);

    /**
     * @brief loadSmartctlInfoToFile
     * @param file
     */
    void loadSmartctlInfoToFile(QFile &file);

    /**
     * @brief loadLspciVSInfoToFile
     * @param file
     */
    void loadLspciVSInfoToFile(QFile &file);

private:
    QString   m_Cmd;                  //<! cmd
    QString   m_File;                 //<! file name
    bool      m_CanNotReplace;        //<! Whether to replace if file existed
    int       m_Waiting;              //<! waiting time
};

#endif // THREADPOOLTASK_H
