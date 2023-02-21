// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    if ("is_server_running" != key) {
        return DeviceInfoManager::getInstance()->getInfo(key);
    }
    if (MainJob::serverIsRunning()) {
        return "1";
    }
    return "0";
}

void DBusInterface::refreshInfo()
{
    emit update();
}

void DBusInterface::setMonitorDeviceFlag(bool flag)
{
    MainJob *parentMainJob = dynamic_cast<MainJob *>(parent());
    if (parentMainJob != nullptr) {
        parentMainJob->setWorkingFlag(flag);
    }
}
