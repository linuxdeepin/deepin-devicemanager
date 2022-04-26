#ifndef DBUSWAKEUPINTERFACE_H
#define DBUSWAKEUPINTERFACE_H

#include <QObject>
#include <QDBusContext>

class DBusWakeupInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.wakeupmanager")
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

    /**
     * @brief setNetworkWake 设置网卡唤醒
     * @param logicalName 网卡的逻辑名称
     * @param wakeup 是否允许唤醒
     * @return
     */
    Q_SCRIPTABLE bool setNetworkWake(const QString& logicalName, bool wakeup);

    /**
     * @brief isNetworkWakeup 判断网卡是否禁用
     * @param logicalName 网卡的逻辑名称
     * @return
     */
    Q_SCRIPTABLE int isNetworkWakeup(const QString& logicalName);

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
