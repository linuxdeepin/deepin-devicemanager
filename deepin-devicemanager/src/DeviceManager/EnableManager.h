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
    bool enableDeviceByInput(const QString &name, bool enable, int index = 0);

    /**
     * @brief isDeviceEnable : 用来启用和禁用设备
     * @param name
     * @return
     */
    bool isDeviceEnable(const QString &name);

    /**
     * @brief enableDeviceByDriver
     * @param driver
     * @return
     */
    bool enableDeviceByDriver(bool enable, const QString &driver);

    /**
     * @brief isDeviceEnableByDriver
     * @param name
     * @return
     */
    bool isDeviceEnableByDriver(const QString &driver);

    /**
     * @brief enablePrinter : 禁用打印机
     * @param name : 打印机名称
     * @return　: 禁用是否成功
     */
    bool enablePrinter(const QString &name, bool enable);

    /**
     * @brief getDeviceID
     * @param name
     * @param index
     * @return
     */
    int getDeviceID(const QString &name, int index);

protected:
    EnableManager();

private:
    /**
     * @brief:命令的执行
     */
    QString getPKStr(const QString &dtStr, const QString &dtInt);
    void getPKStr(QString &dtStr, QString &dtInt, const QString &cStr);

    QString getDriverPath(const QString &driver);


private:
    static EnableManager *s_Instance;
};

#endif // ENABLEMANAGER_H
