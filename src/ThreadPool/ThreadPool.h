#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <QThreadPool>
#include <QRunnable>
#include <QObject>
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
    DT_Others     = 15,
    DT_Power      = 16,
    DT_Print      = 17,
    DT_Storage    = 18,

    DT_Dmidecode  = 19,
    DT_Lshw       = 20,
    DT_Hwinfo     = 21
};


/*
 * @brief:通过命令获取信息的任务
*/
class DeviceTask : public QObject, public QRunnable
{
    Q_OBJECT

public:
    DeviceTask(DeviceType deviceType, ThreadPool *parent = nullptr);
    ~DeviceTask();
protected:
    void run();
private:
    DeviceType           m_Type;
    ThreadPool          *mp_Parent;
};

//class CmdTask: public QObject, public QRunnable
//{
//    Q_OBJECT

//public:
//    CmdTask();
//    ~CmdTask();
//protected:
//    void run();
//private:
//};



class ThreadPool : public QThreadPool
{
    Q_OBJECT
public:
    explicit ThreadPool(QObject *parent = nullptr);

    /**@brief:开始加载信息*/
    void loadDeviceInfo();

    /**@brief:完成加载信息*/
    void finishedTask(const QString &info);

signals:
    void finished(const QString &info);


private:
    QList<DeviceType>      m_ListDeviceType;
    int                    m_FinishedNum;

};

#endif // THREADPOOL_H
