#include "DBusInterface.h"
#include "DeviceInfoManager.h"
#include "MainJob.h"
#include "EnableSqlManager.h"

#include <QDebug>
#include <QFile>

DBusInterface::DBusInterface(QObject *parent)
    : QObject(parent)
{

}

QString DBusInterface::getInfo(const QString &key)
{
    // 不能返回用常引用
    if("is_server_running" != key){
        return DeviceInfoManager::getInstance()->getInfo(key);
    }
    if(MainJob::serverIsRunning()){
        return "1";
    }
    return "0";
}
