#include "NotifyThread.h"

#include <QTimer>
#include <QtDebug>
#include <QProcess>

NotifyThread::NotifyThread(QObject *parent) : QThread(parent)
{

}

void NotifyThread::run()
{
    QString strUsername("");
    QProcess process;
    forever{
        //查看dde-desktop进程是否启动
        process.start("sh", QStringList() << "-c" << " ps aux | grep desktop |grep -v grep");
        process.waitForFinished(-1);
        if(process.readAll().isEmpty()){
            sleep(2);
            continue;
        }
        //查看用户名
        process.start("sh", QStringList() << "-c" << "who");
        process.waitForFinished(-1);
        strUsername = process.readAll().split(' ')[0];
        qInfo() << strUsername;
        if(strUsername.isEmpty()){
            sleep(2);
            continue;
        }

        QString name = QLocale::system().name();

        //通知前端
        QString strCmd = "runuser -l " + strUsername + " -c \"XDG_RUNTIME_DIR=\"/run/user/$(id -u " + strUsername + " )\" /usr/bin/deepin-devicemanager notify " + name + "\"";
        process.start("sh", QStringList() << "-c" << strCmd);
        process.waitForFinished(-1);
            break;
    }
}

