#ifndef DBUSWAKEUPINTERFACE_H
#define DBUSWAKEUPINTERFACE_H

#include <QObject>
#include <QDBusContext>

class DBusWakeupInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.wakeupmanager.test")
public:
    explicit DBusWakeupInterface(QObject* parent = nullptr);

public slots:
    /**
     * @brief setWakeupMachine
     * @param unique_id
     * @param path
     * @param wakeup
     * @return
     */
    Q_SCRIPTABLE bool setWakeupMachine(const QString& unique_id, const QString& path, bool wakeup);

private:

    /**
     * @brief saveWakeupInfo 保存数据到数据库
     * @param unique_id 唯一标识
     * @param path 节点路径
     * @param wakeup 是否唤醒
     */
    void saveWakeupInfo(const QString& unique_id, const QString& path, bool wakeup);
};

#endif // DBUSWAKEUPINTERFACE_H
