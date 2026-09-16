// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbauthorizationutils.h"

#include <QDir>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace {

QString physicalDevicePath(const QString &path, const QString &sysRoot)
{
    if (!path.startsWith("/devices/") || path.contains(".."))
        return {};

    const QFileInfo pathInfo(path);
    const QString pathName = pathInfo.fileName();
    static const QRegularExpression interfacePattern("^(.+):[0-9]+\\.[0-9]+$");
    static const QRegularExpression devicePattern("^[0-9]+-[0-9]+(?:\\.[0-9]+)*$");
    const QRegularExpressionMatch interfaceMatch = interfacePattern.match(pathName);

    QString deviceName;
    QString devicePath;
    if (interfaceMatch.hasMatch()) {
        deviceName = interfaceMatch.captured(1);
        devicePath = pathInfo.path();
        if (QFileInfo(devicePath).fileName() != deviceName)
            return {};
    } else {
        deviceName = pathName;
        devicePath = path;
        if (!devicePattern.match(deviceName).hasMatch())
            return {};
    }

    return QDir(sysRoot + devicePath).exists() ? devicePath : QString();
}

bool readAuthorized(const QString &filePath, bool &authorized)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to read USB authorized file" << filePath << file.errorString();
        return false;
    }

    const QByteArray value = file.readAll().trimmed();
    if (value != "0" && value != "1") {
        qWarning() << "Invalid USB authorized value" << filePath << value;
        return false;
    }

    authorized = value == "1";
    return true;
}

bool writeAuthorized(const QString &filePath, bool authorized)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open USB authorized file" << filePath << file.errorString();
        return false;
    }

    const QByteArray value = authorized ? "1" : "0";
    if (file.write(value) != value.size()) {
        qWarning() << "Failed to write USB authorized file" << filePath << file.errorString();
        return false;
    }

    return true;
}

} // namespace

QStringList UsbAuthorizationUtils::interfacePaths(const QString &path, const QString &sysRoot)
{
    const QString devicePath = physicalDevicePath(path, sysRoot);
    if (devicePath.isEmpty())
        return {};

    const QString deviceName = QFileInfo(devicePath).fileName();
    QDir deviceDir(sysRoot + devicePath);
    const QRegularExpression siblingPattern(QString("^%1:[0-9]+\\.[0-9]+$")
                                                    .arg(QRegularExpression::escape(deviceName)));
    QStringList interfaces;
    const QFileInfoList entries = deviceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo &entry : entries) {
        if (!siblingPattern.match(entry.fileName()).hasMatch())
            continue;

        const QString interfacePath = devicePath + '/' + entry.fileName();
        if (QFileInfo::exists(sysRoot + interfacePath + "/authorized"))
            interfaces.append(interfacePath);
    }

    return interfaces;
}

bool UsbAuthorizationUtils::setInterfacesAuthorized(const QString &path, bool authorized, const QString &sysRoot)
{
    const QString devicePath = physicalDevicePath(path, sysRoot);
    const QStringList interfaces = interfacePaths(path, sysRoot);
    if (devicePath.isEmpty() || interfaces.isEmpty()) {
        qWarning() << "No USB interfaces found for" << path;
        return false;
    }

    QList<QPair<QString, bool>> originalStates;
    for (const QString &interfacePath : interfaces) {
        const QString authorizedPath = sysRoot + interfacePath + "/authorized";
        bool originalState = false;
        if (!readAuthorized(authorizedPath, originalState))
            return false;
        originalStates.append(qMakePair(interfacePath, originalState));
    }

    bool originalDeviceState = false;
    const QString deviceAuthorizedPath = sysRoot + devicePath + "/authorized";
    if (authorized && !readAuthorized(deviceAuthorizedPath, originalDeviceState))
        return false;

    int changedCount = 0;
    const auto rollbackInterfaces = [&originalStates, &sysRoot, &changedCount]() {
        bool restored = true;
        for (int index = changedCount - 1; index >= 0; --index) {
            const QPair<QString, bool> &state = originalStates.at(index);
            if (!writeAuthorized(sysRoot + state.first + "/authorized", state.second)) {
                qCritical() << "Failed to roll back USB interface" << state.first;
                restored = false;
            }
        }
        return restored;
    };

    for (const QString &interfacePath : interfaces) {
        if (!writeAuthorized(sysRoot + interfacePath + "/authorized", authorized)) {
            if (!rollbackInterfaces())
                qCritical() << "USB interface rollback incomplete for" << path;
            return false;
        }
        ++changedCount;
    }

    if (authorized
            && (!writeAuthorized(deviceAuthorizedPath, false)
                || !writeAuthorized(deviceAuthorizedPath, true))) {
        const bool deviceRestored = writeAuthorized(deviceAuthorizedPath, originalDeviceState);
        const bool interfacesRestored = rollbackInterfaces();
        if (!deviceRestored || !interfacesRestored)
            qCritical() << "USB device rollback incomplete for" << devicePath;
        return false;
    }

    qInfo() << "Set USB interfaces authorized" << authorized << path << interfaces;
    return true;
}
