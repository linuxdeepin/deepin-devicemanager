#include "DeviceInfoManager.h"

#include <QMutex>
#include <QDebug>

QMutex mutex;
DeviceInfoManager *DeviceInfoManager::s_Instance = nullptr;

DeviceInfoManager::DeviceInfoManager(QObject *parent)
    : QObject(parent)
{

}

void DeviceInfoManager::addInfo(const QString &key, const QString &value)
{
    QMutexLocker locker(&mutex);
    if (m_MapInfo.find(key) != m_MapInfo.end()) {
        m_MapInfo[key] = value;
    } else {
        m_MapInfo.insert(key, value);
    }
}

const QString &DeviceInfoManager::getInfo(const QString &key)
{
    QMutexLocker locker(&mutex);
    return m_MapInfo[key];
}

bool DeviceInfoManager::isInfoExisted(const QString &key)
{
    QMutexLocker locker(&mutex);
    if (m_MapInfo.find(key) != m_MapInfo.end()) {
        return true;
    } else {
        return false;
    }
}
