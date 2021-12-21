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

#include <mutex>

class QDBusInterface;
class DBusEnableInterface
{
public:
    inline static DBusEnableInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusEnableInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusEnableInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief getRemoveInfo : 获取移除信息
     * @param info : 获取的信息
     * @return
     */
    bool getRemoveInfo(QString &info);

    /**
     * @brief getAuthorizedInfo
     * @param lst
     * @return
     */
    bool getAuthorizedInfo(QString& lst);

    /**
     * @brief isDeviceEnabled 判断设备是否被禁用 通过后台查询
     * @param unique_id 设备的唯一标识
     * @return
     */
    bool isDeviceEnabled(const QString& unique_id);

    /**
     * @brief enable 启用禁用
     * @param name 唯一标识
     * @param path 系统路径
     * @param enable_device 是否禁用
     */
    bool enable(const QString& hclass, const QString& name, const QString& path, const QString& value, bool enable_device, const QString& strDriver="");

    /**
     * @brief enablePrinter 启用禁用打印机
     * @param hclass
     * @param name
     * @param path
     * @param enable_device
     * @return
     */
    bool enablePrinter(const QString& hclass, const QString& name, const QString& path, bool enable_device);

protected:
    DBusEnableInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static std::atomic<DBusEnableInterface *> s_Instance;
    static std::mutex m_mutex;

    QDBusInterface       *mp_Iface;
};

#endif // DBUSENABLEINTERFACE_H
