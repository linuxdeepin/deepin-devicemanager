// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
     * @param hclass 类型
     * @param name 名称
     * @param path 授权文件的路径
     * @param value 传过来的数值
     * @param enable_device 启用或者禁用
     * @param strDriver
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
     * @brief authorizedEnable 通过authorized文件启用禁用设备
     * @param hclass 设备类型
     * @param name 设备名称
     * @param path 设备节点路径
     * @param unique_id 设备的唯一标识
     * @param enable_device 启用或者禁用
     * @param strDriver 驱动名称
     * @return
     */
    bool authorizedEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable_device, const QString strDriver="");

    /**
     * @brief removeEnable 通过remove文件启用禁用设备
     * @param hclass 设备类型
     * @param name 设备名称
     * @param path 设备节点路径
     * @param unique_id 设备的唯一标识
     * @param enable 启用或者禁用
     * @param strDriver 驱动名称
     * @return
     */
    bool removeEnable(const QString& hclass, const QString& name, const QString& path, const QString& unique_id, bool enable, const QString strDriver="");

    /**
     * @brief ioctlEnableNetwork 通过ioctl启用禁用网卡
     * @param hclass 设备类型
     * @param name 设备名称
     * @param logical_name 逻辑名称
     * @param unique_id 唯一标识
     * @param enable 启用或者禁用
     * @param strDriver 驱动名称
     * @return 返回是否成功
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

    bool isRootCaller();
};

#endif // DBUSENABLEINTERFACE_H
