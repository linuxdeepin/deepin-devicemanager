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
        process.start("sh", QStringList() << "-c" << "who");
        process.waitForFinished(-1);
        strUsername = process.readAll().split(' ')[0];
        qInfo() << strUsername;
        if(strUsername.isEmpty()){
            sleep(5);
            continue;
        }

        QString strCmd = "runuser -l " + strUsername + " -c \"XDG_RUNTIME_DIR=\"/run/user/$(id -u " + strUsername + " )\" /usr/bin/deepin-devicemanager notify\"";
        process.start("sh", QStringList() << "-c" << strCmd);
        process.waitForFinished(-1);
            break;
    }
}

