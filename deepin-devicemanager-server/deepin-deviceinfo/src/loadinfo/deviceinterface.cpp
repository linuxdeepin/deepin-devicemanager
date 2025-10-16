// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "deviceinterface.h"
#include "deviceinfomanager.h"
#include "mainjob.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

#include <polkit-qt5-1/PolkitQt1/Authority>

constexpr char kGraphicsMemory[] { "Graphics Memory" };

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

bool DeviceInterface::getGpuMemInfoForFTDTM(QMap<QString, QString> &mapInfo)
{
    const QString filePath = "/sys/kernel/debug/gc/total_mem";
    QString totalValue;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Error opening /sys/kernel/debug/gc/total_mem:" << file.errorString();
        return false;
    }

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    if (content.isEmpty()) {
        qCritical() << "Error: /sys/kernel/debug/gc/total_mem File is empty!";
        return false;
    }

    QRegularExpression regex(R"((\d+(?:\.\d+)?)\s*\(?(MB|GB|KB|B)\)?)",
                                    QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch memInfoMatch = regex.match(content);

    if (!memInfoMatch.hasMatch()) {
        qCritical() << "Error: Failed to find memory info";
        return false;
    }

    double value = memInfoMatch.captured(1).toDouble();
    QString unit = memInfoMatch.captured(2).toUpper();

    if (unit == "MB") {
        totalValue = QString("%1GB").arg(value / 1024.0, 0, 'f', 2);
    } else if (unit == "GB") {
        totalValue = QString("%1GB").arg(value, 0, 'f', 2);
    } else if (unit == "KB") {
        totalValue = QString("%1GB").arg(value / (1024.0 * 1024.0), 0, 'f', 2);
    } else if (unit == "B") {
        totalValue = QString("%1GB").arg(value / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }

    mapInfo.insert(kGraphicsMemory, totalValue);

    return true;
}

DeviceInterface::DeviceInterface(const char *name, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals | QDBusConnection::ExportAllProperties;

    QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name))
            .registerObject("/org/deepin/DeviceInfo", this, opts);
}

QString DeviceInterface::getInfo(const QString &key)
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

void DeviceInterface::refreshInfo()
{
    emit sigUpdate();
}

void DeviceInterface::setMonitorDeviceFlag(bool flag)
{
    MainJob *parentMainJob = dynamic_cast<MainJob *>(parent());
    if (parentMainJob != nullptr) {
        parentMainJob->setWorkingFlag(flag);
    }
}

QString DeviceInterface::getGpuInfoForFTDTM()
{
    static QString gpuMemInfo { "" };
    if (gpuMemInfo.isEmpty()) {
        QMap<QString, QString> mapInfo;
        if (getGpuMemInfoForFTDTM(mapInfo)) {
            for (auto it = mapInfo.begin(); it != mapInfo.end(); ++it) {
                QString tmpInfo = it.key() + ": " + it.value() + "\n";
                gpuMemInfo.append(tmpInfo);
            }
        }
    }
    return gpuMemInfo;
}
