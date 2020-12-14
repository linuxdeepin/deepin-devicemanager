#include "DBusInterface.h"

#include "DeviceInfoManager.h"

DBusInterface::DBusInterface(QObject *parent) : QObject(parent)
{

}

QString DBusInterface::getInfo(const QString &key)
{
    // 不能用常引用
    return DeviceInfoManager::getInstance()->getInfo(key);
}
