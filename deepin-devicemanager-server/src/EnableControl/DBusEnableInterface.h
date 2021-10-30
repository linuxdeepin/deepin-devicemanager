/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Jun.Liu <liujuna@uniontech.com>
*
* Maintainer: XiaoMei.Ji <jixiaomei@uniontech.com>
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

#ifndef DBUSENABLEINTERFACE_H
#define DBUSENABLEINTERFACE_H

#include <QObject>
#include <QDBusContext>

class DBusEnableInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.enablemanager")
public:
    explicit DBusEnableInterface(QObject *parent = nullptr);

public slots:
    /**
     * @brief getRemoveInfo
     * @return
     */
    Q_SCRIPTABLE QString getRemoveInfo();

    /**
     * @brief getAuthorizedInfo 获取被禁用的设备信息
     * @return
     */
    Q_SCRIPTABLE QString getAuthorizedInfo();

    /**
     * @brief enable 启用禁用设备
     * @param key 设备的唯一标识
     * @param path 授权文件的路径
     * @param enable_device 启用或者禁用
     * @return 是否成功
     */
    Q_SCRIPTABLE QString enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device);

    /**
     * @brief enablePrinter 禁用打印机
     * @param hclass
     * @param name
     * @param path
     * @param enable_device
     * @return
     */
    Q_SCRIPTABLE QString enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device);

signals:
    void update();

private:

    /**
     * @brief enable 通过authorized文件启用禁用设备
     * @param key 设备的唯一标识
     * @param path 授权文件的路径
     * @param enable 启用或者禁用
     * @return
     */
    QString authorizedEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable_device);

    /**
     * @brief removeEnable 通过remove文件启用禁用设备
     * @param key 设备的唯一标识
     * @param path 授权文件的路径
     * @param enable 启用或者禁用
     * @return
     */
    QString removeEnable(const QString& hclass, const QString& name, const QString& path, bool enable);

    /**
     * @brief construct_uri
     * @param buffer
     * @param buflen
     * @param base
     * @param value
     */
    void construct_uri(char *buffer, size_t buflen, const char *base, const char *value);
};

#endif // DBUSENABLEINTERFACE_H
