// SPDX-FileCopyrightText: 2019 - 2026 UnionTech Software Technology Co., Ltd.
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
    auto it = m_MapInfo.find(key);
    if (it != m_MapInfo.end()) {
        return it.value();
    }
    static const QString empty;
    return empty;
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
    QString hwinfo = m_MapInfo.value("hwinfo");
    QString pathT = path;
    // Only strip the "/sys" prefix, not every occurrence of the substring.
    // QString::replace("/sys", "") would corrupt paths like "/sys/class/sys-block".
    if (pathT.startsWith("/sys")) {
        pathT = pathT.mid(4);
    }
    // Guard: a bare "/sys" yields an empty pattern that matches every line.
    if (pathT.isEmpty()) {
        qCDebug(appLog) << "Path exists: false";
        return false;
    }
    // Match line by line to avoid false positives from bare substring matching
    // across the entire hwinfo text (e.g. "devices/usb1" matching "devices/usb10").
    const QStringList lines = hwinfo.split('\n');
    for (const QString &line : lines) {
        if (line.contains(pathT)) {
            qCDebug(appLog) << "Path exists: true";
            return true;
        }
    }
    qCDebug(appLog) << "Path exists: false";
    return false;
}

