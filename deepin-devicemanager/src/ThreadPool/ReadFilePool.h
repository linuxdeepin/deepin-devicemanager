#ifndef READFILEPOOL_H
#define READFILEPOOL_H

#include <QObject>
#include <QThreadPool>
#include <QMutex>

class ReadFilePool;

/**
 * @brief The CmdTask class
 * 线程池的任务类,执行命令的任务类
 */
class CmdTask: public QRunnable
{
public:
    CmdTask(QString key, QString file, QString info, ReadFilePool *parent);
    ~CmdTask();
protected:
    void run() override;
private:
    QString m_Key;
    QString m_File;
    QString m_Info;
    ReadFilePool *mp_Parent;
};


class ReadFilePool : public QThreadPool
{
public:
    ReadFilePool();

    /**
     * @brief readAllFile : 加载设备信息
     */
    void readAllFile();

    /**
     * @brief finishedCmd
     * @param info
     * @param cmdInfo
     */
    void finishedCmd(const QString &info, const QMap<QString, QList<QMap<QString, QString> > > &cmdInfo);

private:
    /**
     * @brief initCmd : 初始化命令列表
     */
    void initCmd();

private:
    QString                      m_Arch;
    QList<QStringList>           m_CmdList;
    QMutex m_lock;
};

#endif // READFILEPOOL_H
