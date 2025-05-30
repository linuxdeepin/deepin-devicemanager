// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinfomanager.h"
#include "DDLog.h"

#include <QMutex>
#include <QLoggingCategory>

using namespace DDLog;

QMutex mutex;
std::atomic<DeviceInfoManager *> DeviceInfoManager::s_Instance;
std::mutex DeviceInfoManager::m_mutex;

DeviceInfoManager::DeviceInfoManager(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Initializing DeviceInfoManager";
}

void DeviceInfoManager::addInfo(const QString &key, const QString &value)
{
    qCDebug(appLog) << "Adding/updating info for key:" << key << "value length:" << value.length();
    QMutexLocker locker(&mutex);
    if (m_MapInfo.find(key) != m_MapInfo.end()) {
        qCDebug(appLog) << "Updating existing key:" << key;
        m_MapInfo[key] = value;
    } else {
        qCDebug(appLog) << "Inserting new key:" << key;
        m_MapInfo.insert(key, value);
    }
}

const QString &DeviceInfoManager::getInfo(const QString &key)
{
    qCDebug(appLog) << "Getting info for key:" << key;
    QMutexLocker locker(&mutex);
    return m_MapInfo[key];
}

bool DeviceInfoManager::isInfoExisted(const QString &key)
{
    qCDebug(appLog) << "Checking if info exists for key:" << key;
    QMutexLocker locker(&mutex);
    bool exists = m_MapInfo.find(key) != m_MapInfo.end();
    qCDebug(appLog) << "Info exists:" << exists;
    return exists;
}

bool DeviceInfoManager::isPathExisted(const QString &path)
{
    qCDebug(appLog) << "Checking if path exists:" << path;
    QMutexLocker locker(&mutex);
    const QString &hwinfo = m_MapInfo["hwinfo"];
    QString pathT = path;
    bool exists = hwinfo.contains(pathT.replace("/sys", ""));
    qCDebug(appLog) << "Path exists:" << exists;
    return exists;
}

