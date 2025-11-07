// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinterface.h"
#include "deviceinfomanager.h"
#include "mainjob.h"
#include "DDLog.h"

#include <QDBusConnection>
#include <QDBusMessage>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <polkit-qt5-1/PolkitQt1/Authority>
#else
#include <polkit-qt6-1/PolkitQt1/Authority>
#endif

using namespace DDLog;
using namespace PolkitQt1;
bool DeviceInterface::getUserAuthorPasswd()
{
#ifdef DISABLE_POLKIT
    return true;
#endif
    Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                             SystemBusNameSubject(message().service()),
                                                                             Authority::AllowUserInteraction);
    return result == Authority::Yes;
}

DeviceInterface::DeviceInterface(const char *name, QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "Initializing DeviceInterface for service:" << name;
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name))
            .registerObject("/org/deepin/DeviceInfo", this, opts);
}

QString DeviceInterface::getInfo(const QString &key)
{
    qCDebug(appLog) << "Getting info for key:" << key;

    // 获取设备信息需要身份验证
    if (!getUserAuthorPasswd()) {
        qCWarning(appLog) << "Authorization failed for getInfo operation";
        return "0";
    }

    // 不能返回用常引用
    if ("is_server_running" != key) {
        return DeviceInfoManager::getInstance()->getInfo(key);
    }
    if (MainJob::serverIsRunning()) {
        qCDebug(appLog) << "Server is running";
        return "1";
    }
    qCDebug(appLog) << "Server is not running";
    return "0";
}

void DeviceInterface::refreshInfo()
{
    if (!getUserAuthorPasswd()) {
        qCWarning(appLog) << "Authorization failed for refreshInfo operation";
        return;
    }
    
    emit sigUpdate();
}

void DeviceInterface::setMonitorDeviceFlag(bool flag)
{
    qCDebug(appLog) << "Setting monitor device flag to:" << flag;
    
    if (!getUserAuthorPasswd()) {
        qCWarning(appLog) << "Authorization failed for setMonitorDeviceFlag operation";
        return;
    }
    
    MainJob *parentMainJob = dynamic_cast<MainJob *>(parent());
    if (parentMainJob != nullptr) {
        parentMainJob->setWorkingFlag(flag);
        qCDebug(appLog) << "Monitor flag set successfully";
    } else {
        qCWarning(appLog) << "Failed to set monitor flag - parent MainJob not found";
    }
}
