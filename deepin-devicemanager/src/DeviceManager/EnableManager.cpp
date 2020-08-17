#include "EnableManager.h"
#include <QDebug>
#include <QProcess>
#include <stdlib.h>

EnableManager *EnableManager::s_Instance = nullptr;

EnableManager::EnableManager()
{

}

bool EnableManager::enableDeviceByInput(const QString &name, bool enable)
{
    QString cmd = "xinput set-prop \"" + name + "\" " + "\"Device Enabled\" " + QString("%1").arg(enable ? 1 : 0);
//    qDebug() << cmd;
    QProcess process;
    int msecs = -1;
    process.start(cmd);
    bool res = process.waitForFinished(msecs);
//    QString output = process.readAllStandardOutput();
//    int exitCode = process.exitCode();
//    qDebug() << exitCode;
//    system(cmd.toStdString().c_str());
    return true;
}
