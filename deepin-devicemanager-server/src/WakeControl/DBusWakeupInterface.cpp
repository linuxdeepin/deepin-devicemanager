#include "DBusWakeupInterface.h"
#include "EnableSqlManager.h"
#include "WakeupUtils.h"

#include <QDebug>
#include <QFile>

DBusWakeupInterface::DBusWakeupInterface(QObject* parent)
    : QObject (parent)
{
}

bool DBusWakeupInterface::setWakeupMachine(const QString& unique_id, const QString& path, bool wakeup)
{
    // 读写wake文件
    QString tpath;
    if(!WakeupUtils::wakeupPath(path,tpath))
        return false;
    if(!WakeupUtils::writeWakeupFile(tpath,wakeup))
        return false;

    // 将数据写到数据库或者从数据库删除数据
    saveWakeupInfo(unique_id,path,wakeup);
    return true;
}

void DBusWakeupInterface::saveWakeupInfo(const QString& unique_id, const QString& path, bool wakeup)
{
    if(EnableSqlManager::getInstance()->isWakeupUniqueIdExisted(unique_id)){
        EnableSqlManager::getInstance()->updateWakeData(unique_id,path,wakeup);
    }else{
        EnableSqlManager::getInstance()->insertWakeupData(unique_id,path,wakeup);
    }
}

