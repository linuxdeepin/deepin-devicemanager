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
     * @brief getInfo
     * @param key
     * @param info
     * @return
     */
    bool getInfo(const QString &key, QString &info);

protected:
    DBusInterface();

private:
    /**
     * @brief init
     */
    void init();

private:
    static DBusInterface *s_Instance;
    QDBusInterface       *mp_Iface;
};

#endif // DBUSINTERFACE_H
