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

#ifndef DBUSANYTHINGINTERFACE_H
#define DBUSANYTHINGINTERFACE_H

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#include <mutex>

class DBusAnythingInterface : public QObject
{
    Q_OBJECT
public:
    inline static DBusAnythingInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusAnythingInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusAnythingInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief searchDriver 获取指定目录下的驱动文件
     * @param path 搜索路径
     * @param lstDriver 出参：驱动列表
     * @return
     */
    bool searchDriver(const QString& sPath, QStringList& lstDriver);

protected:
    DBusAnythingInterface();

private:
    /**
     * @brief init 初始化工作，连接dbus
     */
    void init();

    /**
     * @brief hasLFT 判断是否可以通过com.deepin.anything遍历驱动文件
     * ************* /home 可能不行 需要使用 /data/home
     * @param path 需要判断的目录
     * @return
     */
    bool hasLFT(const QString& path);

private:
    static std::atomic<DBusAnythingInterface *> s_Instance;
    static std::mutex m_mutex;
    QDBusInterface       *mp_Iface;
};

#endif // DBUSANYTHINGINTERFACE_H
