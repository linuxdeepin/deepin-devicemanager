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

#include <QThreadPool>
#include <QRunnable>
#include <QObject>
#include <QMutex>
class ThreadPool;

enum DeviceType {
    DT_Null       = 0,
    DT_Audio      = 1,
    DT_Bios       = 2,
    DT_Bluetoorh  = 3,
    DT_Cdrom      = 4,
    DT_Computer   = 5,
    DT_Cpu        = 6,
    DT_Gpu        = 7,
    DT_Image      = 8,
    DT_Keyboard   = 9,
    DT_Memory     = 10,
    DT_Monitor    = 11,
    DT_Mouse      = 12,
    DT_Network    = 13,
    DT_OtherPCI   = 14,
    DT_Storage    = 15,
    DT_Power      = 16,
    DT_Print      = 17,
    DT_Others     = 18
};

class CmdTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    CmdTask(QString key, QString cmd, QString debugFile, QString info, ThreadPool *parent = nullptr);
    ~CmdTask();
protected:
    void run();
private:
    QString m_Key;
    QString m_Cmd;
    QString m_DebugFile;
    QString m_Info;
    ThreadPool          *mp_Parent;
};

class GenerateTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    GenerateTask(DeviceType deviceType, ThreadPool *parent = nullptr);
    ~GenerateTask();
protected:
    void run();
private:
    DeviceType           m_Type;
    ThreadPool          *mp_Parent;
};



class ThreadPool : public QThreadPool
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);

    /**
     * @brief loadCmdInfo:开始加载信息
     */
    void loadCmdInfo();

    /**
     * @brief generateInfo:生成设备信息
     */
    void generateInfo();

    /**
     * @brief finishedCmd:完成加载信息
     * @param info:命令执行结束提示信息
     */
    void finishedCmd(const QString &info, const QMap<QString, QList<QMap<QString, QString> > > &);

    /**
     * @brief finishedGenerateDevice:完成生成设备信息
     * @param lst:busID
     */
    void finishedGenerateDevice(const QStringList &lst);

    /**
     * @brief setFramework:设置机器架构
     * @param arch:架构字符串
     */
    void setFramework(const QString &arch);

private:

    /**
     * @brief getCmdList:获取命令列表
     * @param cmdList:命令列表
     * @param arch:架构
     */
    void getCmdList(QList<QStringList> &cmdList, const QString &arch);

    /**
     * @brief getTypeList:获取设备类型列表
     * @param typeList:设备类型列表
     */
    void getTypeList(QList<DeviceType> &typeList);


signals:
    /**
     * @brief finished:命令执行结束信号
     * @param info:命令执行结束信息
     */
    void finished(const QString &info);


private:
    int                    m_FinishedCmd;
    int                    m_FinishedGenerator;
    int                    m_AllCmdNum;
    int                    m_AllTypeNum;

    QMutex                 m_lock;

    QString                m_Arch;

};

#endif // THREADPOOL_H
