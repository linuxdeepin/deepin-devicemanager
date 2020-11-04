#ifndef LOADINFOTHREAD_H
#define LOADINFOTHREAD_H

#include <QObject>
#include <QThread>

class ReadFilePool;
class GenerateDevicePool;

class LoadInfoThread : public QThread
{
    Q_OBJECT
public:
    LoadInfoThread();

    /**
     * @brief isRunning :判断当前进程是否正在运行
     * @return
     */
    bool isRunning();

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
    ReadFilePool          *mp_ReadFilePool;         //<!  生成读文件的线程池
    GenerateDevicePool    *mp_GenerateDevicePool;   //<!  生成设备线程池
    bool            m_Running;                      //<!  标识是否正在运行
    bool            m_FinishedReadFilePool;         //<!  标识生成读文件的线程池是否结束
};

#endif // LOADINFOTHREAD_H
