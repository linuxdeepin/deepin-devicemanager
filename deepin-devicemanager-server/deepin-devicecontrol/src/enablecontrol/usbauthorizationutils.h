// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBAUTHORIZATIONUTILS_H
#define USBAUTHORIZATIONUTILS_H

#include <QString>
#include <QStringList>

class UsbAuthorizationUtils
{
public:
    /**
     * @brief Return all authorized-capable interfaces of a physical USB device.
     * @param path USB device or interface path below /devices.
     * @param sysRoot Sysfs root, overridden by tests.
     */
    static QStringList interfacePaths(const QString &path, const QString &sysRoot = "/sys");
    /**
     * @brief Set all interfaces to the same authorization state.
     * @param path USB device or interface path below /devices.
     * @param authorized Whether the interfaces should be authorized.
     * @param sysRoot Sysfs root, overridden by tests.
     */
    static bool setInterfacesAuthorized(const QString &path, bool authorized, const QString &sysRoot = "/sys");
};

#endif // USBAUTHORIZATIONUTILS_H
