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

    /**@brief:开始加载信息*/
    void loadCmdInfo();
    void generateInfo();

    /**@brief:完成加载信息*/
    void finishedCmd(const QString &info);
    void finishedGenerateDevice();

private:
    void getCmdList(QList<QStringList> &cmdList);
    void getTypeList(QList<DeviceType> &typeList);


signals:
    void finished(const QString &info);


private:
    int                    m_FinishedCmd;
    int                    m_FinishedGenerator;
    int                    m_AllCmdNum;
    int                    m_AllTypeNum;

    QMutex                 m_lock;

};

#endif // THREADPOOL_H
