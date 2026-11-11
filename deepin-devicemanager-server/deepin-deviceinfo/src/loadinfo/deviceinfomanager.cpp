// SPDX-FileCopyrightText: 2019 ~ 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinfomanager.h"

#include <QMutex>
#include <QLoggingCategory>

QMutex mutex;
std::atomic<DeviceInfoManager *> DeviceInfoManager::s_Instance;
std::mutex DeviceInfoManager::m_mutex;

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

const QString DeviceInfoManager::getInfo(const QString &key)
{
    QMutexLocker locker(&mutex);
    // 不能用 m_MapInfo[key]：QMap::operator[] 在 key 不存在时会插入一个空条目，
    // 导致 isInfoExisted() 误判为"已采集过"，canNotReplace 的命令将永远不会重试。
    // 同时返回引用会在解锁后被其它线程修改，这里改为返回值。
    const auto it = m_MapInfo.constFind(key);
    if (it == m_MapInfo.constEnd())
        return QString();
    return it.value();
}

bool DeviceInfoManager::isInfoExisted(const QString &key)
{
    QMutexLocker locker(&mutex);
    return m_MapInfo.constFind(key) != m_MapInfo.constEnd();
}

bool DeviceInfoManager::isPathExisted(const QString &path)
{
    QMutexLocker locker(&mutex);
    const auto it = m_MapInfo.constFind("hwinfo");
    if (it == m_MapInfo.constEnd())
        return false;
    QString pathT = path;
    return it.value().contains(pathT.replace("/sys", ""));
}

