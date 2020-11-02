#ifndef THREADPOOLTASK_H
#define THREADPOOLTASK_H

#include <QObject>
#include <QRunnable>
#include <QFile>

class ThreadPoolTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ThreadPoolTask(QString cmd, QString file, bool replace, int waiting, QObject *parent = nullptr);
    ~ThreadPoolTask() override;

signals:
    void finished();

protected:
    void run() override;

private:
    void runCmd(const QString &cmd);
    void loadSmartctlFile(QFile &file);
    void loadLspciVSFile(QFile &file);

private:
    QString   m_Cmd;                  // cmd
    QString   m_File;                 // file name
    bool      m_CanNotReplace;        // Whether to replace if file existed
    int       m_Waiting;
};

#endif // THREADPOOLTASK_H
