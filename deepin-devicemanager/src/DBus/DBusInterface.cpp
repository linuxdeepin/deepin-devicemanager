/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
* Maintainer: xxx.xx <xxxxxxx@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DBusInterface.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusInterface *> DBusInterface::s_Instance;
std::mutex DBusInterface::m_mutex;

const QString SERVICE_NAME = "com.deepin.devicemanager";
const QString SERVICE_PATH = "/com/deepin/devicemanager";

DBusInterface::DBusInterface()
    : mp_Iface(nullptr)
{
    // 初始化dbus
    init();
}

bool DBusInterface::getInfo(const QString &key, QString &info)
{
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getInfo", key);
    if (reply.isValid()) {
        info = reply.value();
        return true;
    } else {
        return false;
    }
}

void DBusInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, SERVICE_PATH, "", QDBusConnection::systemBus());
}
