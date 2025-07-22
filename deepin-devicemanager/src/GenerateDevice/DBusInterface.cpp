// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusInterface.h"
#include "DDLog.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QLoggingCategory>
#include <QProcess>

using namespace DDLog;

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusInterface *> DBusInterface::s_Instance;
std::mutex DBusInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceInfo";
const QString DEVICE_SERVICE_PATH = "/org/deepin/DeviceInfo";
const QString DEVICE_SERVICE_INTERFACE = "org.deepin.DeviceInfo";

DBusInterface::DBusInterface()
    : mp_Iface(nullptr)
{
    qCDebug(appLog) << "DBusInterface constructor";
    // 初始化dbus
    init();
}

bool DBusInterface::getInfo(const QString &key, QString &info)
{
    qCDebug(appLog) << "DBusInterface::getInfo start, key:" << key;
    // 调用dbus接口获取设备信息
    QDBusReply<QString> reply = mp_Iface->call("getInfo", key);
    if (reply.isValid()) {
        info = reply.value();
        qCDebug(appLog) << "DBusInterface::getInfo success, key:" << key << "info length:" << info.length();
        return true;
    } else {
        qCInfo(appLog) << "unsucess in getting info from getInfo :"  << key;
        QProcess process;
        QString command = "gdbus call --system --dest org.deepin.DeviceInfo --object-path /org/deepin/DeviceInfo --method org.deepin.DeviceInfo.getInfo hwinfo";
        process.start(command);
        process.waitForFinished();
        QByteArray output = process.readAllStandardOutput();
        QString outputStr = QString::fromLocal8Bit(output);
        qCInfo(appLog) << "getInfo gdbus out:" << outputStr.left(10); //debug detect service work is really unstable
        return false;
    }
}

void DBusInterface::refreshInfo()
{
    qCDebug(appLog) << "DBusInterface::refreshInfo";
    mp_Iface->asyncCall("refreshInfo");
}

bool DBusInterface::getGpuInfoByCustom(const QString &cmd, const QStringList &arguments, QString &gpuInfo)
{
    QDBusReply<QString> replyList = mp_Iface->call("getGpuInfoByCustom", cmd, arguments);
    if (replyList.isValid()) {
        gpuInfo = replyList.value();
        return true;
    } else {
        qCritical() << "Error: failed to call dbus to get gpu memery info! ";
        return false;
    }
}

void DBusInterface::init()
{
    qCDebug(appLog) << "DBusInterface::init start";
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        qCWarning(appLog) << "Cannot connect to the D-Bus session bus.";
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, DEVICE_SERVICE_PATH, DEVICE_SERVICE_INTERFACE, QDBusConnection::systemBus());
    qCDebug(appLog) << "DBusInterface::init end, iface created:" << (mp_Iface != nullptr);
}
