#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

class QDBusInterface;

class DBusInterface
{
public:
    static DBusInterface *getInstance()
    {
        if (!s_Instance)
            s_Instance = new DBusInterface();
        return s_Instance;
    }

    /**
     * @brief getInfo：获取信息
     * @param key：命令关键字
     * @param info：获取的设备信息
     * @return 信息是否有效
     */
    bool getInfo(const QString &key, QString &info);

protected:
    DBusInterface();

private:
    /**
     * @brief init:初始化DBus
     */
    void init();

private:
    static DBusInterface *s_Instance;
    QDBusInterface       *mp_Iface;
};

#endif // DBUSINTERFACE_H
