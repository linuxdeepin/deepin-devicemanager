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
    Q_SCRIPTABLE bool enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device, const QString strDriver="");

    /**
     * @brief enablePrinter 禁用打印机
     * @param hclass 设备类型
     * @param name 打印机名称
     * @param path -----
     * @param enable_device 是否被启用
     * @return
     */
    Q_SCRIPTABLE bool enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device);

    /**
     * @brief isDeviceEnabled 判断设备是否被禁用，通过查询数据库来判断
     * @param unique_id 设备的唯一 sid
     * @return
     */
    Q_SCRIPTABLE bool isDeviceEnabled(const QString& unique_id);

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
    bool authorizedEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable_device, const QString strDriver="");

    /**
     * @brief removeEnable 通过remove文件启用禁用设备
     * @param key 设备的唯一标识
     * @param path 授权文件的路径
     * @param enable 启用或者禁用
     * @return
     */
    bool removeEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable, const QString strDriver="");

    /**
     * @brief ioctlEnableNetwork 通过ioctl启用禁用网卡
     * @param key 设备的唯一标识
     * @param logical_name 网卡的逻辑名称
     * @param enable 启用或者禁用
     * @return
     */
    bool ioctlEnableNetwork(const QString& hclass, const QString& name, const QString& logical_name, const QString& unique_id, bool enable, const QString strDriver="");

    /**
     * @brief construct_uri
     * @param buffer
     * @param buflen
     * @param base
     * @param value
     */
    void construct_uri(char *buffer, size_t buflen, const char *base, const char *value);

    /**
     * @brief modifyPath 修正路径 保证xxx.0
     *                比如将  /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.1
     *                修正为  /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.0
     * @param path 需要修正的路径 /devices/pci0000:00/0000:00:14.0/usb1/1-8/1-8:1.0
     */
    void modifyPath(QString& path);
};

#endif // DBUSENABLEINTERFACE_H
