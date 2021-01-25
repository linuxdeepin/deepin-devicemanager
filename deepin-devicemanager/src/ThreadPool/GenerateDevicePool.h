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
#ifndef GENERATEDEVICEPOOL_H
#define GENERATEDEVICEPOOL_H

#include <QObject>
#include <QThreadPool>
#include <QMutex>

/**
 * @brief The DeviceType enum
 */
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


/**
 * @brief The GenerateTask class
 * 线程池的任务类，生成设备的任务类
 */
class GenerateTask: public QObject, public QRunnable
{
    Q_OBJECT

public:
    GenerateTask(DeviceType deviceType);
    ~GenerateTask();
signals:
    void finished(const QStringList &lst);
protected:
    void run();
private:
    DeviceType           m_Type;
};



class GenerateDevicePool : public QThreadPool
{
    Q_OBJECT
public:
    GenerateDevicePool();

    /**
     * @brief generateDevice
     */
    void generateDevice();

private:
    /**
     * @brief initType
     */
    void initType();

private slots:
    /**
     * @brief slotFinished : end operation
     * @param lst
     */
    void slotFinished(const QStringList &lst);

private:
    QList<DeviceType>            m_TypeList;
    int                          m_FinishedGenerator;
//    QMutex                       m_lock;
};

#endif // GENERATEDEVICEPOOL_H
