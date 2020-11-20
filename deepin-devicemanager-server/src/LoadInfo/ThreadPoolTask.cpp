#include "ThreadPoolTask.h"

#include <QTime>
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QDir>

#include <unistd.h>

ThreadPoolTask::ThreadPoolTask(QString cmd, QString file, bool replace, int waiting, QObject *parent)
    : QObject(parent),
      m_Cmd(cmd),
      m_File(file),
      m_CanNotReplace(replace),
      m_Waiting(waiting)
{

}

ThreadPoolTask::~ThreadPoolTask()
{

}

void ThreadPoolTask::run()
{
    // 不用重新采集的信息，如果文件存在就不需要重新采集
    QString path = "/tmp/device-info/" + m_File;
    QFile file(path);
    if (m_CanNotReplace && file.exists()) {
        return;
    }

    // 执行命令
    runCmd(m_Cmd);

    if (m_File == "lsblk_d.txt") {
        // 如果命令是 lsblk  , 则需要执行 smartctl --all /dev/***命令
        loadSmartctlFile(file);
    } else if (m_File == "lspci.txt") {
        // 如果命令是 lspci  , 则需要执行 lspci -v -s %1 > lspci_vs.txt 命令
        loadLspciVSFile(file);
    }
}

void ThreadPoolTask::runCmd(const QString &cmd)
{
    // 开始时刻
    qint64 begin = QDateTime::currentMSecsSinceEpoch();

    QProcess process;
    QStringList options;

    // QProcess执行带管道的命令
    options << "-c" << cmd;
    process.start("/bin/bash", options);
    process.waitForFinished(m_Waiting);

    // 结束时刻
    qint64 end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << cmd << " *********************************** " << end - begin << "ms";
}

void ThreadPoolTask::loadSmartctlFile(QFile &file)
{
    // 加载smartctl信息
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        QStringList lines = info.split("\n");
        foreach (QString line, lines) {
            QStringList words = line.replace(QRegExp("[\\s]+"), " ").split(" ");
            // NAME ROTA
            if (words.size() != 2 || words[0] == "NAME") {
                continue;
            }

            QString smartCmd = QString("smartctl --all /dev/%1 > /tmp/device-info/smartctl_%2.txt").arg(words[0].trimmed()).arg(words[0].trimmed());
            runCmd(smartCmd);
        }
        file.close();
    }
}

void ThreadPoolTask::loadLspciVSFile(QFile &file)
{
    // 加载lspci -v -s xxx信息
    if (file.open(QIODevice::ReadOnly)) {
        QString info = file.readAll();
        QStringList lines = info.split("\n");
        foreach (const QString &line, lines) {
            QStringList words = line.split(" ");
            if (words.size() < 2) {
                continue;
            }

            // 获取ISA bridge唯一标识
            if (words[1] == QString("ISA")) {
                QString cmd = QString("lspci -v -s %1 > /tmp/device-info/lspci_vs.txt").arg(words[0].trimmed());
                runCmd(cmd);
                break;
            }
        }
        file.close();
    }
}
