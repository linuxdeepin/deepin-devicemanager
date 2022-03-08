#ifndef DBUSWAKEUPINTERFACE_H
#define DBUSWAKEUPINTERFACE_H

#include <QObject>

#include <mutex>

class QDBusInterface;

class DBusWakeupInterface
{
public:
    inline static DBusWakeupInterface *getInstance()
    {
        // 利用原子变量解决，单例模式造成的内存泄露
        DBusWakeupInterface *sin = s_Instance.load();

        if (!sin) {
            // std::lock_guard 自动加锁解锁
            std::lock_guard<std::mutex> lock(m_mutex);
            sin = s_Instance.load();

            if (!sin) {
                sin = new DBusWakeupInterface();
                s_Instance.store(sin);
            }
        }

        return sin;
    }

    /**
     * @brief setWakeupMachine 设置是否唤醒
     * @param unique_id 设备的唯一标识
     * @param path 设备节点路径
     * @param wakeup 可唤醒 不可唤醒
     * @return
     */
    bool setWakeupMachine(const QString& unique_id, const QString& path, bool wakeup);

protected:
    DBusWakeupInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();
private:
    static std::atomic<DBusWakeupInterface *> s_Instance;
    static std::mutex                         m_mutex;
    QDBusInterface                            *mp_Iface;
};

#endif // DBUSWAKEUPINTERFACE_H
