#include "DBusInterface.h"
#include "DeviceInfoManager.h"
#include "MainJob.h"

#include <QDebug>

DBusInterface::DBusInterface(QObject *parent)
    : QObject(parent)
    , mp_MainJob(nullptr)
{

}

void DBusInterface::setMainJob(MainJob *job)
{
    mp_MainJob = job;
}

QString DBusInterface::getInfo(const QString &key)
{
    // 不能用常引用
    return DeviceInfoManager::getInstance()->getInfo(key);
}

QString DBusInterface::execCmd(const QString &cmd)
{
    if (!mp_MainJob)
        return "0";

    // Request update
    if (cmd.startsWith("UPDATE_UI")) {
        if (mp_MainJob->isServerRunning())
            return QString::number(IR_SUCCESS);
        else
            return QString::number(IR_UPDATE);
    }

    // return result cmd
    INSTRUCTION_RES res = mp_MainJob->executeClientInstruction(cmd);
    return QString::number(res);
}
