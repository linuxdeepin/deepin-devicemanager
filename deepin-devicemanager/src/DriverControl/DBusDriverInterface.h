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

#ifndef DBUSDRIVERINTERFACE_H
#define DBUSDRIVERINTERFACE_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>

#include <mutex>

class DBusDriverInterface : public QObject
{
Q_OBJECT
public:
    inline static DBusDriverInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusDriverInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusDriverInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief uninstallDriver 卸载驱动，通过dbus调用 com.deepin.devicemanager /com/deepin/drivermanager 里面的unInstallDriver接口
     * @param driver 需要卸载的驱动名称
     */
    void uninstallDriver(const QString& driver);

    /**
     * @brief installDriver 更新驱动，通过dbus调用 com.deepin.devicemanager /com/deepin/drivermanager 里面的installDriver接口
     * @param driver 需要安装的驱动文件名称
     */
    void installDriver(const QString& driver);

signals:
    void processChange(qint32 value, QString details);
    void processEnd(bool sucess);

protected:
    explicit DBusDriverInterface(QObject* parent = nullptr);
    virtual ~DBusDriverInterface();

private slots:
    /**
     * @brief slotProcessChange 链接后台驱动处理过程中发送的信号
     * @param value 当前处理的进度
     * @param detail 发送过来的时时信息
     */
    void slotProcessChange(qint32 value,QString detail);

    /**
     * @brief slotCallFinished 更新结束结束的回调
     */
    void slotCallFinished(QDBusPendingCallWatcher* watcher);

private:
    /**
     * @brief init 初始化工作，连接dbus
     */
    void init();

private:
    static std::atomic<DBusDriverInterface *> s_Instance;
    static std::mutex m_mutex;
    QDBusInterface       *mp_Iface;
};

#endif // DBUSDRIVERINTERFACE_H
