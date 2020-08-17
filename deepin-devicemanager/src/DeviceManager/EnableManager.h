#ifndef ENABLEMANAGER_H
#define ENABLEMANAGER_H

#include <QObject>

class EnableManager : public QObject
{
    Q_OBJECT
public:
    static EnableManager *instance()
    {
        if (!s_Instance)
            s_Instance = new EnableManager;
        return s_Instance;
    }

    /**
     * @brief enableDeviceByInput : 用来启用禁用设备
     * @param name : 该设备名称
     * @return : 返回是否禁用成功
     */
    bool enableDeviceByInput(const QString &name, bool enable);

protected:
    EnableManager();

private:
    static EnableManager *s_Instance;
};

#endif // ENABLEMANAGER_H
