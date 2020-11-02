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

signals:
    void finished(const QString &message);

protected:
    void run() override;

private:
    ReadFilePool          *mp_ReadFilePool;
    GenerateDevicePool    *mp_GenerateDevicePool;
    bool            m_Running;    //<!  标识是否正在运行
};

#endif // LOADINFOTHREAD_H
